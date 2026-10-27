#include "model.h"
#include "load_transaction.h"

#include "ink/core/name/name_pool.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/instruction/branch_instruction.h"
#include "ink/ir/instruction/call_instruction.h"
#include "ink/ir/instruction/conditional_branch_instruction.h"
#include "ink/ir/instruction/getelementptr_instruction.h"
#include "ink/ir/instruction/icmp_instruction.h"
#include "ink/ir/instruction/load_instruction.h"
#include "ink/ir/instruction/neg_instruction.h"
#include "ink/ir/instruction/return_instruction.h"
#include "ink/ir/instruction/store_instruction.h"

#include <algorithm>
#include <optional>
#include <unordered_map>
#include <unordered_set>

namespace ink::ir::detail
{
  struct IRModelAccess
  {
      static void setInitialized(AllocaInstruction &Value, bool Initialized) noexcept
      {
        Value.Initialized = Initialized;
      }

      static bool isStringStorage(const Module &Owner, const GlobalVariable &Value) noexcept
      {
        const auto *Initializer = Value.initializer();
        if (!Initializer || !StringConstant::classof(Initializer))
        {
          return false;
        }
        const auto It = Owner.StringStorages.find(static_cast<const StringConstant *>(Initializer));
        return It != Owner.StringStorages.end() && It->second == &Value;
      }

      static bool registerStringStorage(Module &Owner, const GlobalVariable &Value)
      {
        const auto *Initializer = Value.initializer();
        if (!Initializer || !StringConstant::classof(Initializer) || Value.variableType()->isMutable() || !Value.type()->isMutable())
        {
          return false;
        }
        return Owner.StringStorages.emplace(static_cast<const StringConstant *>(Initializer), &Value).second;
      }
  };

  namespace
  {
    std::string nameText(core::Name Name)
    {
      return std::string(core::NamePool::instance().text(Name));
    }

    std::array<std::uint64_t, 5> targetAlignments(const target::TargetContext &Target)
    {
      return {Target.integerAlignment(8), Target.integerAlignment(32), Target.integerAlignment(64), Target.integerAlignment(128), Target.floatAlignment(64)};
    }

    IRSerializationResult<void> validateModel(const Model &Data, const IRReadOptions &Options)
    {
      auto Reject = [](std::string Message, IRSerializationErrorCode Code = IRSerializationErrorCode::InvalidIR) -> IRSerializationResult<void>
      {
        return std::unexpected(IRSerializationError{Code, 0, std::move(Message)});
      };
      if (Data.Records.empty() || Data.Records[0].Kind != RecordKind::Module)
      {
        return Reject("The first IR record must describe the module");
      }
      if (Data.Records.size() > Options.MaxRecords || Data.Records.size() >= NullId)
      {
        return Reject("IR record limit exceeded", IRSerializationErrorCode::ResourceLimit);
      }
      std::size_t OperandCount = 0;
      std::size_t StringBytes = 0;
      for (std::size_t Index = 0; Index < Data.Records.size(); ++Index)
      {
        const auto &Entry = Data.Records[Index];
        const auto &N = Entry.Numbers;
        const auto &R = Entry.References;
        const auto &S = Entry.Strings;
        if (Entry.Numbers.size() > Options.MaxOperands - OperandCount)
        {
          return Reject("IR operand limit exceeded", IRSerializationErrorCode::ResourceLimit);
        }
        OperandCount += Entry.Numbers.size();
        if (Entry.References.size() > Options.MaxOperands - OperandCount)
        {
          return Reject("IR operand limit exceeded", IRSerializationErrorCode::ResourceLimit);
        }
        OperandCount += Entry.References.size();
        if (Entry.Strings.size() > Options.MaxOperands - OperandCount)
        {
          return Reject("IR operand limit exceeded", IRSerializationErrorCode::ResourceLimit);
        }
        OperandCount += Entry.Strings.size();
        for (const auto &Text : Entry.Strings)
        {
          if (Text.size() > Options.MaxStringBytes - StringBytes)
          {
            return Reject("IR string byte limit exceeded", IRSerializationErrorCode::ResourceLimit);
          }
          StringBytes += Text.size();
        }
        if (!isValidRecordKind(Entry.Kind))
        {
          return Reject("Unknown IR record kind", IRSerializationErrorCode::UnsupportedKind);
        }
        if (!validateRecordShape(Entry.Kind, N.size(), R.size(), S.size()))
        {
          return Reject("Malformed IR record attributes");
        }
        auto Flag = [&](std::size_t Position)
        {
          return N[Position] <= 1;
        };
        bool Valid = true;
        switch (Entry.Kind)
        {
        case RecordKind::Module:
          Valid = Index == 0;
          break;
        case RecordKind::BoolType:
        case RecordKind::VoidType:
        case RecordKind::NullPointerType:
        case RecordKind::PointerType:
        case RecordKind::ReferenceType:
        case RecordKind::QualifiedType:
        case RecordKind::ArrayType:
          Valid = Flag(schema::Type::Mutable);
          break;
        case RecordKind::IntegerType:
          Valid = Flag(schema::IntegerType::Mutable) && Flag(schema::IntegerType::Signed) && N[schema::IntegerType::Width] <= core::IntegerBits::MaxBitWidth && N[schema::IntegerType::Width] > 0;
          break;
        case RecordKind::FloatType:
          Valid = Flag(schema::FloatType::Mutable) && (N[schema::FloatType::Width] == 32 || N[schema::FloatType::Width] == 64);
          break;
        case RecordKind::FunctionType:
          Valid = std::all_of(N.begin(), N.end(), [](std::uint64_t Value)
          {
            return Value <= 1;
          });
          break;
        case RecordKind::ClassType:
        case RecordKind::InterfaceType:
        case RecordKind::StructType:
          Valid = Flag(schema::NominalType::Frozen) && N[schema::NominalType::Alignment] != 0 && (N[schema::NominalType::Alignment] & (N[schema::NominalType::Alignment] - 1)) == 0;
          break;
        case RecordKind::Function:
          Valid = decodeEnum<SymbolLinkage>(N[schema::Function::Linkage]).has_value() && decodeEnum<LanguageLinkage>(N[schema::Function::Language]).has_value() && decodeEnum<FunctionVisibility>(N[schema::Function::Visibility]).has_value() && decodeEnum<MethodDispatch>(N[schema::Function::Dispatch]).has_value() && Flag(schema::Function::Override) && Flag(schema::Function::Final) && (N[schema::Function::Slot] <= std::numeric_limits<std::uint32_t>::max() || N[schema::Function::Slot] == NoVirtualSlot);
          break;
        case RecordKind::Parameter:
          Valid = N[schema::Parameter::Index] <= std::numeric_limits<std::size_t>::max();
          break;
        case RecordKind::Global:
          Valid = Flag(schema::Global::StringStorage);
          break;
        case RecordKind::BoolConstant:
          Valid = Flag(schema::BoolConstant::Value);
          break;
        case RecordKind::Alloca:
          Valid = Flag(schema::Alloca::Initialized);
          break;
        case RecordKind::ICmp:
          Valid = decodeEnum<ICmpPredicate>(N[schema::ICmp::Predicate]).has_value();
          break;
        case RecordKind::GetElementPtr:
          Valid = Flag(schema::GetElementPtr::InBounds);
          break;
        default:
          break;
        }
        if (!Valid)
        {
          return Reject("Malformed IR record attributes");
        }
        for (std::size_t Operand = 0; Operand < R.size(); ++Operand)
        {
          const bool Nullable = isNullableReference(Entry.Kind, Operand);
          if ((R[Operand] == NullId && !Nullable) || (R[Operand] != NullId && R[Operand] >= Data.Records.size()))
          {
            return Reject("Invalid IR record reference", IRSerializationErrorCode::InvalidReference);
          }
        }
        if (Entry.Kind == RecordKind::Module || Entry.Kind == RecordKind::Import || Entry.Kind == RecordKind::Export)
        {
          for (const auto &Name : S)
          {
            if (Name.empty())
            {
              return Reject("Module and symbol identifier components must not be empty");
            }
          }
        }
      }
      return {};
    }

