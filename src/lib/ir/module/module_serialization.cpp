#include "module_serialization_internal.h"

#include "ink/ir/ir_builder.h"
#include "ink/ir/analysis/type_layout.h"
#include "ink/parser/ast_walker.h"

#include <algorithm>
#include <unordered_map>

namespace ink::ir
{
  class ModuleArchiveAccess
  {
    public:
      static void retain(Module &Owner, std::shared_ptr<const parser::ParseResult> Parsed)
      {
        if (std::find(Owner.ArchivedASTs.begin(), Owner.ArchivedASTs.end(), Parsed) == Owner.ArchivedASTs.end())
        {
          Owner.ArchivedASTs.push_back(std::move(Parsed));
        }
      }
  };
} // namespace ink::ir

namespace ink::ir::archive
{
  namespace
  {
    // These small enums are part of the wire schema; changing their C++ order requires an explicit mapping.
    static_assert(static_cast<unsigned>(AccessKind::ReadOnly) == 0 && static_cast<unsigned>(AccessKind::ReadWrite) == 1);
    static_assert(static_cast<unsigned>(ParameterKind::Positional) == 0 && static_cast<unsigned>(ParameterKind::Named) == 1 && static_cast<unsigned>(ParameterKind::Variadic) == 2);
    static_assert(static_cast<unsigned>(CallingConvention::C) == 0 && static_cast<unsigned>(CallingConvention::Fast) == 1 && static_cast<unsigned>(CallingConvention::Cold) == 2);
    static_assert(static_cast<unsigned>(LanguageLinkage::Ink) == 0 && static_cast<unsigned>(LanguageLinkage::C) == 1);
    static_assert(static_cast<unsigned>(FunctionBinding::Local) == 0 && static_cast<unsigned>(FunctionBinding::Import) == 1 && static_cast<unsigned>(FunctionBinding::Export) == 2);
    static_assert(static_cast<unsigned>(VisibilityKind::Public) == 0 && static_cast<unsigned>(VisibilityKind::Private) == 1);

    bool isType(Tag Kind)
    {
      return Kind >= Tag::Meta && Kind <= Tag::FunctionType;
    }

    bool isPool(Tag Kind)
    {
      return (Kind >= Tag::Meta && Kind <= Tag::StringConstant) || Kind == Tag::ArrayConstant || Kind == Tag::ClassConstant;
    }

    bool syntax(Tag Kind)
    {
      return Kind == Tag::AST || Kind == Tag::ModuleDecl || Kind == Tag::FunctionDecl || Kind == Tag::ClassDecl;
    }

    ModuleArchiveStatus astStatus(parser::ASTArchiveStatus Status)
    {
      switch (Status)
      {
      case parser::ASTArchiveStatus::Success:
        return ModuleArchiveStatus::Success;
      case parser::ASTArchiveStatus::InvalidInput:
        return ModuleArchiveStatus::InvalidInput;
      case parser::ASTArchiveStatus::InvalidArchive:
        return ModuleArchiveStatus::InvalidArchive;
      case parser::ASTArchiveStatus::UnsupportedVersion:
        return ModuleArchiveStatus::UnsupportedVersion;
      case parser::ASTArchiveStatus::LimitExceeded:
        return ModuleArchiveStatus::LimitExceeded;
      }
      return ModuleArchiveStatus::InvalidArchive;
    }

    class Collector
    {
      public:
        Collector(const Module &Root, State &Data, std::span<const parser::ParseResult *const> Sources)
            : Root(Root),
              Data(Data),
              Sources(Sources.begin(), Sources.end())
        {
        }

        bool run()
        {
          add(Root, 0, 1);
          // Breadth-first ownership traversal keeps parents before children without host recursion.
          for (std::size_t Index = 0; Data.good() && Index < Objects.size(); ++Index)
          {
            const Value &Object = *Objects[Index];
            const auto Child = [&](const Value &ValueObject)
            {
              if (ValueObject.outer() != &Object)
              {
                Data.fail("Inconsistent IR ownership", ModuleArchiveStatus::InvalidInput);
                return;
              }
              add(ValueObject, Index + 1, Depths[Index] + 1);
            };
            if (Module::classof(&Object))
            {
              Child(static_cast<const Module &>(Object).entryBlock());
            }
            else if (Function::classof(&Object))
            {
              const auto &FunctionValue = static_cast<const Function &>(Object);
              for (const auto &Parameter : FunctionValue.parameters())
              {
                Child(*Parameter);
              }
              for (const auto &Block : FunctionValue.blocks())
              {
                Child(*Block);
              }
            }
            else if (BasicBlock::classof(&Object))
            {
              for (const auto &ValueObject : static_cast<const BasicBlock &>(Object).values())
              {
                Child(*ValueObject);
              }
            }
          }
          for (std::size_t Index = 0; Data.good() && Index < Objects.size(); ++Index)
          {
            // References can grow Records, so construct the payload off-vector.
            Record Entry;
            Entry.Parent = Data.Records[Index].Parent;
            const Value &Object = *Objects[Index];
            Entry.Type = reference(Object.type());
            encode(Object, Entry);
            if (Data.good())
            {
              Data.Records[Index] = std::move(Entry);
            }
          }
          return Data.good() && declarations();
        }

