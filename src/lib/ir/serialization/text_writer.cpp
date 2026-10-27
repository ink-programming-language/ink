#include "text_format.h"

#include "ink/core/config.h"

#include <charconv>
#include <limits>
#include <string>
#include <unordered_set>
#include <utility>

namespace ink::ir::detail
{
  namespace
  {
    bool isPlainName(std::string_view Name)
    {
      if (Name.empty() || (Name.front() >= '0' && Name.front() <= '9') || Name.find("...") != std::string_view::npos || Name.find("->") != std::string_view::npos)
      {
        return false;
      }
      for (const unsigned char Character : Name)
      {
        if (!((Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z') || (Character >= '0' && Character <= '9') || Character == '_' || Character == '.' || Character == '$' || Character == '-'))
        {
          return false;
        }
      }
      return true;
    }

    struct Symbol
    {
        std::string Text;
        bool Quoted = false;
        bool ExplicitName = false;
    };

    struct SymbolScope
    {
        std::unordered_set<std::string> Used;
        std::size_t NextAnonymous = 0;
    };

    enum class RenderKind
    {
      Text,
      Number,
      Type,
      Value,
      TypedValue,
    };

    struct RenderStep
    {
        RenderKind Kind;
        std::uint64_t Number = 0;
        std::string_view Text;
    };

    class AssemblyWriter
    {
      public:
        AssemblyWriter(const Model &Data, std::size_t MaxBytes)
            : Data(Data),
              MaxBytes(MaxBytes),
              Symbols(Data.Records.size()),
              Parameters(Data.Records.size()),
              Blocks(Data.Records.size()),
              Instructions(Data.Records.size()),
              Fields(Data.Records.size()),
              Referenced(Data.Records.size(), false)
        {
        }

        IRSerializationResult<std::string> write()
        {
          index();
          append("ink_ir ");
          append(INK_IR_VERSION);
          append("\ntarget(pointer_bits = ");
          number(Data.PointerBits);
          append(", byte_order = ");
          append(enumName<target::ByteOrder>(Data.ByteOrder));
          append(", native_abi = ");
          number(Data.NativeAbi ? 1 : 0);
          append(", alignments = [");
          for (std::size_t Index = 0; Index < Data.Alignments.size(); ++Index)
          {
            separator(Index);
            number(Data.Alignments[Index]);
          }
          append("])\n");
          keyword(RecordKind::Module);
          strings(Data.Records[0].Strings);
          bool Attributes = false;
          for (const auto Index : {schema::Module::Init, schema::Module::Deinit})
          {
            if (Data.Records[0].References[Index] != NullId)
            {
              attribute(Attributes, Index == schema::Module::Init ? "init = " : "deinit = ");
              reference(Data.Records[0].References[Index]);
            }
          }
          endAttributes(Attributes);
          append("\n{\n");
          bool First = true;
          for (std::uint32_t Id = 1; Id < Data.Records.size() && !Failed; ++Id)
          {
            if (isNominal(Data.Records[Id].Kind))
            {
              topSeparator(First);
              nominal(Id);
            }
          }
          for (std::uint32_t Id = 1; Id < Data.Records.size() && !Failed; ++Id)
          {
            if (Data.Records[Id].Kind == RecordKind::Global)
            {
              topSeparator(First);
              global(Id);
            }
          }
          for (std::uint32_t Id = 1; Id < Data.Records.size() && !Failed; ++Id)
          {
            if (Data.Records[Id].Kind == RecordKind::Function)
            {
              topSeparator(First);
              function(Id);
            }
          }
          for (std::uint32_t Id = 1; Id < Data.Records.size() && !Failed; ++Id)
          {
            const auto &Entry = Data.Records[Id];
            if (Entry.Kind == RecordKind::Import)
            {
              topSeparator(First);
              append("  ");
              keyword(RecordKind::Import);
              reference(Id);
              append(" : ");
              type(Entry.References[schema::Import::Type]);
              append(" from ");
              strings(Entry.Strings);
              append('\n');
            }
            else if (Entry.Kind == RecordKind::Export)
            {
              topSeparator(First);
              append("  ");
              keyword(RecordKind::Export);
              const auto Declaration = Entry.References[schema::Export::Declaration];
              if (isType(Data.Records[Declaration].Kind))
              {
                append("type ");
                type(Declaration);
              }
              else
              {
                reference(Declaration);
              }
              append(" as ");
              string(Entry.Strings[schema::Export::Name]);
              append('\n');
            }
          }
          append("}\n");
          if (Failed)
          {
            return std::unexpected(IRSerializationError{IRSerializationErrorCode::ResourceLimit, 0, "Text output exceeds MaxBytes"});
          }
          return std::move(Output);
        }

      private:
        void append(std::string_view Text)
        {
          if (Failed || Text.size() > MaxBytes - Output.size())
          {
            Failed = true;
            return;
          }
          Output.append(Text);
        }

        void append(char Character)
        {
          append(std::string_view(&Character, 1));
        }

        void keyword(RecordKind Kind)
        {
          append(recordTextName(Kind));
          append(' ');
        }

        void number(std::uint64_t Number)
        {
          char Buffer[20];
          const auto Result = std::to_chars(Buffer, Buffer + sizeof(Buffer), Number);
          append(std::string_view(Buffer, static_cast<std::size_t>(Result.ptr - Buffer)));
        }

        void hexadecimal(std::uint64_t Number, bool Pad = false)
        {
          constexpr char Digits[] = "0123456789ABCDEF";
          char Buffer[16];
          std::size_t Begin = sizeof(Buffer);
          do
          {
            Buffer[--Begin] = Digits[Number & 15];
            Number >>= 4;
          } while (Number != 0 || (Pad && Begin != 0));
          append(std::string_view(Buffer + Begin, sizeof(Buffer) - Begin));
        }

        void string(std::string_view Text)
        {
          constexpr char Digits[] = "0123456789ABCDEF";
          append('"');
          for (const unsigned char Byte : Text)
          {
            if (Byte >= 0x20 && Byte <= 0x7E && Byte != '"' && Byte != '\\')
            {
              append(static_cast<char>(Byte));
            }
            else
            {
              append('\\');
              append(Digits[Byte >> 4]);
              append(Digits[Byte & 15]);
            }
            if (Failed)
            {
              break;
            }
          }
          append('"');
        }

        void strings(const std::vector<std::string> &Values)
        {
          append('[');
          for (std::size_t Index = 0; Index < Values.size() && !Failed; ++Index)
          {
            separator(Index);
            string(Values[Index]);
          }
          append(']');
        }

        void separator(std::size_t Index)
        {
          if (Index != 0)
          {
            append(", ");
          }
        }

        void topSeparator(bool &First)
        {
          if (!First)
          {
            append('\n');
          }
          First = false;
        }

        void attribute(bool &Started, std::string_view Name)
        {
          append(Started ? ", " : " [");
          Started = true;
          append(Name);
        }

        void endAttributes(bool Started)
        {
          if (Started)
          {
            append(']');
          }
        }

        void nameAttribute(std::uint32_t Id, bool &Started)
        {
          if (Symbols[Id].ExplicitName)
          {
            attribute(Started, "name = ");
            string(Data.Records[Id].Strings[schema::Named::Name]);
          }
        }

        void assign(std::uint32_t Id, SymbolScope &Scope)
        {
          const auto &Entry = Data.Records[Id];
          const bool HasName = isNominal(Entry.Kind) || Entry.Kind == RecordKind::Function || Entry.Kind == RecordKind::Global || Entry.Kind == RecordKind::Parameter || Entry.Kind == RecordKind::Block || Entry.Kind == RecordKind::Alloca;
          const std::string_view Name = HasName ? std::string_view(Entry.Strings[schema::Named::Name]) : std::string_view{};
          auto &Result = Symbols[Id];
          if (Name.empty())
          {
            do
            {
              Result.Text = std::to_string(Scope.NextAnonymous++);
            } while (!Scope.Used.insert(Result.Text).second);
            return;
          }
          Result.Text = Name;
          for (std::size_t Suffix = 1; !Scope.Used.insert(Result.Text).second; ++Suffix)
          {
            Result.Text = std::string(Name) + "." + std::to_string(Suffix);
            Result.ExplicitName = true;
          }
          Result.Quoted = !isPlainName(Result.Text);
        }

        void index()
        {
          SymbolScope Globals;
          SymbolScope Types;
          for (const auto &Entry : Data.Records)
          {
            for (auto Reference : Entry.References)
            {
              if (Reference != NullId)
              {
                Referenced[Reference] = true;
              }
            }
          }
          for (std::uint32_t Id = 1; Id < Data.Records.size(); ++Id)
          {
            const auto &Entry = Data.Records[Id];
            if (isNominal(Entry.Kind))
            {
              assign(Id, Types);
            }
            else if (Entry.Kind == RecordKind::Function || Entry.Kind == RecordKind::Global || Entry.Kind == RecordKind::Import)
            {
              assign(Id, Globals);
            }
            else if (Entry.Kind == RecordKind::Parameter)
            {
              Parameters[Entry.References[schema::Parameter::Function]].push_back(Id);
            }
            else if (Entry.Kind == RecordKind::Block)
            {
              Blocks[Entry.References[schema::Block::Function]].push_back(Id);
            }
            else if (Entry.Kind == RecordKind::Field)
            {
              Fields[Entry.References[schema::Field::Owner]].push_back(Id);
            }
            else if (isInstruction(Entry.Kind))
            {
              Instructions[Entry.References[schema::Instruction::Block]].push_back(Id);
            }
          }
          for (std::uint32_t Id = 1; Id < Data.Records.size(); ++Id)
          {
            if (Data.Records[Id].Kind != RecordKind::Function)
            {
              continue;
            }
            SymbolScope Locals;
            for (auto Parameter : Parameters[Id])
            {
              assign(Parameter, Locals);
            }
            for (auto Block : Blocks[Id])
            {
              assign(Block, Locals);
              for (auto Instruction : Instructions[Block])
              {
                if (hasResult(Instruction))
                {
                  assign(Instruction, Locals);
                }
              }
            }
          }
        }

        void symbol(std::uint32_t Id)
        {
          if (Symbols[Id].Quoted)
          {
            string(Symbols[Id].Text);
          }
          else
          {
            append(Symbols[Id].Text);
          }
        }

        void reference(std::uint32_t Id)
        {
          const auto Kind = Data.Records[Id].Kind;
          append(Kind == RecordKind::Function || Kind == RecordKind::Global || Kind == RecordKind::Import ? '@' : '%');
          symbol(Id);
        }

        std::uint32_t valueType(std::uint32_t Id) const
        {
          const auto &Entry = Data.Records[Id];
          if (Entry.Kind == RecordKind::Parameter)
          {
            const auto &Function = Data.Records[Entry.References[schema::Parameter::Function]];
            const auto &Signature = Data.Records[Function.References[schema::Function::Signature]];
            return Signature.References[schema::FunctionType::ParametersBegin + static_cast<std::size_t>(Entry.Numbers[schema::Parameter::Index])];
          }
          return Entry.References[schema::Typed::Type];
        }

        void type(std::uint32_t Id)
        {
          render({RenderKind::Type, Id, {}});
        }

        void typedValue(std::uint32_t Id)
        {
          render({RenderKind::TypedValue, Id, {}});
        }

        void value(std::uint32_t Id)
        {
          render({RenderKind::Value, Id, {}});
        }

        void render(RenderStep First)
        {
          std::vector<RenderStep> Work{First};
          while (!Work.empty() && !Failed)
          {
            const auto Step = Work.back();
            Work.pop_back();
            if (Step.Kind == RenderKind::Text)
            {
              append(Step.Text);
              continue;
            }
            if (Step.Kind == RenderKind::Number)
            {
              number(Step.Number);
              continue;
            }
            const auto Id = static_cast<std::uint32_t>(Step.Number);
            const auto &Entry = Data.Records[Id];
            if (Step.Kind == RenderKind::TypedValue)
            {
              Work.push_back({RenderKind::Value, Id, {}});
              Work.push_back({RenderKind::Text, 0, " "});
              Work.push_back({RenderKind::Type, valueType(Id), {}});
              continue;
            }
            if (Step.Kind == RenderKind::Type)
            {
              renderType(Id, Work);
              continue;
            }
            switch (Entry.Kind)
            {
            case RecordKind::BoolConstant:
              append(Entry.Numbers[schema::BoolConstant::Value] ? "true" : "false");
              break;
            case RecordKind::IntegerConstant:
              integer(Entry);
              break;
            case RecordKind::FloatConstant:
              append(recordTextName(Entry.Kind));
              append("(0x");
              hexadecimal(Entry.Numbers[schema::FloatConstant::Bits]);
              append(')');
              break;
            case RecordKind::NullPointerConstant:
              append(recordTextName(Entry.Kind));
              break;
            case RecordKind::StringConstant:
              append(recordTextName(Entry.Kind));
              string(Entry.Strings[schema::StringConstant::Bytes]);
              break;
            case RecordKind::ArrayConstant:
              append(recordTextName(Entry.Kind));
              Work.push_back({RenderKind::Text, 0, "]"});
              for (std::size_t Index = Entry.References.size(); Index > schema::ArrayConstant::ElementsBegin; --Index)
              {
                Work.push_back({RenderKind::TypedValue, Entry.References[Index - 1], {}});
                if (Index > schema::ArrayConstant::ElementsBegin + 1)
                {
                  Work.push_back({RenderKind::Text, 0, ", "});
                }
              }
              break;
            default:
              reference(Id);
              break;
            }
          }
        }

        void renderType(std::uint32_t Id, std::vector<RenderStep> &Work)
        {
          const auto &Entry = Data.Records[Id];
          if (isNominal(Entry.Kind))
          {
            reference(Id);
            return;
          }
          if (Entry.Kind != RecordKind::MetaType && Entry.Kind != RecordKind::VariadicType && !Entry.Numbers[schema::Type::Mutable])
          {
            keyword(RecordKind::QualifiedType);
          }
          switch (Entry.Kind)
          {
          case RecordKind::BoolType:
          case RecordKind::VoidType:
          case RecordKind::NullPointerType:
          case RecordKind::MetaType:
          case RecordKind::VariadicType:
            append(recordTextName(Entry.Kind));
            break;
          case RecordKind::IntegerType:
            append(Entry.Numbers[schema::IntegerType::Signed] ? 'i' : 'u');
            number(Entry.Numbers[schema::IntegerType::Width]);
            break;
          case RecordKind::FloatType:
            append(recordTextName(Entry.Kind));
            number(Entry.Numbers[schema::FloatType::Width]);
            break;
          case RecordKind::PointerType:
          case RecordKind::ReferenceType:
            append(recordTextName(Entry.Kind));
            Work.push_back({RenderKind::Type, Entry.References[schema::PointerType::Element], {}});
            break;
          case RecordKind::QualifiedType:
            reference(Entry.References[schema::QualifiedType::Declaration]);
            break;
          case RecordKind::ArrayType:
            append(recordTextName(Entry.Kind));
            number(Entry.Numbers[schema::ArrayType::Count]);
            append(" x ");
            Work.push_back({RenderKind::Text, 0, "]"});
            Work.push_back({RenderKind::Type, Entry.References[schema::ArrayType::Element], {}});
            break;
          case RecordKind::FunctionType:
            append(recordTextName(Entry.Kind));
            append('(');
            Work.push_back({RenderKind::Type, Entry.References[schema::FunctionType::Return], {}});
            Work.push_back({RenderKind::Text, 0, ") -> "});
            for (std::size_t Index = Entry.References.size(); Index > schema::FunctionType::ParametersBegin; --Index)
            {
              const auto ParameterIndex = Index - schema::FunctionType::ParametersBegin - 1;
              if (Entry.Numbers[schema::FunctionType::VariadicsBegin + ParameterIndex])
              {
                Work.push_back({RenderKind::Text, 0, "..."});
              }
              Work.push_back({RenderKind::Type, Entry.References[schema::FunctionType::ParametersBegin + ParameterIndex], {}});
              if (ParameterIndex != 0)
              {
                Work.push_back({RenderKind::Text, 0, ", "});
              }
            }
            break;
          default:
            break;
          }
        }

        void integer(const Record &Entry)
        {
          const auto &Type = Data.Records[Entry.References[schema::IntegerConstant::Type]];
          const auto Width = Type.Numbers[schema::IntegerType::Width];
          if (Width > 64)
          {
            append("0x");
            bool First = true;
            for (auto Index = Entry.Numbers.size(); Index != 0; --Index)
            {
              if (First && Entry.Numbers[Index - 1] == 0 && Index != 1)
              {
                continue;
              }
              hexadecimal(Entry.Numbers[Index - 1], !First);
              First = false;
            }
            return;
          }
          const auto Bits = Entry.Numbers[schema::IntegerConstant::WordsBegin];
          if (Type.Numbers[schema::IntegerType::Signed] && (Bits & (std::uint64_t{1} << (Width - 1))))
          {
            append('-');
            const auto Mask = Width == 64 ? std::numeric_limits<std::uint64_t>::max() : (std::uint64_t{1} << Width) - 1;
            number((~Bits + 1) & Mask);
          }
          else
          {
            number(Bits);
          }
        }

        void nominal(std::uint32_t Id)
        {
          const auto &Entry = Data.Records[Id];
          append("  ");
          reference(Id);
          append(" = type ");
          append(recordTextName(Entry.Kind));
          bool Attributes = false;
          attribute(Attributes, "size = ");
          number(Entry.Numbers[schema::NominalType::Size]);
          attribute(Attributes, "alignment = ");
          number(Entry.Numbers[schema::NominalType::Alignment]);
          if (Entry.Numbers[schema::NominalType::Frozen])
          {
            attribute(Attributes, "frozen");
          }
          nameAttribute(Id, Attributes);
          endAttributes(Attributes);
          append("\n  {\n");
          for (auto FieldId : Fields[Id])
          {
            const auto &Field = Data.Records[FieldId];
            append("    ");
            keyword(RecordKind::Field);
            type(Field.References[schema::Field::Type]);
            append(' ');
            string(Field.Strings[schema::Field::Name]);
            append(" [offset = ");
            number(Field.Numbers[schema::Field::Offset]);
            append("] = ");
            if (Field.References[schema::Field::Initializer] == NullId)
            {
              append("none");
            }
            else
            {
              typedValue(Field.References[schema::Field::Initializer]);
            }
            append('\n');
          }
          append("  }\n");
        }

        void global(std::uint32_t Id)
        {
          const auto &Entry = Data.Records[Id];
          append("  ");
          reference(Id);
          append(" = ");
          keyword(RecordKind::Global);
          type(Entry.References[schema::Global::Type]);
          append(" = ");
          if (Entry.References[schema::Global::Initializer] == NullId)
          {
            append("none");
          }
          else
          {
            typedValue(Entry.References[schema::Global::Initializer]);
          }
          bool Attributes = false;
          if (Entry.Numbers[schema::Global::StringStorage])
          {
            attribute(Attributes, "string_storage");
          }
          nameAttribute(Id, Attributes);
          endAttributes(Attributes);
          append('\n');
        }

        void function(std::uint32_t Id)
        {
          const auto &Entry = Data.Records[Id];
          const auto &Signature = Data.Records[Entry.References[schema::Function::Signature]];
          append("  ");
          append(Blocks[Id].empty() ? "declare" : recordTextName(RecordKind::Function));
          append(' ');
          if (Entry.Numbers[schema::Function::Linkage] != encodeEnum(SymbolLinkage::Local))
          {
            append(enumName<SymbolLinkage>(Entry.Numbers[schema::Function::Linkage]));
            append(' ');
          }
          if (Entry.Numbers[schema::Function::Language] != encodeEnum(LanguageLinkage::Ink))
          {
            string(enumName<LanguageLinkage>(Entry.Numbers[schema::Function::Language]));
            append(' ');
          }
          type(Signature.References[schema::FunctionType::Return]);
          append(' ');
          reference(Id);
          append('(');
          for (std::size_t Index = 0; Index < Parameters[Id].size(); ++Index)
          {
            separator(Index);
            type(Signature.References[schema::FunctionType::ParametersBegin + Index]);
            if (Signature.Numbers[schema::FunctionType::VariadicsBegin + Index])
            {
              append("...");
            }
            append(' ');
            reference(Parameters[Id][Index]);
            bool ParameterAttributes = false;
            nameAttribute(Parameters[Id][Index], ParameterAttributes);
            endAttributes(ParameterAttributes);
          }
          append(')');
          bool Attributes = false;
          if (Entry.Numbers[schema::Function::Visibility] != encodeEnum(FunctionVisibility::Public))
          {
            attribute(Attributes, enumName<FunctionVisibility>(Entry.Numbers[schema::Function::Visibility]));
          }
          if (Entry.References[schema::Function::Owner] != NullId)
          {
            attribute(Attributes, "owner = ");
            reference(Entry.References[schema::Function::Owner]);
          }
          if (Entry.Numbers[schema::Function::Dispatch] != encodeEnum(MethodDispatch::Direct))
          {
            attribute(Attributes, enumName<MethodDispatch>(Entry.Numbers[schema::Function::Dispatch]));
          }
          if (Entry.Numbers[schema::Function::Override])
          {
            attribute(Attributes, "override");
          }
          if (Entry.Numbers[schema::Function::Final])
          {
            attribute(Attributes, "final");
          }
          if (Entry.Numbers[schema::Function::Slot] != NoVirtualSlot)
          {
            attribute(Attributes, "slot = ");
            number(Entry.Numbers[schema::Function::Slot]);
          }
          if (!Signature.Numbers[schema::FunctionType::Mutable])
          {
            attribute(Attributes, "signature_const");
          }
          nameAttribute(Id, Attributes);
          endAttributes(Attributes);
          append('\n');
          if (Blocks[Id].empty())
          {
            return;
          }
          append("  {\n");
          for (auto Block : Blocks[Id])
          {
            append("  ");
            symbol(Block);
            append(':');
            bool BlockAttributes = false;
            nameAttribute(Block, BlockAttributes);
            endAttributes(BlockAttributes);
            append('\n');
            for (auto Instruction : Instructions[Block])
            {
              instruction(Instruction);
            }
          }
          append("  }\n");
        }

        void instruction(std::uint32_t Id)
        {
          const auto &Entry = Data.Records[Id];
          const auto &R = Entry.References;
          append("    ");
          if (hasResult(Id))
          {
            reference(Id);
            append(" = ");
          }
          keyword(Entry.Kind);
          bool Attributes = false;
          switch (Entry.Kind)
          {
          case RecordKind::Alloca:
            type(Data.Records[R[schema::Alloca::Type]].References[schema::PointerType::Element]);
            if (Entry.Numbers[schema::Alloca::Initialized])
            {
              attribute(Attributes, "initialized");
            }
            nameAttribute(Id, Attributes);
            break;
          case RecordKind::Load:
            type(R[schema::Load::Type]);
            append(", ");
            typedValue(R[schema::Load::Pointer]);
            break;
          case RecordKind::Store:
            typedValue(R[schema::Store::Value]);
            append(", ");
            typedValue(R[schema::Store::Pointer]);
            break;
          case RecordKind::Neg:
            typedValue(R[schema::Neg::Operand]);
            break;
          case RecordKind::ICmp:
            append(enumName<ICmpPredicate>(Entry.Numbers[schema::ICmp::Predicate]));
            append(' ');
            typedValue(R[schema::ICmp::Left]);
            append(", ");
            typedValue(R[schema::ICmp::Right]);
            break;
          case RecordKind::Branch:
            append("label ");
            reference(R[schema::Branch::Target]);
            break;
          case RecordKind::CondBranch:
            typedValue(R[schema::CondBranch::Condition]);
            append(", label ");
            reference(R[schema::CondBranch::TrueTarget]);
            append(", label ");
            reference(R[schema::CondBranch::FalseTarget]);
            break;
          case RecordKind::Return:
            if (R[schema::Return::Value] == NullId)
            {
              append(recordTextName(RecordKind::VoidType));
            }
            else
            {
              typedValue(R[schema::Return::Value]);
            }
            break;
          case RecordKind::Call:
            type(R[schema::Call::Type]);
            append(' ');
            value(R[schema::Call::Callee]);
            append('(');
            for (std::size_t Index = schema::Call::ArgumentsBegin; Index < R.size(); ++Index)
            {
              const auto ArgumentIndex = Index - schema::Call::ArgumentsBegin;
              separator(ArgumentIndex);
              typedValue(R[Index]);
              if (!Entry.Strings[schema::Call::ArgumentNamesBegin + ArgumentIndex].empty())
              {
                append(" [name = ");
                string(Entry.Strings[schema::Call::ArgumentNamesBegin + ArgumentIndex]);
                append(']');
              }
            }
            append(')');
            if (Data.Records[R[schema::Call::Callee]].Kind != RecordKind::Function || Data.Records[R[schema::Call::Callee]].References[schema::Function::Signature] != R[schema::Call::Signature])
            {
              attribute(Attributes, "signature = ");
              type(R[schema::Call::Signature]);
            }
            break;
          case RecordKind::GetElementPtr:
            type(R[schema::GetElementPtr::SourceType]);
            append(", ");
            typedValue(R[schema::GetElementPtr::Pointer]);
            for (std::size_t Index = schema::GetElementPtr::IndicesBegin; Index < R.size(); ++Index)
            {
              append(", ");
              typedValue(R[Index]);
            }
            if (Entry.Numbers[schema::GetElementPtr::InBounds])
            {
              attribute(Attributes, "inbounds");
            }
            break;
          default:
            break;
          }
          endAttributes(Attributes);
          append('\n');
        }

        bool hasResult(std::uint32_t Id) const
        {
          const auto &Entry = Data.Records[Id];
          return Data.Records[Entry.References[schema::Instruction::Type]].Kind != RecordKind::VoidType || (Entry.Kind == RecordKind::Call && Referenced[Id]);
        }

        const Model &Data;
        std::size_t MaxBytes;
        std::string Output;
        std::vector<Symbol> Symbols;
        std::vector<std::vector<std::uint32_t>> Parameters;
        std::vector<std::vector<std::uint32_t>> Blocks;
        std::vector<std::vector<std::uint32_t>> Instructions;
        std::vector<std::vector<std::uint32_t>> Fields;
        std::vector<bool> Referenced;
        bool Failed = false;
    };
  } // namespace

  IRSerializationResult<std::string> writeAssembly(const Model &Data, std::size_t MaxBytes)
  {
    return AssemblyWriter(Data, MaxBytes).write();
  }
} // namespace ink::ir::detail