    class Collector
    {
      public:
        explicit Collector(const Module &Owner)
            : Owner(Owner)
        {
          Data.PointerBits = static_cast<std::uint32_t>(Owner.targetContext().pointerWidth());
          Data.ByteOrder = static_cast<std::uint32_t>(encodeEnum(Owner.targetContext().byteOrder()));
          Data.NativeAbi = Owner.targetContext().isNativeAbiCompatible();
          Data.Alignments = targetAlignments(Owner.targetContext());
        }

        IRSerializationResult<Model> run()
        {
          addOwned(&Owner);
          for (const auto &Value : Owner.classes())
          {
            addNominal(*Value);
          }
          for (const auto &Value : Owner.interfaces())
          {
            addNominal(*Value);
          }
          for (const auto &Value : Owner.structs())
          {
            addNominal(*Value);
          }
          for (const auto &Value : Owner.globalVariables())
          {
            addOwned(Value.get());
          }
          for (const auto &Value : Owner.globalFunctions())
          {
            addFunction(*Value, nullptr, nullptr);
          }
          for (std::size_t Index = 0; Index < Work.size() && !Error; ++Index)
          {
            if (Work[Index])
            {
              Record Result;
              collect(*Work[Index], Result);
              Data.Records[Index] = std::move(Result);
            }
          }
          if (Error)
          {
            return std::unexpected(std::move(*Error));
          }
          return std::move(Data);
        }

      private:
        void fail(std::string Message, IRSerializationErrorCode Code = IRSerializationErrorCode::InvalidIR)
        {
          if (!Error)
          {
            Error = IRSerializationError{Code, 0, std::move(Message)};
          }
        }

        std::uint32_t registerValue(const Value *Value)
        {
          if (!Value)
          {
            return NullId;
          }
          if (auto It = IDs.find(Value); It != IDs.end())
          {
            return It->second;
          }
          if (Data.Records.size() >= NullId)
          {
            fail("IR record ID capacity exceeded", IRSerializationErrorCode::ResourceLimit);
            return NullId;
          }
          const auto ID = static_cast<std::uint32_t>(Data.Records.size());
          IDs.emplace(Value, ID);
          Data.Records.emplace_back();
          Work.push_back(Value);
          return ID;
        }

        void addOwned(const Value *Value)
        {
          Owned.insert(Value);
          registerValue(Value);
        }

        void addFunction(const Function &Value, const NominalType *Parent, const Method *MethodValue)
        {
          addOwned(&Value);
          FunctionOwners.emplace(&Value, Parent);
          if (MethodValue)
          {
            Methods.emplace(&Value, MethodValue);
          }
          for (const auto &ParameterValue : Value.parameters())
          {
            addOwned(ParameterValue.get());
          }
          for (const auto &Block : Value.blocks())
          {
            addOwned(Block.get());
            for (const auto &Instruction : Block->instructions())
            {
              addOwned(Instruction.get());
            }
          }
        }

        void addNominal(const NominalType &Value)
        {
          addOwned(&Value);
          for (const auto &Entry : Value.methods())
          {
            if (Entry.function())
            {
              addFunction(*Entry.function(), &Value, &Entry);
            }
            else
            {
              fail("A nominal type contains an empty method");
            }
          }
        }

        std::uint32_t reference(const Value *Value)
        {
          if (!Value)
          {
            return NullId;
          }
          if (!Owned.contains(Value) && !(Type::classof(Value) && Owner.ownsType(static_cast<const Type *>(Value))) && !(Constant::classof(Value) && Owner.constantPool().owns(static_cast<const Constant *>(Value))))
          {
            fail("IR references a value outside this module and its context");
            return NullId;
          }
          return registerValue(Value);
        }

        void appendRecord(Record Entry)
        {
          Data.Records.push_back(std::move(Entry));
          Work.push_back(nullptr);
        }