      private:
        bool declarations()
        {
          struct PendingDecl
          {
              const Decl *Object;
              std::uint64_t Parent;
              std::size_t Depth;
          };
          std::vector<PendingDecl> PendingDeclarations;
          for (std::size_t Index = 0; Index < Objects.size(); ++Index)
          {
            if (Module::classof(Objects[Index]))
            {
              const auto &Owner = static_cast<const Module &>(*Objects[Index]);
              for (const auto &Source : Owner.archivedASTs())
              {
                if (std::find(Sources.begin(), Sources.end(), Source.get()) == Sources.end())
                {
                  Sources.push_back(Source.get());
                }
              }
              if (Owner.declarationRoot())
              {
                PendingDeclarations.push_back({Owner.declarationRoot(), Index + 1, Depths[Index] + 1});
              }
            }
          }
          if (PendingDeclarations.empty())
          {
            return true;
          }
          struct ASTReference
          {
              const parser::ParseResult *Parsed;
              std::size_t Node;
          };
          std::unordered_map<const parser::ASTNodeBase *, ASTReference> References;
          for (const auto *Source : Sources)
          {
            if (!Source || !Source->Unit || !Source->Unit->root())
            {
              return Data.fail("Missing declaration AST input", ModuleArchiveStatus::InvalidInput);
            }
            std::size_t NodeId = 0;
            parser::ASTWalker{}.walk(Source->Unit->root(), [&](const parser::ASTNodeBase *)
                                     {
                                       return Data.good() ? parser::WalkAction::Continue : parser::WalkAction::Stop;
                                     },
                                     [&](const parser::ASTNodeBase *Node)
                                     {
                                       if (++NodeId > Data.Limits.AST.MaxNodes)
                                       {
                                         Data.fail("Module AST node limit exceeded", ModuleArchiveStatus::LimitExceeded);
                                       }
                                       else if (Data.charge(1, 128))
                                       {
                                         References.emplace(Node, ASTReference{Source, NodeId});
                                       }
                                     });
            if (!Data.good())
            {
              return false;
            }
          }
          std::unordered_map<const parser::ParseResult *, std::size_t> ASTIds;
          for (std::size_t Position = 0; Data.good() && Position < PendingDeclarations.size(); ++Position)
          {
            const auto Current = PendingDeclarations[Position];
            if (Current.Depth > Data.Limits.MaxNestingDepth)
            {
              return Data.fail("Module declaration nesting limit exceeded", ModuleArchiveStatus::LimitExceeded);
            }
            const auto Found = References.find(&Current.Object->ast());
            if (Found == References.end())
            {
              return Data.fail("Provide the ParseResult owning each declaration AST in Sources", ModuleArchiveStatus::InvalidInput);
            }
            auto ASTId = ASTIds.find(Found->second.Parsed);
            if (ASTId == ASTIds.end())
            {
              auto Snapshot = Data.TextFormat ? parser::trySerializeASTText(*Found->second.Parsed, Data.astLimits()) : parser::trySerializeAST(*Found->second.Parsed, Data.astLimits());
              if (!Snapshot.succeeded())
              {
                return Data.fail(Snapshot.Message, astStatus(Snapshot.Status));
              }
              if (!Data.addRecord() || !Data.string(Snapshot.Bytes.size()))
              {
                return false;
              }
              auto &ASTRecord = Data.Records.back();
              ASTRecord.Kind = Tag::AST;
              ASTRecord.Text = std::move(Snapshot.Bytes);
              ASTId = ASTIds.emplace(Found->second.Parsed, Data.Records.size()).first;
            }
            Record Entry;
            Entry.Parent = Current.Parent;
            Entry.Kind = ModuleDecl::classof(Current.Object) ? Tag::ModuleDecl : FunctionDecl::classof(Current.Object) ? Tag::FunctionDecl
                                                                                                                       : Tag::ClassDecl;
            field(Entry, ASTId->second);
            field(Entry, Found->second.Node);
            name(Entry, Current.Object->name());
            if (!Data.addRecord())
            {
              return false;
            }
            Data.Records.back() = std::move(Entry);
            const auto Id = Data.Records.size();
            for (const auto &Child : Current.Object->children())
            {
              if (Child->parent() != Current.Object || &Child->module() != &Current.Object->module())
              {
                return Data.fail("Inconsistent declaration ownership", ModuleArchiveStatus::InvalidInput);
              }
              PendingDeclarations.push_back({Child.get(), Id, Current.Depth + 1});
            }
          }
          return Data.good();
        }

        std::uint64_t add(const Value &Object, std::uint64_t Parent, std::size_t Depth)
        {
          if (!Data.good())
          {
            return 0;
          }
          if (&Object.context() != &Root.context() || Ids.contains(&Object))
          {
            Data.fail("Foreign or multiply owned IR object", ModuleArchiveStatus::InvalidInput);
            return 0;
          }
          if (Depth > Data.Limits.MaxNestingDepth)
          {
            Data.fail("Module archive nesting limit exceeded", ModuleArchiveStatus::LimitExceeded);
            return 0;
          }
          if (!Data.addRecord())
          {
            return 0;
          }
          Data.Records.back().Parent = Parent;
          Objects.push_back(&Object);
          Depths.push_back(Depth);
          Ids.emplace(&Object, Objects.size());
          return Objects.size();
        }

        std::uint64_t reference(const Value &Object)
        {
          const auto Found = Ids.find(&Object);
          if (Found != Ids.end())
          {
            return Found->second;
          }
          if (!Type::classof(&Object) && !Constant::classof(&Object))
          {
            Data.fail("IR operand is outside the module ownership tree", ModuleArchiveStatus::InvalidInput);
            return 0;
          }
          return add(Object, 0, 1);
        }

        void field(Record &Entry, std::uint64_t Number)
        {
          if (Entry.Fields.size() >= Data.Limits.MaxFields)
          {
            Data.fail("Module archive field limit exceeded", ModuleArchiveStatus::LimitExceeded);
          }
          else if (Data.fields(1))
          {
            Entry.Fields.push_back(Number);
          }
        }

        void ref(Record &Entry, const Value &Object)
        {
          field(Entry, reference(Object));
        }

        void text(Record &Entry, std::string_view Text)
        {
          if (Data.string(Text.size()))
          {
            Entry.Text = Text;
          }
        }

        void name(Record &Entry, Name NameValue)
        {
          text(Entry, Root.context().namePool().text(NameValue));
        }

        void encode(const Value &Object, Record &Entry)
        {
          if (Type::classof(&Object))
          {
            encodeType(static_cast<const Type &>(Object), Entry);
            return;
          }
          switch (Object.kind())
          {
          case ValueKind::IntegerConstant:
            Entry.Kind = Tag::IntegerConstant;
            for (auto Word : static_cast<const IntegerConstant &>(Object).value().words())
            {
              field(Entry, Word);
            }
            break;
          case ValueKind::FloatConstant:
            Entry.Kind = Tag::FloatConstant;
            field(Entry, static_cast<const FloatConstant &>(Object).value().bits());
            break;
          case ValueKind::BoolConstant:
            Entry.Kind = Tag::BoolConstant;
            field(Entry, static_cast<const BoolConstant &>(Object).value());
            break;
          case ValueKind::StringConstant:
            Entry.Kind = Tag::StringConstant;
            text(Entry, static_cast<const StringConstant &>(Object).value());
            break;
          case ValueKind::ArrayConstant:
            Entry.Kind = Tag::ArrayConstant;
            for (const auto *Element : static_cast<const ArrayConstant &>(Object).elements())
            {
              ref(Entry, *Element);
            }
            break;
          case ValueKind::ClassConstant:
            Entry.Kind = Tag::ClassConstant;
            for (const auto *Field : static_cast<const ClassConstant &>(Object).fields())
            {
              ref(Entry, *Field);
            }
            break;
          case ValueKind::ClassInstruction:
            Entry.Kind = Tag::ClassValue;
            for (const auto *Field : static_cast<const ClassInstruction &>(Object).fields())
            {
              ref(Entry, *Field);
            }
            break;
          case ValueKind::FieldPointerInstruction:
            Entry.Kind = Tag::FieldPointer;
            field(Entry, static_cast<const FieldPointerInstruction &>(Object).fieldIndex());
            ref(Entry, static_cast<const FieldPointerInstruction &>(Object).address());
            break;
          case ValueKind::FieldExtractInstruction:
            Entry.Kind = Tag::FieldExtract;
            field(Entry, static_cast<const FieldExtractInstruction &>(Object).fieldIndex());
            ref(Entry, static_cast<const FieldExtractInstruction &>(Object).object());
            break;
          case ValueKind::ArrayInstruction:
            Entry.Kind = Tag::ArrayValue;
            field(Entry, static_cast<const ArrayInstruction &>(Object).repeated());
            for (const auto *Element : static_cast<const ArrayInstruction &>(Object).elements())
            {
              ref(Entry, *Element);
            }
            break;
          case ValueKind::ArrayElementPointerInstruction:
            Entry.Kind = Tag::ArrayElementPointer;
            ref(Entry, static_cast<const ArrayElementPointerInstruction &>(Object).address());
            ref(Entry, static_cast<const ArrayElementPointerInstruction &>(Object).index());
            break;
          case ValueKind::ArrayExtractInstruction:
            Entry.Kind = Tag::ArrayExtract;
            ref(Entry, static_cast<const ArrayExtractInstruction &>(Object).array());
            ref(Entry, static_cast<const ArrayExtractInstruction &>(Object).index());
            break;
          case ValueKind::Module:
            Entry.Kind = Tag::Module;
            name(Entry, static_cast<const Module &>(Object).name());
            for (const auto *Class : static_cast<const Module &>(Object).classTypes())
            {
              ref(Entry, *Class);
            }
            break;
          case ValueKind::Function:
          {
            Entry.Kind = Tag::Function;
            const auto &FunctionValue = static_cast<const Function &>(Object);
            if (FunctionValue.isNativeExport() && !FunctionValue.hasBody())
            {
              Data.fail("Native export requires a function body", ModuleArchiveStatus::InvalidInput);
              break;
            }
            name(Entry, FunctionValue.name());
            field(Entry, static_cast<unsigned>(FunctionValue.callingConvention()));
            field(Entry, static_cast<unsigned>(FunctionValue.languageLinkage()));
            field(Entry, static_cast<unsigned>(FunctionValue.visibility()));
            field(Entry, static_cast<unsigned>(FunctionValue.binding()));
            if (const ClassType *Owner = FunctionValue.classOwner())
            {
              ref(Entry, *Owner);
              field(Entry, FunctionValue.initializerField() == std::numeric_limits<std::size_t>::max() ? 0 : FunctionValue.initializerField() + 1);
            }
            break;
          }
          case ValueKind::BasicBlock:
            Entry.Kind = Tag::Block;
            break;
          case ValueKind::FunctionParameter:
          {
            Entry.Kind = Tag::Parameter;
            const auto &Parameter = static_cast<const FunctionParameter &>(Object);
            name(Entry, Parameter.name());
            field(Entry, Parameter.index());
            field(Entry, static_cast<unsigned>(Parameter.parameterKind()));
            break;
          }
          case ValueKind::CallInstruction:
          {
            Entry.Kind = Tag::Call;
            const auto &Call = static_cast<const CallInstruction &>(Object);
            ref(Entry, Call.callee());
            for (const Value *Argument : Call.arguments())
            {
              ref(Entry, *Argument);
            }
            break;
          }
          case ValueKind::CStringInstruction:
            Entry.Kind = Tag::CString;
            ref(Entry, static_cast<const CStringInstruction &>(Object).source());
            break;
          case ValueKind::AllocaInstruction:
            Entry.Kind = Tag::Alloca;
            ref(Entry, static_cast<const AllocaInstruction &>(Object).allocatedType());
            break;
          case ValueKind::LoadInstruction:
            Entry.Kind = Tag::Load;
            ref(Entry, static_cast<const LoadInstruction &>(Object).address());
            break;
          case ValueKind::StoreInstruction:
            Entry.Kind = Tag::Store;
            ref(Entry, static_cast<const StoreInstruction &>(Object).address());
            ref(Entry, static_cast<const StoreInstruction &>(Object).storedValue());
            break;
          case ValueKind::AddInstruction:
            Entry.Kind = Tag::Add;
            ref(Entry, static_cast<const AddInstruction &>(Object).left());
            ref(Entry, static_cast<const AddInstruction &>(Object).right());
            break;
          case ValueKind::LogicalNotInstruction:
            Entry.Kind = Tag::LogicalNot;
            ref(Entry, static_cast<const LogicalNotInstruction &>(Object).operand());
            break;
          case ValueKind::LogicalAndInstruction:
            Entry.Kind = Tag::LogicalAnd;
            ref(Entry, static_cast<const LogicalAndInstruction &>(Object).left());
            ref(Entry, static_cast<const LogicalAndInstruction &>(Object).right());
            break;
          case ValueKind::LogicalOrInstruction:
            Entry.Kind = Tag::LogicalOr;
            ref(Entry, static_cast<const LogicalOrInstruction &>(Object).left());
            ref(Entry, static_cast<const LogicalOrInstruction &>(Object).right());
            break;
          case ValueKind::CompareInstruction:
          {
            Entry.Kind = Tag::Compare;
            const auto &Compare = static_cast<const CompareInstruction &>(Object);
            const auto *Predicate = comparisonPredicateInfo(Compare.predicate());
            if (!Predicate)
            {
              Data.fail("Unsupported comparison predicate", ModuleArchiveStatus::InvalidInput);
              break;
            }
            field(Entry, Predicate->Wire);
            ref(Entry, Compare.left());
            ref(Entry, Compare.right());
            break;
          }
          case ValueKind::ReturnInstruction:
            Entry.Kind = Tag::Return;
            if (const Value *Returned = static_cast<const ReturnInstruction &>(Object).returnedValue())
            {
              ref(Entry, *Returned);
            }
            break;
          case ValueKind::BranchInstruction:
            Entry.Kind = Tag::Branch;
            ref(Entry, static_cast<const BranchInstruction &>(Object).target());
            break;
          case ValueKind::ConditionalBranchInstruction:
          {
            Entry.Kind = Tag::ConditionalBranch;
            const auto &Branch = static_cast<const ConditionalBranchInstruction &>(Object);
            ref(Entry, Branch.condition());
            ref(Entry, Branch.trueTarget());
            ref(Entry, Branch.falseTarget());
            break;
          }
          default:
            Data.fail("Unsupported IR value kind", ModuleArchiveStatus::InvalidInput);
          }
        }