        void collect(const Value &Value, Record &Result)
        {
          if (Type::classof(&Value))
          {
            collectType(static_cast<const Type &>(Value), Result);
            return;
          }
          if (Constant::classof(&Value))
          {
            collectConstant(static_cast<const Constant &>(Value), Result);
            return;
          }
          if (Instruction::classof(&Value))
          {
            collectInstruction(static_cast<const Instruction &>(Value), Result);
            return;
          }
          switch (Value.kind())
          {
          case ValueKind::Module:
            Result = makeRecord(RecordKind::Module);
            Result.References[schema::Module::Init] = reference(Owner.initializationFunction());
            Result.References[schema::Module::Deinit] = reference(Owner.deinitializationFunction());
            for (core::Name Component : Owner.moduleID().Components)
            {
              Result.Strings.push_back(nameText(Component));
            }
            for (const auto &Import : Owner.imports())
            {
              auto Entry = makeRecord(RecordKind::Import);
              Entry.References[schema::Import::Type] = reference(Import.SymbolType);
              Entry.Strings.clear();
              for (core::Name Component : Import.Target.Module.Components)
              {
                Entry.Strings.push_back(nameText(Component));
              }
              Entry.Strings.push_back(nameText(Import.Target.Symbol.SymbolName));
              appendRecord(std::move(Entry));
            }
            for (const auto &Export : Owner.exports())
            {
              for (const auto &Entry : Export.Entries)
              {
                auto ExportRecord = makeRecord(RecordKind::Export);
                ExportRecord.References[schema::Export::Declaration] = reference(Entry.Declaration);
                ExportRecord.Strings[schema::Export::Name] = nameText(Export.Symbol.SymbolName);
                appendRecord(std::move(ExportRecord));
              }
            }
            break;
          case ValueKind::Function:
          {
            const auto &FunctionValue = static_cast<const Function &>(Value);
            Result = makeRecord(RecordKind::Function);
            Result.References[schema::Function::Signature] = reference(FunctionValue.functionType());
            Result.References[schema::Function::Owner] = reference(FunctionOwners[&FunctionValue]);
            Result.Numbers[schema::Function::Linkage] = encodeEnum(FunctionValue.symbolLinkage());
            Result.Numbers[schema::Function::Language] = encodeEnum(FunctionValue.languageLinkage());
            Result.Numbers[schema::Function::Visibility] = encodeEnum(FunctionValue.visibility());
            Result.Numbers[schema::Function::Dispatch] = encodeEnum(MethodDispatch::Direct);
            Result.Numbers[schema::Function::Slot] = NoVirtualSlot;
            Result.Strings[schema::Function::Name] = nameText(FunctionValue.name());
            if (auto It = Methods.find(&FunctionValue); It != Methods.end())
            {
              const auto &Info = It->second->virtualInfo();
              Result.Numbers[schema::Function::Dispatch] = encodeEnum(Info.Dispatch);
              Result.Numbers[schema::Function::Override] = Info.Override;
              Result.Numbers[schema::Function::Final] = Info.Final;
              Result.Numbers[schema::Function::Slot] = Info.Slot ? static_cast<std::uint64_t>(*Info.Slot) : NoVirtualSlot;
            }
            break;
          }
          case ValueKind::Parameter:
          {
            const auto &ParameterValue = static_cast<const Parameter &>(Value);
            Result = makeRecord(RecordKind::Parameter);
            Result.Numbers[schema::Parameter::Index] = ParameterValue.index();
            Result.References[schema::Parameter::Function] = reference(ParameterValue.function());
            Result.Strings[schema::Parameter::Name] = nameText(ParameterValue.name());
            break;
          }
          case ValueKind::BasicBlock:
          {
            const auto &Block = static_cast<const BasicBlock &>(Value);
            Result = makeRecord(RecordKind::Block);
            Result.References[schema::Block::Function] = reference(Block.function());
            Result.Strings[schema::Block::Name] = nameText(Block.name());
            break;
          }
          case ValueKind::GlobalVariable:
          {
            const auto &Global = static_cast<const GlobalVariable &>(Value);
            Result = makeRecord(RecordKind::Global);
            Result.Numbers[schema::Global::StringStorage] = IRModelAccess::isStringStorage(Owner, Global);
            Result.References[schema::Global::Type] = reference(Global.type());
            Result.References[schema::Global::Initializer] = reference(Global.initializer());
            Result.Strings[schema::Global::Name] = nameText(Global.name());
            break;
          }
          default:
            fail("Unsupported IR value kind", IRSerializationErrorCode::UnsupportedKind);
            break;
          }
        }

        void collectType(const Type &Value, Record &Result)
        {
          if (NominalType::classof(&Value))
          {
            const auto &Nominal = static_cast<const NominalType &>(Value);
            if (Nominal.declarationType() != &Nominal)
            {
              Result = makeRecord(RecordKind::QualifiedType);
              Result.Numbers[schema::QualifiedType::Mutable] = Value.isMutable();
              Result.References[schema::QualifiedType::Declaration] = reference(Nominal.declarationType());
              return;
            }
            Result = makeRecord(Value.kind() == ValueKind::Class ? RecordKind::ClassType : Value.kind() == ValueKind::Interface ? RecordKind::InterfaceType : RecordKind::StructType);
            Result.Numbers[schema::NominalType::Size] = Nominal.size();
            Result.Numbers[schema::NominalType::Alignment] = Nominal.alignment();
            Result.Numbers[schema::NominalType::Frozen] = Nominal.isFrozen();
            Result.Strings[schema::NominalType::Name] = nameText(Nominal.name());
            for (const auto &Field : Nominal.fields())
            {
              auto Entry = makeRecord(RecordKind::Field);
              Entry.Numbers[schema::Field::Offset] = Field.offset();
              Entry.References[schema::Field::Owner] = reference(&Nominal);
              Entry.References[schema::Field::Type] = reference(Field.type());
              Entry.References[schema::Field::Initializer] = reference(Field.initializer());
              Entry.Strings[schema::Field::Name] = nameText(Field.name());
              appendRecord(std::move(Entry));
            }
            return;
          }
          switch (Value.kind())
          {
          case ValueKind::Bool:
            Result = makeRecord(RecordKind::BoolType);
            break;
          case ValueKind::Void:
            Result = makeRecord(RecordKind::VoidType);
            break;
          case ValueKind::NullPointer:
            Result = makeRecord(RecordKind::NullPointerType);
            break;
          case ValueKind::Integer:
          {
            const auto &Integer = static_cast<const IntegerType &>(Value);
            Result = makeRecord(RecordKind::IntegerType);
            Result.Numbers[schema::IntegerType::Width] = Integer.width();
            Result.Numbers[schema::IntegerType::Signed] = Integer.isSigned();
            break;
          }
          case ValueKind::Float:
            Result = makeRecord(RecordKind::FloatType);
            Result.Numbers[schema::FloatType::Width] = static_cast<const FloatType &>(Value).width();
            break;
          case ValueKind::MetaType:
            Result = makeRecord(RecordKind::MetaType);
            return;
          case ValueKind::Variadic:
            Result = makeRecord(RecordKind::VariadicType);
            return;
          case ValueKind::Pointer:
            Result = makeRecord(RecordKind::PointerType);
            Result.References[schema::PointerType::Element] = reference(static_cast<const PointerType &>(Value).pointeeType());
            break;
          case ValueKind::Reference:
            Result = makeRecord(RecordKind::ReferenceType);
            Result.References[schema::ReferenceType::Element] = reference(static_cast<const ReferenceType &>(Value).referencedType());
            break;
          case ValueKind::Array:
          {
            const auto &Array = static_cast<const ArrayType &>(Value);
            Result = makeRecord(RecordKind::ArrayType);
            Result.Numbers[schema::ArrayType::Count] = Array.elementCount();
            Result.References[schema::ArrayType::Element] = reference(Array.elementType());
            break;
          }
          case ValueKind::FunctionType:
          {
            const auto &Signature = static_cast<const FunctionType &>(Value);
            Result = makeRecord(RecordKind::FunctionType);
            Result.References[schema::FunctionType::Return] = reference(Signature.returnType());
            for (const auto &Parameter : Signature.parameters())
            {
              Result.Numbers.push_back(Parameter.Variadic);
              Result.References.push_back(reference(Parameter.ParameterType));
            }
            break;
          }
          default:
            fail("Unsupported IR type kind", IRSerializationErrorCode::UnsupportedKind);
            return;
          }
          Result.Numbers[schema::Type::Mutable] = Value.isMutable();
        }