        void encodeType(const Type &Object, Record &Entry)
        {
          switch (Object.typeKind())
          {
          case TypeKind::Meta:
            Entry.Kind = Tag::Meta;
            break;
          case TypeKind::Void:
            Entry.Kind = Tag::Void;
            break;
          case TypeKind::Bool:
            Entry.Kind = Tag::Bool;
            break;
          case TypeKind::Label:
            Entry.Kind = Tag::Label;
            break;
          case TypeKind::Module:
            Entry.Kind = Tag::ModuleType;
            break;
          case TypeKind::Integer:
            Entry.Kind = Tag::Integer;
            field(Entry, static_cast<const IntegerType &>(Object).bitWidth());
            field(Entry, static_cast<const IntegerType &>(Object).isSigned());
            break;
          case TypeKind::Float:
            Entry.Kind = Tag::Float;
            field(Entry, static_cast<const FloatType &>(Object).bitWidth());
            break;
          case TypeKind::Array:
            Entry.Kind = Tag::Array;
            ref(Entry, static_cast<const ArrayType &>(Object).elementType());
            field(Entry, static_cast<const ArrayType &>(Object).elementCount());
            break;
          case TypeKind::Slice:
            Entry.Kind = Tag::Slice;
            ref(Entry, static_cast<const SliceType &>(Object).elementType());
            field(Entry, static_cast<unsigned>(static_cast<const SliceType &>(Object).access()));
            break;
          case TypeKind::Pointer:
            Entry.Kind = Tag::Pointer;
            ref(Entry, static_cast<const PointerType &>(Object).pointeeType());
            field(Entry, static_cast<unsigned>(static_cast<const PointerType &>(Object).access()));
            break;
          case TypeKind::Reference:
            Entry.Kind = Tag::Reference;
            ref(Entry, static_cast<const ReferenceType &>(Object).referentType());
            field(Entry, static_cast<unsigned>(static_cast<const ReferenceType &>(Object).access()));
            break;
          case TypeKind::Class:
          {
            Entry.Kind = Tag::Class;
            const auto &Class = static_cast<const ClassType &>(Object);
            std::string Text(Root.context().namePool().text(Class.name()));
            if (Class.isComplete())
            {
              field(Entry, Text.size());
              field(Entry, Class.identity().size());
              Text += Class.identity();
              for (const ClassField &Field : Class.fields())
              {
                ref(Entry, *Field.FieldType);
                field(Entry, static_cast<unsigned>(Field.Visibility));
                const auto FieldName = Root.context().namePool().text(Field.FieldName);
                field(Entry, FieldName.size());
                Text += FieldName;
              }
            }
            text(Entry, Text);
            break;
          }
          case TypeKind::Enum:
            Entry.Kind = Tag::Enum;
            name(Entry, static_cast<const UserDefinedType &>(Object).name());
            break;
          case TypeKind::Interface:
            Entry.Kind = Tag::Interface;
            name(Entry, static_cast<const UserDefinedType &>(Object).name());
            break;
          case TypeKind::Function:
            Entry.Kind = Tag::FunctionType;
            ref(Entry, static_cast<const FunctionType &>(Object).returnType());
            for (const Type *Parameter : static_cast<const FunctionType &>(Object).parameterTypes())
            {
              ref(Entry, *Parameter);
            }
            break;
          }
        }

        const Module &Root;
        State &Data;
        std::vector<const Value *> Objects;
        std::vector<std::size_t> Depths;
        std::unordered_map<const Value *, std::uint64_t> Ids;
        std::vector<const parser::ParseResult *> Sources;
    };

    class Restorer
    {
      public:
        Restorer(IRContext &Context, State &Data)
            : Context(Context),
              Data(Data),
              Builder(Context),
              Values(Data.Records.size() + 1),
              Owners(Values.size()),
              Children(Values.size()),
              Users(Values.size()),
              Pending(Values.size()),
              Depths(Values.size(), 1),
              ASTs(Values.size()),
              ASTNodes(Values.size()),
              Declarations(Values.size())
        {
        }

        Module *run()
        {
          if (!validate())
          {
            return nullptr;
          }
          std::vector<std::size_t> Ready;
          for (std::size_t Id = 1; Id < Values.size(); ++Id)
          {
            if (Pending[Id] == 0 && !syntax(record(Id).Kind))
            {
              Ready.push_back(Id);
            }
          }
          for (std::size_t Position = 0; Position < Ready.size(); ++Position)
          {
            const auto Id = Ready[Position];
            if (isType(record(Id).Kind) && !materialize(Id, Ready))
            {
              return nullptr;
            }
          }
          if (!completeClasses())
          {
            return nullptr;
          }
          for (std::size_t Position = 0; Position < Ready.size(); ++Position)
          {
            const auto Id = Ready[Position];
            if (!isType(record(Id).Kind) && !materialize(Id, Ready))
            {
              return nullptr;
            }
          }
          if (Ready.size() != ValueCount)
          {
            Data.fail("Cyclic IR type or operand dependencies");
            return nullptr;
          }
          for (const auto &[Operand, Type] : Data.OperandTypes)
          {
            if (!validId(Operand) || !validId(Type) || !Values[Operand] || &Values[Operand]->type() != Values[Type])
            {
              Data.fail("Operand type does not match its definition");
              return nullptr;
            }
          }
          for (std::size_t Id = 2; Id < Values.size(); ++Id)
          {
            if (!Owners[Id])
            {
              continue;
            }
            auto *Parent = const_cast<Value *>(Values[record(Id).Parent]);
            if (!BasicBlock::classof(Parent) || !Builder.appendValue(static_cast<BasicBlock &>(*Parent), std::move(Owners[Id])))
            {
              Data.fail("Invalid block ownership or instruction placement at object " + std::to_string(Id));
              return nullptr;
            }
          }
          auto *Root = static_cast<Module *>(Owners[1].get());
          if (!restoreDeclarations())
          {
            return nullptr;
          }
          if (!Builder.appendModule(std::move(Owners[1])))
          {
            Data.fail("Cannot attach restored module");
            return nullptr;
          }
          return Root;
        }

      private:
        bool materialize(std::size_t Id, std::vector<std::size_t> &Ready)
        {
          Values[Id] = create(Id);
          if (!Values[Id] || &Values[Id]->type() != Values[record(Id).Type])
          {
            return Data.fail("Invalid IR object or mismatched result type at object " + std::to_string(Id) + " (" + std::string(tagInfo(static_cast<unsigned>(record(Id).Kind))->Text) + ")");
          }
          for (auto User : Users[Id])
          {
            if (--Pending[User] == 0)
            {
              Ready.push_back(User);
            }
          }
          return true;
        }

        bool completeClasses()
        {
          for (std::size_t Id = 1; Id < Values.size(); ++Id)
          {
            const auto &Entry = record(Id);
            if (isType(Entry.Kind) && !Values[Id])
            {
              return Data.fail("Cyclic IR structural type dependencies");
            }
            if (Entry.Kind != Tag::Class || Entry.Fields.empty() || Data.TypeAliases.contains(Id))
            {
              continue;
            }
            const auto &Fields = Entry.Fields;
            std::size_t Offset = static_cast<std::size_t>(Fields[0] + Fields[1]);
            std::vector<ClassField> Members;
            for (std::size_t Index = 2; Index < Fields.size(); Index += 3)
            {
              const auto *FieldType = as<Type>(Fields[Index]);
              if (!FieldType)
              {
                return Data.fail("Class field type is unavailable");
              }
              Members.push_back({Context.namePool().intern(std::string_view(Entry.Text).substr(Offset, static_cast<std::size_t>(Fields[Index + 2]))), FieldType, static_cast<VisibilityKind>(Fields[Index + 1])});
              Offset += static_cast<std::size_t>(Fields[Index + 2]);
            }
            if (!Builder.defineClassType(*as<ClassType>(Id), Members, std::string_view(Entry.Text).substr(static_cast<std::size_t>(Fields[0]), static_cast<std::size_t>(Fields[1]))))
            {
              return Data.fail("Invalid class definition or recursive value layout");
            }
          }
          for (std::size_t Id = 1; Id < Values.size(); ++Id)
          {
            if (record(Id).Kind == Tag::Class && !record(Id).Fields.empty() && !computeTypeLayout(*as<ClassType>(Id), Context.compilationContext().targetContext()))
            {
              return Data.fail("Incomplete or overflowing class layout");
            }
          }
          return true;
        }

        const Record &record(std::size_t Id) const
        {
          return Data.Records[Id - 1];
        }

        bool validId(std::uint64_t Id) const
        {
          return Id && Id < Values.size();
        }

        bool dependency(std::size_t Id, std::uint64_t Target, bool NeedsType = false)
        {
          if (!validId(Target) || syntax(record(Target).Kind) || (NeedsType && !isType(record(Target).Kind)))
          {
            return Data.fail("Invalid IR reference at object " + std::to_string(Id));
          }
          Users[Target].push_back(Id);
          ++Pending[Id];
          return true;
        }

        bool validate()
        {
          if (Data.Records.empty() || record(1).Kind != Tag::Module || record(1).Parent)
          {
            return Data.fail("Archive must start with a root module");
          }
          for (std::size_t Id = 1; Id < Values.size(); ++Id)
          {
            const auto &Entry = record(Id);
            const auto *Info = tagInfo(static_cast<unsigned>(Entry.Kind));
            if (!Info || Entry.Fields.size() < Info->MinFields || Entry.Fields.size() > Info->MaxFields || (!Info->HasText && !Entry.Text.empty()))
            {
              return Data.fail("Invalid fields at object " + std::to_string(Id));
            }
            if (syntax(Entry.Kind))
            {
              if (Entry.Type)
              {
                return Data.fail("Syntax records cannot have an IR type");
              }
              if (Entry.Kind == Tag::AST)
              {
                if (Entry.Parent)
                {
                  return Data.fail("AST snapshots cannot have a structural parent");
                }
                continue;
              }
              if (!Entry.Parent || Entry.Parent >= Id || !validId(Entry.Fields[0]) || record(Entry.Fields[0]).Kind != Tag::AST || !Entry.Fields[1])
              {
                return Data.fail("Invalid declaration parent or AST reference");
              }
              const auto ParentKind = record(Entry.Parent).Kind;
              if (Entry.Kind == Tag::ModuleDecl ? ParentKind != Tag::Module : ParentKind != Tag::ModuleDecl && ParentKind != Tag::FunctionDecl && ParentKind != Tag::ClassDecl)
              {
                return Data.fail("Invalid declaration parent kind");
              }
              Depths[Id] = Depths[Entry.Parent] + 1;
              if (Depths[Id] > Data.Limits.MaxNestingDepth)
              {
                return Data.fail("Module declaration nesting limit exceeded", ModuleArchiveStatus::LimitExceeded);
              }
              continue;
            }
            ++ValueCount;
            if (const auto Alias = Data.TypeAliases.find(Id); Alias != Data.TypeAliases.end() && !dependency(Id, Alias->second, true))
            {
              return false;
            }
            if (Entry.Kind == Tag::Meta)
            {
              if (Entry.Type != Id)
              {
                return Data.fail("Metatype must reference itself");
              }
            }
            else if (!dependency(Id, Entry.Type, true))
            {
              return false;
            }
            if (isPool(Entry.Kind))
            {
              if (Entry.Parent)
              {
                return Data.fail("Pool objects cannot have owners");
              }
            }
            else if (Id != 1)
            {
              if (!Entry.Parent || Entry.Parent >= Id)
              {
                return Data.fail("IR owners must precede their children");
              }
              const auto ParentKind = record(Entry.Parent).Kind;
              if ((Entry.Kind == Tag::Parameter && ParentKind != Tag::Function) || (Entry.Kind == Tag::Block ? ParentKind != Tag::Function && ParentKind != Tag::Module && ParentKind != Tag::Block : Entry.Kind != Tag::Parameter && ParentKind != Tag::Block))
              {
                return Data.fail("Invalid IR parent kind");
              }
              Depths[Id] = Depths[Entry.Parent] + 1;
              if (Depths[Id] > Data.Limits.MaxNestingDepth)
              {
                return Data.fail("Module archive ownership depth exceeded", ModuleArchiveStatus::LimitExceeded);
              }
              Children[Entry.Parent].push_back(Id);
            }
            switch (Entry.Kind)
            {
            case Tag::Class:
              if (!Entry.Fields.empty())
              {
                const auto &Fields = Entry.Fields;
                if (Fields.size() < 2 || (Fields.size() - 2) % 3 || !Fields[0] || !Fields[1] || Fields[0] > Entry.Text.size() || Fields[1] > Entry.Text.size() - Fields[0])
                {
                  return Data.fail("Invalid class name or identity");
                }
                std::size_t Offset = static_cast<std::size_t>(Fields[0] + Fields[1]);
                for (std::size_t Index = 2; Index < Fields.size(); Index += 3)
                {
                  if (!validId(Fields[Index]) || !isType(record(Fields[Index]).Kind) || Fields[Index + 1] > 1 || !Fields[Index + 2] || Fields[Index + 2] > Entry.Text.size() - Offset)
                  {
                    return Data.fail("Invalid class field metadata");
                  }
                  Offset += static_cast<std::size_t>(Fields[Index + 2]);
                }
                if (Offset != Entry.Text.size())
                {
                  return Data.fail("Trailing class field names");
                }
              }
              break;
            case Tag::Module:
              for (auto Class : Entry.Fields)
              {
                if (!dependency(Id, Class, true) || record(Class).Kind != Tag::Class)
                {
                  return Data.fail("Invalid reflected class registration");
                }
              }
              break;
            case Tag::Function:
              if (Entry.Fields.size() != 4 && (Entry.Fields.size() != 6 || !dependency(Id, Entry.Fields[4], true) || record(Entry.Fields[4]).Kind != Tag::Class))
              {
                return Data.fail("Invalid reflected function owner");
              }
              break;
            case Tag::FieldPointer:
            case Tag::FieldExtract:
              if (Entry.Fields[0] > std::numeric_limits<std::size_t>::max() || !dependency(Id, Entry.Fields[1]))
              {
                return Data.fail("Invalid class field operand");
              }
              break;
            case Tag::Array:
            case Tag::Slice:
            case Tag::Pointer:
            case Tag::Reference:
            case Tag::Alloca:
              if (!dependency(Id, Entry.Fields[0], true))
              {
                return false;
              }
              break;
            case Tag::FunctionType:
            case Tag::ArrayConstant:
            case Tag::ClassConstant:
            case Tag::ClassValue:
            case Tag::ArrayElementPointer:
            case Tag::ArrayExtract:
            case Tag::Call:
            case Tag::CString:
            case Tag::Load:
            case Tag::Store:
            case Tag::Add:
            case Tag::Return:
            case Tag::Branch:
            case Tag::ConditionalBranch:
            case Tag::LogicalNot:
            case Tag::LogicalAnd:
            case Tag::LogicalOr:
              for (auto Target : Entry.Fields)
              {
                if (!dependency(Id, Target, Entry.Kind == Tag::FunctionType))
                {
                  return false;
                }
              }
              break;
            case Tag::ArrayValue:
              if (Entry.Fields[0] > 1)
              {
                return Data.fail("Invalid array repetition flag");
              }
              for (std::size_t Index = 1; Index < Entry.Fields.size(); ++Index)
              {
                if (!dependency(Id, Entry.Fields[Index]))
                {
                  return false;
                }
              }
              break;
            case Tag::Compare:
              if (!comparisonPredicateInfo(Entry.Fields[0]))
              {
                return Data.fail("Invalid comparison predicate");
              }
              if (!dependency(Id, Entry.Fields[1]) || !dependency(Id, Entry.Fields[2]))
              {
                return false;
              }
              break;
            case Tag::Parameter:
            case Tag::Block:
              if (!dependency(Id, Entry.Parent))
              {
                return false;
              }
              break;
            default:
              break;
            }
          }
          for (std::size_t Id = 1; Id < Values.size(); ++Id)
          {
            const auto &Entry = record(Id);
            if (Entry.Kind == Tag::Branch || Entry.Kind == Tag::ConditionalBranch)
            {
              const auto &SourceBlock = record(Entry.Parent);
              if (SourceBlock.Kind != Tag::Block || !SourceBlock.Parent || record(SourceBlock.Parent).Kind != Tag::Function)
              {
                return Data.fail("Branch requires a function block");
              }
              for (std::size_t Index = Entry.Kind == Tag::ConditionalBranch ? 1 : 0; Index < Entry.Fields.size(); ++Index)
              {
                const auto &Target = record(Entry.Fields[Index]);
                if (Target.Kind != Tag::Block || Target.Parent != SourceBlock.Parent)
                {
                  return Data.fail("Branch target must be a block in the same function");
                }
              }
            }
            if (Entry.Kind == Tag::Module && (Children[Id].size() != 1 || record(Children[Id][0]).Kind != Tag::Block))
            {
              return Data.fail("Module requires exactly one entry block");
            }
            if (Entry.Kind != Tag::Function)
            {
              continue;
            }
            if (record(Entry.Type).Kind != Tag::FunctionType)
            {
              return Data.fail("Function requires a function signature");
            }
            std::size_t ParameterIndex = 0;
            std::size_t PreviousBlock = 0;
            const auto &Signature = record(Entry.Type).Fields;
            for (auto Child : Children[Id])
            {
              const auto &ChildEntry = record(Child);
              if (ChildEntry.Kind == Tag::Parameter)
              {
                if (PreviousBlock || ParameterIndex + 1 >= Signature.size() || ChildEntry.Fields[0] != ParameterIndex || ChildEntry.Type != Signature[ParameterIndex + 1] || ChildEntry.Fields[1] > 2)
                {
                  return Data.fail("Invalid function parameter metadata");
                }
                ++ParameterIndex;
              }
              else
              {
                if (PreviousBlock && !dependency(Child, PreviousBlock))
                {
                  return false;
                }
                PreviousBlock = Child;
              }
            }
            if (ParameterIndex + 1 != Signature.size())
            {
              return Data.fail("Function parameter count does not match its signature");
            }
            if ((Entry.Fields[3] == 1 && PreviousBlock) || (Entry.Fields[3] == 2 && !PreviousBlock))
            {
              return Data.fail("Native import must be a declaration and native export must have a body");
            }
          }
          return true;
        }