        void collectConstant(const Constant &Value, Record &Result)
        {
          const auto TypeId = reference(Value.type());
          switch (Value.kind())
          {
          case ValueKind::BoolConstant:
            Result = makeRecord(RecordKind::BoolConstant);
            Result.Numbers[schema::BoolConstant::Value] = static_cast<const BoolConstant &>(Value).value();
            break;
          case ValueKind::IntegerConstant:
          {
            Result = makeRecord(RecordKind::IntegerConstant);
            const auto Words = static_cast<const IntegerConstant &>(Value).bits().words();
            Result.Numbers.assign(Words.begin(), Words.end());
            break;
          }
          case ValueKind::FloatConstant:
            Result = makeRecord(RecordKind::FloatConstant);
            Result.Numbers[schema::FloatConstant::Bits] = static_cast<const FloatConstant &>(Value).bits();
            break;
          case ValueKind::NullPointerConstant:
            Result = makeRecord(RecordKind::NullPointerConstant);
            break;
          case ValueKind::StringConstant:
            Result = makeRecord(RecordKind::StringConstant);
            Result.Strings[schema::StringConstant::Bytes] = std::string(static_cast<const StringConstant &>(Value).value());
            break;
          case ValueKind::ArrayConstant:
            Result = makeRecord(RecordKind::ArrayConstant);
            for (const auto *Element : static_cast<const ArrayConstant &>(Value).elements())
            {
              Result.References.push_back(reference(Element));
            }
            break;
          default:
            fail("Unsupported IR constant kind", IRSerializationErrorCode::UnsupportedKind);
            return;
          }
          Result.References[schema::Typed::Type] = TypeId;
        }

        void collectInstruction(const Instruction &Value, Record &Result)
        {
          const auto TypeId = reference(Value.type());
          const auto BlockId = reference(Value.parentBlock());
          switch (Value.kind())
          {
          case ValueKind::AllocaInstruction:
          {
            const auto &Instruction = static_cast<const AllocaInstruction &>(Value);
            Result = makeRecord(RecordKind::Alloca);
            Result.Numbers[schema::Alloca::Initialized] = Instruction.isInitialized();
            Result.Strings[schema::Alloca::Name] = nameText(Instruction.name());
            break;
          }
          case ValueKind::LoadInstruction:
            Result = makeRecord(RecordKind::Load);
            Result.References[schema::Load::Pointer] = reference(static_cast<const LoadInstruction &>(Value).pointer());
            break;
          case ValueKind::StoreInstruction:
          {
            const auto &Instruction = static_cast<const StoreInstruction &>(Value);
            Result = makeRecord(RecordKind::Store);
            Result.References[schema::Store::Value] = reference(Instruction.storedValue());
            Result.References[schema::Store::Pointer] = reference(Instruction.pointer());
            break;
          }
          case ValueKind::NegInstruction:
            Result = makeRecord(RecordKind::Neg);
            Result.References[schema::Neg::Operand] = reference(static_cast<const NegInstruction &>(Value).operand());
            break;
          case ValueKind::ICmpInstruction:
          {
            const auto &Instruction = static_cast<const ICmpInstruction &>(Value);
            Result = makeRecord(RecordKind::ICmp);
            Result.Numbers[schema::ICmp::Predicate] = encodeEnum(Instruction.predicate());
            Result.References[schema::ICmp::Left] = reference(Instruction.left());
            Result.References[schema::ICmp::Right] = reference(Instruction.right());
            break;
          }
          case ValueKind::BranchInstruction:
            Result = makeRecord(RecordKind::Branch);
            Result.References[schema::Branch::Target] = reference(static_cast<const BranchInstruction &>(Value).target());
            break;
          case ValueKind::ConditionalBranchInstruction:
          {
            const auto &Instruction = static_cast<const ConditionalBranchInstruction &>(Value);
            Result = makeRecord(RecordKind::CondBranch);
            Result.References[schema::CondBranch::Condition] = reference(Instruction.condition());
            Result.References[schema::CondBranch::TrueTarget] = reference(Instruction.trueTarget());
            Result.References[schema::CondBranch::FalseTarget] = reference(Instruction.falseTarget());
            break;
          }
          case ValueKind::ReturnInstruction:
            Result = makeRecord(RecordKind::Return);
            Result.References[schema::Return::Value] = reference(static_cast<const ReturnInstruction &>(Value).returnedValue());
            break;
          case ValueKind::CallInstruction:
          {
            const auto &Instruction = static_cast<const CallInstruction &>(Value);
            Result = makeRecord(RecordKind::Call);
            Result.References[schema::Call::Signature] = reference(Instruction.functionType());
            Result.References[schema::Call::Callee] = reference(Instruction.callee());
            for (const auto &Argument : Instruction.arguments())
            {
              Result.References.push_back(reference(Argument.value()));
              Result.Strings.push_back(nameText(Argument.name()));
            }
            break;
          }
          case ValueKind::GetElementPtrInstruction:
          {
            const auto &Instruction = static_cast<const GetElementPtrInstruction &>(Value);
            Result = makeRecord(RecordKind::GetElementPtr);
            Result.Numbers[schema::GetElementPtr::InBounds] = Instruction.isInBounds();
            Result.References[schema::GetElementPtr::SourceType] = reference(Instruction.sourceElementType());
            Result.References[schema::GetElementPtr::Pointer] = reference(Instruction.pointer());
            for (const auto *Index : Instruction.indices())
            {
              Result.References.push_back(reference(Index));
            }
            break;
          }
          default:
            fail("Unsupported IR instruction kind", IRSerializationErrorCode::UnsupportedKind);
            return;
          }
          Result.References[schema::Instruction::Type] = TypeId;
          Result.References[schema::Instruction::Block] = BlockId;
        }