        bool restoreDeclarations()
        {
          core::FrontendContext Frontend(Context.compilationContext());
          for (std::size_t Id = 1; Id < Values.size(); ++Id)
          {
            if (record(Id).Kind != Tag::AST)
            {
              continue;
            }
            auto Result = Data.TextFormat ? parser::tryDeserializeASTText(Frontend, record(Id).Text, Data.astLimits()) : parser::tryDeserializeAST(Frontend, record(Id).Text, Data.astLimits());
            if (!Result.succeeded())
            {
              return Data.fail(Result.Message, astStatus(Result.Status));
            }
            if (!Data.charge(Result.AllocationBytes, 1))
            {
              return false;
            }
            ASTs[Id] = std::make_shared<const parser::ParseResult>(std::move(Result.Parsed));
            parser::ASTWalker{}.walk(ASTs[Id]->Unit->root(), [&](const parser::ASTNodeBase *)
                                     {
                                       return Data.good() ? parser::WalkAction::Continue : parser::WalkAction::Stop;
                                     },
                                     [&](const parser::ASTNodeBase *Node)
                                     {
                                       if (Data.charge(1, sizeof(Node) * 2))
                                       {
                                         ASTNodes[Id].push_back(Node);
                                       }
                                     });
            if (!Data.good())
            {
              return false;
            }
          }
          for (std::size_t Id = 1; Id < Values.size(); ++Id)
          {
            const auto &Entry = record(Id);
            if (!syntax(Entry.Kind) || Entry.Kind == Tag::AST)
            {
              continue;
            }
            const auto ASTId = Entry.Fields[0];
            const auto NodeId = Entry.Fields[1];
            if (NodeId > ASTNodes[ASTId].size())
            {
              return Data.fail("Declaration AST node ID is out of range");
            }
            const auto *Node = ASTNodes[ASTId][NodeId - 1];
            const auto NameValue = Context.namePool().intern(Entry.Text);
            if (Entry.Kind == Tag::ModuleDecl)
            {
              auto &Owner = *const_cast<Module *>(static_cast<const Module *>(Values[Entry.Parent]));
              if (NameValue == Owner.name() && Node == ASTs[ASTId]->Unit->root())
              {
                Declarations[Id] = Builder.createModuleDecl(Owner, *static_cast<const parser::ModuleAST *>(Node));
              }
            }
            else if (Entry.Kind == Tag::FunctionDecl && parser::FunctionDecl::classof(Node))
            {
              Declarations[Id] = Builder.createFunctionDecl(*Declarations[Entry.Parent], NameValue, *static_cast<const parser::FunctionDecl *>(Node));
            }
            else if (Entry.Kind == Tag::ClassDecl && parser::ClassDecl::classof(Node))
            {
              Declarations[Id] = Builder.createClassDecl(*Declarations[Entry.Parent], NameValue, *static_cast<const parser::ClassDecl *>(Node));
            }
            if (!Declarations[Id])
            {
              return Data.fail("Declaration AST kind, generic parameters or name is invalid");
            }
            ModuleArchiveAccess::retain(Declarations[Id]->module(), ASTs[ASTId]);
          }
          return true;
        }

        template <typename T>
        const T *as(std::uint64_t Id) const
        {
          return T::classof(Values[Id]) ? static_cast<const T *>(Values[Id]) : nullptr;
        }

        const Value *own(std::size_t Id, std::unique_ptr<Value> Owner)
        {
          Owners[Id] = std::move(Owner);
          return Owners[Id].get();
        }

        const Value *create(std::size_t Id)
        {
          if (const auto Alias = Data.TypeAliases.find(Id); Alias != Data.TypeAliases.end())
          {
            return Values[Alias->second];
          }
          const auto &Entry = record(Id);
          const auto &Fields = Entry.Fields;
          auto &Types = Context.typePool();
          auto &Constants = Context.constantPool();
          const auto NameValue = [&]()
          {
            return Context.namePool().intern(Entry.Text);
          };
          switch (Entry.Kind)
          {
          case Tag::Meta:
            // Install before checking its self-referential type.
            return Values[Id] = &Types.getType<TypeKind::Meta>();
          case Tag::Void:
            return &Types.getType<TypeKind::Void>();
          case Tag::Bool:
            return &Types.getType<TypeKind::Bool>();
          case Tag::Label:
            return &Types.getType<TypeKind::Label>();
          case Tag::ModuleType:
            return &Types.getType<TypeKind::Module>();
          case Tag::Integer:
            return Fields[0] <= UINT32_MAX && Fields[1] <= 1 ? Types.getType<TypeKind::Integer>(static_cast<std::uint32_t>(Fields[0]), Fields[1] != 0) : nullptr;
          case Tag::Float:
            return Fields[0] <= UINT32_MAX ? Types.getType<TypeKind::Float>(static_cast<std::uint32_t>(Fields[0])) : nullptr;
          case Tag::Array:
            return Types.getType<TypeKind::Array>(*as<Type>(Fields[0]), Fields[1]);
          case Tag::Slice:
            return Fields[1] <= 1 ? Types.getType<TypeKind::Slice>(*as<Type>(Fields[0]), static_cast<AccessKind>(Fields[1])) : nullptr;
          case Tag::Pointer:
            return Fields[1] <= 1 ? Types.getType<TypeKind::Pointer>(*as<Type>(Fields[0]), static_cast<AccessKind>(Fields[1])) : nullptr;
          case Tag::Reference:
            return Fields[1] <= 1 ? Types.getType<TypeKind::Reference>(*as<Type>(Fields[0]), static_cast<AccessKind>(Fields[1])) : nullptr;
          case Tag::Class:
            return Types.createClassType(Context.namePool().intern(Fields.empty() ? std::string_view(Entry.Text) : std::string_view(Entry.Text).substr(0, static_cast<std::size_t>(Fields[0]))));
          case Tag::Enum:
            return Types.createEnumType(NameValue());
          case Tag::Interface:
            return Types.createInterfaceType(NameValue());
          case Tag::FunctionType:
          {
            std::vector<const Type *> Parameters;
            for (std::size_t Index = 1; Index < Fields.size(); ++Index)
            {
              Parameters.push_back(as<Type>(Fields[Index]));
            }
            return Types.getType<TypeKind::Function>(*as<Type>(Fields[0]), Parameters);
          }
          case Tag::IntegerConstant:
            if (const auto *TypeValue = as<IntegerType>(Entry.Type))
            {
              return Constants.getIntegerConstant(*TypeValue, IntegerBits(TypeValue->bitWidth(), Fields));
            }
            return nullptr;
          case Tag::FloatConstant:
            if (const auto *TypeValue = as<FloatType>(Entry.Type))
            {
              return Constants.getFloatConstant(*TypeValue, FloatBits(TypeValue->bitWidth(), Fields[0]));
            }
            return nullptr;
          case Tag::BoolConstant:
            return Fields[0] <= 1 ? &Constants.getBoolConstant(Fields[0] != 0) : nullptr;
          case Tag::StringConstant:
            if (const auto *TypeValue = as<SliceType>(Entry.Type))
            {
              return Constants.getStringConstant(*TypeValue, Entry.Text);
            }
            return nullptr;
          case Tag::ArrayConstant:
            if (const auto *TypeValue = as<ArrayType>(Entry.Type))
            {
              std::vector<const Constant *> Elements;
              for (auto Element : Fields)
              {
                Elements.push_back(as<Constant>(Element));
              }
              return Constants.getArrayConstant(*TypeValue, Elements);
            }
            return nullptr;
          case Tag::ClassConstant:
            if (const auto *TypeValue = as<ClassType>(Entry.Type))
            {
              std::vector<const Constant *> Members;
              for (auto Field : Fields)
              {
                Members.push_back(as<Constant>(Field));
              }
              return Constants.getClassConstant(*TypeValue, Members);
            }
            return nullptr;
          case Tag::ClassValue:
            if (const auto *TypeValue = as<ClassType>(Entry.Type))
            {
              std::vector<const Value *> Members;
              for (auto Field : Fields)
              {
                Members.push_back(Values[Field]);
              }
              return own(Id, Builder.createDetachedClassInstruction(*TypeValue, Members));
            }
            return nullptr;
          case Tag::FieldPointer:
            return own(Id, Builder.createDetachedFieldPointerInstruction(*Values[Fields[1]], static_cast<std::size_t>(Fields[0])));
          case Tag::FieldExtract:
            return own(Id, Builder.createDetachedFieldExtractInstruction(*Values[Fields[1]], static_cast<std::size_t>(Fields[0])));
          case Tag::ArrayValue:
            if (const auto *TypeValue = as<ArrayType>(Entry.Type))
            {
              std::vector<const Value *> Elements;
              for (std::size_t Index = 1; Index < Fields.size(); ++Index)
              {
                Elements.push_back(Values[Fields[Index]]);
              }
              return own(Id, Builder.createDetachedArrayInstruction(*TypeValue, Elements, Fields[0] != 0));
            }
            return nullptr;
          case Tag::ArrayElementPointer:
            return own(Id, Builder.createDetachedArrayElementPointerInstruction(*Values[Fields[0]], *Values[Fields[1]]));
          case Tag::ArrayExtract:
            return own(Id, Builder.createDetachedArrayExtractInstruction(*Values[Fields[0]], *Values[Fields[1]]));
          case Tag::Module:
            if (auto *ModuleValue = Builder.createModule(NameValue()))
            {
              for (auto Class : Fields)
              {
                if (!as<ClassType>(Class) || !Builder.registerClassType(*ModuleValue, *as<ClassType>(Class)))
                {
                  return nullptr;
                }
              }
              return own(Id, Builder.removeModule(*ModuleValue));
            }
            return nullptr;
          case Tag::Function:
          {
            if (Fields[0] > 2 || Fields[1] > 1 || Fields[2] > 1 || Fields[3] > 2)
            {
              return nullptr;
            }
            std::vector<ParameterKind> Kinds;
            std::vector<Name> Names;
            for (auto Child : Children[Id])
            {
              if (record(Child).Kind == Tag::Parameter)
              {
                Kinds.push_back(static_cast<ParameterKind>(record(Child).Fields[1]));
                Names.push_back(Context.namePool().intern(record(Child).Text));
              }
            }
            auto FunctionValue = Builder.createFunction(NameValue(), *as<FunctionType>(Entry.Type), Kinds, Names, static_cast<CallingConvention>(Fields[0]), static_cast<LanguageLinkage>(Fields[1]), static_cast<FunctionBinding>(Fields[3]));
            if (!FunctionValue || !Builder.setFunctionVisibility(*FunctionValue, static_cast<VisibilityKind>(Fields[2])))
            {
              return nullptr;
            }
            if (Fields.size() == 6)
            {
              const auto *Owner = as<ClassType>(Fields[4]);
              if (!Owner || !(Fields[5] == 0 ? Builder.setClassMethod(*Owner, *FunctionValue) : Builder.setFieldInitializer(*Owner, static_cast<std::size_t>(Fields[5] - 1), *FunctionValue)))
              {
                return nullptr;
              }
            }
            return own(Id, std::move(FunctionValue));
          }
          case Tag::Parameter:
            return static_cast<const Function *>(Values[Entry.Parent])->parameters()[Fields[0]].get();
          case Tag::Block:
          {
            auto *Parent = const_cast<Value *>(Values[Entry.Parent]);
            if (Module::classof(Parent))
            {
              return &static_cast<Module *>(Parent)->entryBlock();
            }
            if (Function::classof(Parent))
            {
              return Builder.createBasicBlock(*static_cast<Function *>(Parent));
            }
            return own(Id, Builder.createBasicBlock());
          }
          case Tag::Call:
          {
            std::vector<const Value *> Arguments;
            for (std::size_t Index = 1; Index < Fields.size(); ++Index)
            {
              Arguments.push_back(Values[Fields[Index]]);
            }
            return own(Id, Builder.createDetachedCallInstruction(*Values[Fields[0]], Arguments));
          }
          case Tag::CString:
            return as<StringConstant>(Fields[0]) ? own(Id, Builder.createDetachedCStringInstruction(*as<StringConstant>(Fields[0]))) : nullptr;
          case Tag::Alloca:
            return own(Id, Builder.createDetachedAllocaInstruction(*as<Type>(Fields[0])));
          case Tag::Load:
            return own(Id, Builder.createDetachedLoadInstruction(*Values[Fields[0]]));
          case Tag::Store:
            return own(Id, Builder.createDetachedStoreInstruction(*Values[Fields[0]], *Values[Fields[1]]));
          case Tag::Add:
            return own(Id, Builder.createDetachedAddInstruction(*Values[Fields[0]], *Values[Fields[1]]));
          case Tag::LogicalNot:
            return own(Id, Builder.createDetachedLogicalNotInstruction(*Values[Fields[0]]));
          case Tag::LogicalAnd:
            return own(Id, Builder.createDetachedLogicalAndInstruction(*Values[Fields[0]], *Values[Fields[1]]));
          case Tag::LogicalOr:
            return own(Id, Builder.createDetachedLogicalOrInstruction(*Values[Fields[0]], *Values[Fields[1]]));
          case Tag::Compare:
            return own(Id, Builder.createDetachedCompareInstruction(comparisonPredicateInfo(Fields[0])->Predicate, *Values[Fields[1]], *Values[Fields[2]]));
          case Tag::Return:
            return own(Id, Builder.createDetachedReturnInstruction(Fields.empty() ? nullptr : Values[Fields[0]]));
          case Tag::Branch:
            return own(Id, Builder.createDetachedBranchInstruction(*as<BasicBlock>(Fields[0])));
          case Tag::ConditionalBranch:
            return own(Id, Builder.createDetachedConditionalBranchInstruction(*Values[Fields[0]], *as<BasicBlock>(Fields[1]), *as<BasicBlock>(Fields[2])));
          case Tag::AST:
          case Tag::ModuleDecl:
          case Tag::FunctionDecl:
          case Tag::ClassDecl:
            break;
          }
          return nullptr;
        }