        const Module &Owner;
        Model Data;
        std::vector<const Value *> Work;
        std::unordered_map<const Value *, std::uint32_t> IDs;
        std::unordered_set<const Value *> Owned;
        std::unordered_map<const Function *, const NominalType *> FunctionOwners;
        std::unordered_map<const Function *, const Method *> Methods;
        std::optional<IRSerializationError> Error;
    };

    class Decoder
    {
      public:
        Decoder(IRContext &Context, Module &Result, const Model &Data, const IRReadOptions &Options)
            : Context(Context),
              Result(Result),
              Data(Data),
              Options(Options),
              Values(Data.Records.size()),
              States(Data.Records.size()),
              Depths(Data.Records.size()),
              Functions(Data.Records.size()),
              Instructions(Data.Records.size())
        {
          Values[0] = &Result;
          States[0] = 2;
        }

        IRSerializationResult<void> run()
        {
          for (std::size_t Index = 1; Index < Data.Records.size() && !Error; ++Index)
          {
            const auto &Entry = Data.Records[Index];
            if (isNominal(Entry.Kind))
            {
              auto Name = intern(Entry.Strings[schema::Named::Name]);
              NominalType *Value = Entry.Kind == RecordKind::ClassType ? static_cast<NominalType *>(Result.createClassType(Entry.Numbers[schema::NominalType::Size], Entry.Numbers[schema::NominalType::Alignment], Name)) : Entry.Kind == RecordKind::InterfaceType ? static_cast<NominalType *>(Result.createInterfaceType(Entry.Numbers[schema::NominalType::Size], Entry.Numbers[schema::NominalType::Alignment], Name)) : static_cast<NominalType *>(Result.createStructType(Entry.Numbers[schema::NominalType::Size], Entry.Numbers[schema::NominalType::Alignment], Name));
              set(Index, Value);
            }
          }
          for (std::size_t Index = 1; Index < Data.Records.size() && !Error; ++Index)
          {
            const auto &Entry = Data.Records[Index];
            if (Entry.Kind != RecordKind::Function)
            {
              continue;
            }
            resolve(Entry.References[schema::Function::Signature]);
            const auto *Signature = as<FunctionType>(Entry.References[schema::Function::Signature]);
            if (!Signature)
            {
              break;
            }
            auto Callable = std::make_unique<Function>(*Signature, intern(Entry.Strings[schema::Named::Name]), *decodeEnum<SymbolLinkage>(Entry.Numbers[schema::Function::Linkage]), *decodeEnum<LanguageLinkage>(Entry.Numbers[schema::Function::Language]), *decodeEnum<FunctionVisibility>(Entry.Numbers[schema::Function::Visibility]));
            set(Index, Callable.get());
            if (Entry.References[schema::Function::Owner] == NullId)
            {
              if (Entry.Numbers[schema::Function::Dispatch] != encodeEnum(MethodDispatch::Direct) || Entry.Numbers[schema::Function::Override] || Entry.Numbers[schema::Function::Final] || Entry.Numbers[schema::Function::Slot] != NoVirtualSlot)
              {
                fail("A global function has method-only attributes");
              }
              if (!Result.appendGlobalFunction(std::move(Callable)))
              {
                fail("Invalid global function signature");
              }
            }
            else
            {
              as<NominalType>(Entry.References[schema::Function::Owner]);
              Functions[Index] = std::move(Callable);
            }
          }
          std::unordered_set<const Parameter *> SeenParameters;
          for (std::size_t Index = 1; Index < Data.Records.size() && !Error; ++Index)
          {
            const auto &Entry = Data.Records[Index];
            if (Entry.Kind == RecordKind::Parameter)
            {
              auto *Parent = mutableAs<Function>(Entry.References[schema::Parameter::Function]);
              if (!Parent)
              {
                break;
              }
              auto *Value = Parent->parameter(static_cast<std::size_t>(Entry.Numbers[schema::Parameter::Index]));
              if (!Value || !SeenParameters.insert(Value).second)
              {
                fail("Invalid or duplicate formal parameter index");
                break;
              }
              Value->setName(intern(Entry.Strings[schema::Named::Name]));
              set(Index, Value);
            }
            else if (Entry.Kind == RecordKind::Block)
            {
              auto *Parent = mutableAs<Function>(Entry.References[schema::Block::Function]);
              if (Parent)
              {
                set(Index, Parent->createBlock(intern(Entry.Strings[schema::Named::Name])));
              }
            }
          }
          for (std::size_t Index = 1; Index < Data.Records.size() && !Error; ++Index)
          {
            const auto &Entry = Data.Records[Index];
            if (Entry.Kind == RecordKind::Function)
            {
              for (const auto &Parameter : as<Function>(static_cast<std::uint32_t>(Index))->parameters())
              {
                if (!SeenParameters.contains(Parameter.get()))
                {
                  fail("Missing formal parameter record");
                }
              }
            }
            else if (Entry.Kind == RecordKind::Global)
            {
              resolve(Entry.References[schema::Global::Type]);
              resolve(Entry.References[schema::Global::Initializer]);
              const auto *Address = as<PointerType>(Entry.References[schema::Global::Type]);
              const auto *Initializer = as<Constant>(Entry.References[schema::Global::Initializer], true);
              if (Error || !Address)
              {
                break;
              }
              auto Variable = std::make_unique<GlobalVariable>(*Address, intern(Entry.Strings[schema::Named::Name]));
              if (!Variable->setInitializer(Initializer))
              {
                fail("Incompatible global initializer");
                break;
              }
              auto *Value = Result.appendGlobalVariable(std::move(Variable));
              set(Index, Value);
              if (Value && Entry.Numbers[schema::Global::StringStorage] && !IRModelAccess::registerStringStorage(Result, *Value))
              {
                fail("Invalid or duplicate string storage");
              }
            }
          }
          for (std::size_t Index = 1; Index < Data.Records.size() && !Error; ++Index)
          {
            const auto Kind = Data.Records[Index].Kind;
            if (Kind != RecordKind::Field && Kind != RecordKind::Import && Kind != RecordKind::Export)
            {
              resolve(static_cast<std::uint32_t>(Index));
            }
          }
          for (std::size_t Index = 1; Index < Data.Records.size() && !Error; ++Index)
          {
            if (Instructions[Index])
            {
              auto *Parent = mutableAs<BasicBlock>(Data.Records[Index].References[schema::Instruction::Block]);
              if (Parent)
              {
                Parent->appendInstruction(std::move(Instructions[Index]));
              }
            }
          }
          for (std::size_t Index = 1; Index < Data.Records.size() && !Error; ++Index)
          {
            const auto &Entry = Data.Records[Index];
            if (Entry.Kind == RecordKind::Field)
            {
              auto *Owner = mutableAs<NominalType>(Entry.References[schema::Field::Owner]);
              const auto *Type = as<ink::ir::Type>(Entry.References[schema::Field::Type]);
              const auto *Initializer = as<Constant>(Entry.References[schema::Field::Initializer], true);
              if (!Error && !Owner->addField(Field(Type, intern(Entry.Strings[schema::Named::Name]), Entry.Numbers[schema::Field::Offset], Initializer)))
              {
                fail("Invalid nominal field metadata");
              }
            }
            else if (Entry.Kind == RecordKind::Function && Functions[Index])
            {
              auto *Owner = mutableAs<NominalType>(Entry.References[schema::Function::Owner]);
              MethodVirtualInfo Info{*decodeEnum<MethodDispatch>(Entry.Numbers[schema::Function::Dispatch]), Entry.Numbers[schema::Function::Override] != 0, Entry.Numbers[schema::Function::Final] != 0, std::nullopt};
              if (Entry.Numbers[schema::Function::Slot] != NoVirtualSlot)
              {
                Info.Slot = static_cast<std::uint32_t>(Entry.Numbers[schema::Function::Slot]);
              }
              Method EntryValue(std::move(Functions[Index]), Info);
              if (!Error && !Owner->addMethod(std::move(EntryValue)))
              {
                fail("Invalid method metadata");
              }
            }
            else if (Entry.Kind == RecordKind::Import)
            {
              SymbolImport Import;
              Import.SymbolType = as<Type>(Entry.References[schema::Import::Type]);
              for (std::size_t Part = 0; Part + 1 < Entry.Strings.size(); ++Part)
              {
                Import.Target.Module.Components.push_back(intern(Entry.Strings[Part]));
              }
              Import.Target.Symbol.SymbolName = intern(Entry.Strings.back());
              Result.imports().push_back(std::move(Import));
            }
            else if (Entry.Kind == RecordKind::Export)
            {
              const auto *Declaration = value(Entry.References[schema::Export::Declaration]);
              if (!Error && !Result.addExport({intern(Entry.Strings[schema::Named::Name])}, *Declaration))
              {
                fail("Invalid exported declaration");
              }
            }
          }
          for (std::size_t Index = 1; Index < Data.Records.size() && !Error; ++Index)
          {
            const auto &Entry = Data.Records[Index];
            if (isNominal(Entry.Kind) && Entry.Numbers[schema::NominalType::Frozen])
            {
              mutableAs<NominalType>(static_cast<std::uint32_t>(Index))->freeze();
            }
          }
          if (Error)
          {
            return std::unexpected(std::move(*Error));
          }
          auto *Initialization = mutableAs<Function>(Data.Records[0].References[schema::Module::Init], true);
          auto *Deinitialization = mutableAs<Function>(Data.Records[0].References[schema::Module::Deinit], true);
          if (!Error && (!Result.setInitializationFunction(Initialization) || !Result.setDeinitializationFunction(Deinitialization)))
          {
            fail("Invalid module lifecycle function");
          }
          if (Error)
          {
            return std::unexpected(std::move(*Error));
          }
          return IRVerifier::verify(Result);
        }