        IRContext &Context;
        State &Data;
        IRBuilder Builder;
        std::vector<const Value *> Values;
        std::vector<std::unique_ptr<Value>> Owners;
        std::vector<std::vector<std::size_t>> Children;
        std::vector<std::vector<std::size_t>> Users;
        std::vector<std::size_t> Pending;
        std::vector<std::size_t> Depths;
        std::vector<std::shared_ptr<const parser::ParseResult>> ASTs;
        std::vector<std::vector<const parser::ASTNodeBase *>> ASTNodes;
        std::vector<Decl *> Declarations;
        std::size_t ValueCount = 0;
    };
  } // namespace

  bool collect(const Module &ModuleValue, State &Data, std::span<const parser::ParseResult *const> Sources)
  {
    return Collector(ModuleValue, Data, Sources).run();
  }

  Module *restore(IRContext &Context, State &Data)
  {
    return Restorer(Context, Data).run();
  }
} // namespace ink::ir::archive

namespace ink::ir
{
  namespace
  {
    ModuleSerializeResult serialize(const Module &ModuleValue, ModuleArchiveLimits Limits, bool Binary, std::span<const parser::ParseResult *const> Sources)
    {
      archive::State Data(Limits);
      Data.TextFormat = !Binary;
      if (!archive::collect(ModuleValue, Data, Sources))
      {
        return {{}, Data.Status, std::move(Data.Message)};
      }
      auto Bytes = Binary ? archive::writeBinary(Data) : archive::writeText(Data);
      return {Data.good() ? std::move(Bytes) : std::string{}, Data.Status, std::move(Data.Message)};
    }

    ModuleDeserializeResult deserialize(IRContext &Context, std::string_view Bytes, ModuleArchiveLimits Limits, bool Binary)
    {
      archive::State Data(Limits);
      Data.TextFormat = !Binary;
      if (Bytes.size() > Limits.MaxArchiveBytes)
      {
        return {nullptr, ModuleArchiveStatus::LimitExceeded, "Module archive byte limit exceeded"};
      }
      const bool Read = Binary ? archive::readBinary(Bytes, Data) : archive::readText(Bytes, Data);
      auto *ModuleValue = Read ? archive::restore(Context, Data) : nullptr;
      return {ModuleValue, Data.Status, std::move(Data.Message)};
    }
  } // namespace

  ModuleSerializeResult serializeModuleText(const Module &ModuleValue, ModuleArchiveLimits Limits, std::span<const parser::ParseResult *const> Sources)
  {
    return serialize(ModuleValue, Limits, false, Sources);
  }

  ModuleSerializeResult serializeModuleBinary(const Module &ModuleValue, ModuleArchiveLimits Limits, std::span<const parser::ParseResult *const> Sources)
  {
    return serialize(ModuleValue, Limits, true, Sources);
  }

  ModuleDeserializeResult deserializeModuleText(IRContext &Context, std::string_view Text, ModuleArchiveLimits Limits)
  {
    return deserialize(Context, Text, Limits, false);
  }

  ModuleDeserializeResult deserializeModuleBinary(IRContext &Context, std::string_view Bytes, ModuleArchiveLimits Limits)
  {
    return deserialize(Context, Bytes, Limits, true);
  }
} // namespace ink::ir