      private:
        void fail(std::string Message, IRSerializationErrorCode Code = IRSerializationErrorCode::InvalidIR)
        {
          if (!Error)
          {
            Error = IRSerializationError{Code, 0, std::move(Message)};
          }
        }

        core::Name intern(std::string_view Text)
        {
          auto Name = core::NamePool::instance().intern(Text);
          if (!Text.empty() && !Name.valid())
          {
            fail("Name pool capacity exceeded", IRSerializationErrorCode::ResourceLimit);
          }
          return Name;
        }

        const Value *value(std::uint32_t ID, bool Optional = false)
        {
          if (Optional && ID == NullId)
          {
            return nullptr;
          }
          if (ID >= Values.size() || !Values[ID])
          {
            fail("Reference does not identify a constructed IR value", IRSerializationErrorCode::InvalidReference);
            return nullptr;
          }
          return Values[ID];
        }

        template <typename T>
        const T *as(std::uint32_t ID, bool Optional = false)
        {
          const auto *Value = value(ID, Optional);
          if (!Value)
          {
            return nullptr;
          }
          if (!T::classof(Value))
          {
            fail("Reference has an incompatible IR kind", IRSerializationErrorCode::InvalidReference);
            return nullptr;
          }
          return static_cast<const T *>(Value);
        }

        template <typename T>
        T *mutableAs(std::uint32_t ID, bool Optional = false)
        {
          return const_cast<T *>(as<T>(ID, Optional));
        }

        void set(std::size_t ID, const Value *Value)
        {
          if (!Value)
          {
            fail("IR construction rejected invalid record data");
            return;
          }
          if (Type::classof(Value) || Constant::classof(Value))
          {
            std::size_t Depth = 1;
            for (const auto Reference : Data.Records[ID].References)
            {
              if (Reference != NullId)
              {
                Depth = std::max(Depth, Depths[Reference] + 1);
              }
            }
            if (Depth > Options.MaxDepth)
            {
              fail("IR structural type or constant nesting limit exceeded", IRSerializationErrorCode::ResourceLimit);
              return;
            }
            Depths[ID] = Depth;
          }
          Values[ID] = Value;
          States[ID] = 2;
        }

        // Explicit DFS permits long linear SSA chains without consuming the C++ stack.
        // MaxDepth counts structural type/constant nesting, not instruction dependencies.
        void resolve(std::uint32_t ID)
        {
          if (ID == NullId || Error || (ID < States.size() && States[ID] == 2))
          {
            return;
          }
          struct Frame
          {
              std::uint32_t ID;
              std::size_t Next;
              std::size_t Depth;
          };
          std::vector<Frame> Stack{{ID, 0, 0}};
          while (!Stack.empty() && !Error)
          {
            auto &Frame = Stack.back();
            if (Frame.ID >= Data.Records.size())
            {
              fail("Reference ID is out of range", IRSerializationErrorCode::InvalidReference);
              break;
            }
            const auto &Entry = Data.Records[Frame.ID];
            if (States[Frame.ID] == 2)
            {
              Stack.pop_back();
              continue;
            }
            if (Frame.Next == 0)
            {
              States[Frame.ID] = 1;
              if (!isInstruction(Entry.Kind) && ++Frame.Depth > Options.MaxDepth)
              {
                fail("IR dependency nesting limit exceeded", IRSerializationErrorCode::ResourceLimit);
                break;
              }
            }
            if (Frame.Next < Entry.References.size())
            {
              const auto Child = Entry.References[Frame.Next++];
              if (Child == NullId || States[Child] == 2)
              {
                continue;
              }
              if (States[Child] == 1)
              {
                fail("Cyclic structural type, constant or SSA value dependency", IRSerializationErrorCode::InvalidReference);
                break;
              }
              Stack.push_back({Child, 0, Frame.Depth});
              continue;
            }
            build(Frame.ID);
            Stack.pop_back();
          }
        }

        void build(std::uint32_t ID)
        {
          const auto &Entry = Data.Records[ID];
          const auto &N = Entry.Numbers;
          const auto &R = Entry.References;
          auto &Pool = Context.typePool();
          auto &Constants = Context.constantPool();
          const Value *Constructed = nullptr;
          switch (Entry.Kind)
          {
          case RecordKind::BoolType:
            Constructed = Pool.get<BoolType>(N[schema::Type::Mutable] != 0);
            break;
          case RecordKind::IntegerType:
            Constructed = Pool.get<IntegerType>(static_cast<std::uint32_t>(N[schema::IntegerType::Width]), N[schema::IntegerType::Signed] != 0, N[schema::IntegerType::Mutable] != 0);
            break;
          case RecordKind::FloatType:
            Constructed = Pool.get<FloatType>(static_cast<std::uint32_t>(N[schema::FloatType::Width]), N[schema::FloatType::Mutable] != 0);
            break;
          case RecordKind::VoidType:
            Constructed = Pool.get<VoidType>(N[schema::Type::Mutable] != 0);
            break;
          case RecordKind::NullPointerType:
            Constructed = Pool.get<NullPointerType>(N[schema::Type::Mutable] != 0);
            break;
          case RecordKind::MetaType:
            Constructed = Pool.get<MetaType>();
            break;
          case RecordKind::VariadicType:
            Constructed = Pool.get<VariadicType>();
            break;
          case RecordKind::QualifiedType:
            Constructed = Pool.get(as<NominalType>(R[schema::QualifiedType::Declaration]), N[schema::QualifiedType::Mutable] != 0);
            break;
          case RecordKind::PointerType:
            Constructed = Pool.get<PointerType>(as<Type>(R[schema::PointerType::Element]), N[schema::PointerType::Mutable] != 0);
            break;
          case RecordKind::ReferenceType:
            Constructed = Pool.get<ReferenceType>(as<Type>(R[schema::ReferenceType::Element]), N[schema::ReferenceType::Mutable] != 0);
            break;
          case RecordKind::ArrayType:
            Constructed = Pool.get<ArrayType>(as<Type>(R[schema::ArrayType::Element]), N[schema::ArrayType::Count], N[schema::ArrayType::Mutable] != 0);
            break;
          case RecordKind::FunctionType:
          {
            std::vector<FunctionTypeParameter> Parameters;
            Parameters.reserve(R.size() - schema::FunctionType::ParametersBegin);
            for (std::size_t Index = schema::FunctionType::ParametersBegin; Index < R.size(); ++Index)
            {
              Parameters.emplace_back(as<Type>(R[Index]), N[Index - schema::FunctionType::ParametersBegin + schema::FunctionType::VariadicsBegin] != 0);
            }
            Constructed = Pool.get<FunctionType>(as<Type>(R[schema::FunctionType::Return]), std::span<const FunctionTypeParameter>(Parameters), N[schema::FunctionType::Mutable] != 0);
            break;
          }
          case RecordKind::BoolConstant:
            Constructed = Constants.get<BoolConstant>(N[schema::BoolConstant::Value] != 0);
            break;
          case RecordKind::IntegerConstant:
          {
            const auto *Type = as<IntegerType>(R[schema::IntegerConstant::Type]);
            if (Type && N.size() == (static_cast<std::size_t>(Type->width()) + 63) / 64)
            {
              const auto Bits = core::IntegerBits::fromWords(Type->width(), N);
              if (Bits)
              {
                Constructed = Constants.get<IntegerConstant>(Type, *Bits);
              }
            }
            break;
          }
          case RecordKind::FloatConstant:
          {
            const auto *Type = as<FloatType>(R[schema::FloatConstant::Type]);
            if (Type)
            {
              const auto Bits = core::IntegerBits::fromUnsigned(Type->width(), N[schema::FloatConstant::Bits]);
              if (Bits)
              {
                Constructed = Constants.get<FloatConstant>(Type, *Bits);
              }
            }
            break;
          }
          case RecordKind::NullPointerConstant:
            Constructed = Constants.get<NullPointerConstant>();
            break;
          case RecordKind::StringConstant:
            Constructed = Constants.get<StringConstant>(Entry.Strings[schema::StringConstant::Bytes]);
            break;
          case RecordKind::ArrayConstant:
          {
            std::vector<const Constant *> Elements;
            Elements.reserve(R.size() - schema::ArrayConstant::ElementsBegin);
            for (std::size_t Index = schema::ArrayConstant::ElementsBegin; Index < R.size(); ++Index)
            {
              Elements.push_back(as<Constant>(R[Index]));
            }
            Constructed = Constants.get<ArrayConstant>(as<ArrayType>(R[schema::ArrayConstant::Type]), std::move(Elements));
            break;
          }
          default:
            if (isInstruction(Entry.Kind))
            {
              buildInstruction(ID);
              return;
            }
            fail("Unexpected reference to a declaration or metadata record", IRSerializationErrorCode::InvalidReference);
            return;
          }
          if (Constructed && Constant::classof(Constructed) && Constructed->type() != as<Type>(R[schema::Typed::Type]))
          {
            fail("Constant type does not match its encoded type");
          }
          set(ID, Constructed);
        }

        void buildInstruction(std::uint32_t ID)
        {
          const auto &Entry = Data.Records[ID];
          const auto &N = Entry.Numbers;
          const auto &R = Entry.References;
          const auto *Type = as<ink::ir::Type>(R[schema::Instruction::Type]);
          as<BasicBlock>(R[schema::Instruction::Block]);
          if (Error || !Type)
          {
            return;
          }
          std::unique_ptr<Instruction> InstructionValue;
          switch (Entry.Kind)
          {
          case RecordKind::Alloca:
          {
            const auto *Pointer = as<PointerType>(R[schema::Alloca::Type]);
            if (Pointer)
            {
              auto Value = std::make_unique<AllocaInstruction>(*Pointer);
              IRModelAccess::setInitialized(*Value, N[schema::Alloca::Initialized] != 0);
              Value->setName(intern(Entry.Strings[schema::Alloca::Name]));
              InstructionValue = std::move(Value);
            }
            break;
          }
          case RecordKind::Load:
            InstructionValue = std::make_unique<LoadInstruction>(*Type, *value(R[schema::Load::Pointer]));
            break;
          case RecordKind::Store:
          {
            const auto *Void = as<VoidType>(R[schema::Store::Type]);
            if (Void)
            {
              InstructionValue = std::make_unique<StoreInstruction>(*Void, *value(R[schema::Store::Value]), *value(R[schema::Store::Pointer]));
            }
            break;
          }
          case RecordKind::Neg:
            InstructionValue = std::make_unique<NegInstruction>(*value(R[schema::Neg::Operand]));
            break;
          case RecordKind::ICmp:
          {
            const auto *Bool = as<BoolType>(R[schema::ICmp::Type]);
            if (Bool)
            {
              InstructionValue = std::make_unique<ICmpInstruction>(*Bool, *decodeEnum<ICmpPredicate>(N[schema::ICmp::Predicate]), *value(R[schema::ICmp::Left]), *value(R[schema::ICmp::Right]));
            }
            break;
          }
          case RecordKind::Branch:
          {
            const auto *Void = as<VoidType>(R[schema::Branch::Type]);
            auto *Target = mutableAs<BasicBlock>(R[schema::Branch::Target]);
            if (!Error)
            {
              InstructionValue = std::make_unique<BranchInstruction>(*Void, *Target);
            }
            break;
          }
          case RecordKind::CondBranch:
          {
            const auto *Void = as<VoidType>(R[schema::CondBranch::Type]);
            auto *True = mutableAs<BasicBlock>(R[schema::CondBranch::TrueTarget]);
            auto *False = mutableAs<BasicBlock>(R[schema::CondBranch::FalseTarget]);
            if (!Error)
            {
              InstructionValue = std::make_unique<ConditionalBranchInstruction>(*Void, *value(R[schema::CondBranch::Condition]), *True, *False);
            }
            break;
          }
          case RecordKind::Return:
          {
            const auto *Void = as<VoidType>(R[schema::Return::Type]);
            if (Void)
            {
              InstructionValue = std::make_unique<ReturnInstruction>(*Void, value(R[schema::Return::Value], true));
            }
            break;
          }
          case RecordKind::Call:
          {
            const auto *Signature = as<FunctionType>(R[schema::Call::Signature]);
            std::vector<Argument> Arguments;
            Arguments.reserve(R.size() - schema::Call::ArgumentsBegin);
            for (std::size_t Index = schema::Call::ArgumentsBegin; Index < R.size(); ++Index)
            {
              Arguments.emplace_back(*value(R[Index]), intern(Entry.Strings[Index - schema::Call::ArgumentsBegin + schema::Call::ArgumentNamesBegin]));
            }
            if (!Error)
            {
              InstructionValue = std::make_unique<CallInstruction>(*Signature, *value(R[schema::Call::Callee]), std::move(Arguments));
            }
            break;
          }
          case RecordKind::GetElementPtr:
          {
            const auto *Pointer = as<PointerType>(R[schema::GetElementPtr::Type]);
            const auto *Source = as<ink::ir::Type>(R[schema::GetElementPtr::SourceType]);
            std::vector<const Value *> Indices;
            Indices.reserve(R.size() - schema::GetElementPtr::IndicesBegin);
            for (std::size_t Index = schema::GetElementPtr::IndicesBegin; Index < R.size(); ++Index)
            {
              Indices.push_back(value(R[Index]));
            }
            if (!Error)
            {
              InstructionValue = std::make_unique<GetElementPtrInstruction>(*Pointer, *Source, *value(R[schema::GetElementPtr::Pointer]), std::move(Indices), N[schema::GetElementPtr::InBounds] != 0);
            }
            break;
          }
          default:
            fail("Unsupported instruction opcode", IRSerializationErrorCode::UnsupportedKind);
            break;
          }
          if (InstructionValue && InstructionValue->type() != Type)
          {
            fail("Instruction result type does not match its encoded type");
          }
          set(ID, InstructionValue.get());
          Instructions[ID] = std::move(InstructionValue);
        }

        IRContext &Context;
        Module &Result;
        const Model &Data;
        const IRReadOptions &Options;
        std::vector<const Value *> Values;
        std::vector<std::uint8_t> States;
        std::vector<std::size_t> Depths;
        std::vector<std::unique_ptr<Function>> Functions;
        std::vector<std::unique_ptr<Instruction>> Instructions;
        std::optional<IRSerializationError> Error;
    };
  } // namespace

  IRSerializationResult<Model> collectModel(const Module &ModuleValue)
  {
    return Collector(ModuleValue).run();
  }

  IRSerializationResult<std::unique_ptr<Module>> decodeModel(IRContext &Context, const Model &Data, const IRReadOptions &Options)
  {
    const auto &Target = Context.targetContext();
    if (Data.PointerBits != static_cast<std::uint32_t>(Target.pointerWidth()) || Data.ByteOrder != encodeEnum(Target.byteOrder()) || Data.NativeAbi != Target.isNativeAbiCompatible() || Data.Alignments != targetAlignments(Target))
    {
      return std::unexpected(IRSerializationError{IRSerializationErrorCode::TargetMismatch, 0, "IR target layout does not match the destination context"});
    }
    auto Valid = validateModel(Data, Options);
    if (!Valid)
    {
      return std::unexpected(std::move(Valid.error()));
    }
    ModuleID ID;
    for (const auto &Component : Data.Records[0].Strings)
    {
      const auto Name = core::NamePool::instance().intern(Component);
      if (!Name.valid())
      {
        return std::unexpected(IRSerializationError{IRSerializationErrorCode::ResourceLimit, 0, "Name pool capacity exceeded"});
      }
      ID.Components.push_back(Name);
    }
    auto Result = std::make_unique<Module>(Context, std::move(ID));
    IRLoadTransaction Transaction(Context);
    Decoder Loader(Context, *Result, Data, Options);
    auto Loaded = Loader.run();
    if (!Loaded)
    {
      return std::unexpected(std::move(Loaded.error()));
    }
    Transaction.commit();
    return Result;
  }
} // namespace ink::ir::detail
