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
    struct KindName
    {
        RecordKind Kind;
        std::string_view Name;
    };

    constexpr KindName KindNames[] = {
        {RecordKind::Module, "module"},
        {RecordKind::BoolType, "type.bool"},
        {RecordKind::IntegerType, "type.integer"},
        {RecordKind::FloatType, "type.float"},
        {RecordKind::VoidType, "type.void"},
        {RecordKind::NullPointerType, "type.nullptr"},
        {RecordKind::MetaType, "type.meta"},
        {RecordKind::VariadicType, "type.variadic"},
        {RecordKind::PointerType, "type.pointer"},
        {RecordKind::ReferenceType, "type.reference"},
        {RecordKind::ArrayType, "type.array"},
        {RecordKind::FunctionType, "type.function"},
        {RecordKind::ClassType, "type.class"},
        {RecordKind::InterfaceType, "type.interface"},
        {RecordKind::StructType, "type.struct"},
        {RecordKind::QualifiedType, "type.qualified"},
        {RecordKind::Field, "field"},
        {RecordKind::Function, "define"},
        {RecordKind::Parameter, "parameter"},
        {RecordKind::Block, "block"},
        {RecordKind::Global, "global"},
        {RecordKind::Import, "import"},
        {RecordKind::Export, "export"},
        {RecordKind::BoolConstant, "const.bool"},
        {RecordKind::IntegerConstant, "const.integer"},
        {RecordKind::FloatConstant, "const.float"},
        {RecordKind::NullPointerConstant, "const.nullptr"},
        {RecordKind::StringConstant, "const.string"},
        {RecordKind::ArrayConstant, "const.array"},
        {RecordKind::Alloca, "alloca"},
        {RecordKind::Load, "load"},
        {RecordKind::Store, "store"},
        {RecordKind::Neg, "neg"},
        {RecordKind::ICmp, "icmp"},
        {RecordKind::Branch, "br"},
        {RecordKind::CondBranch, "br_if"},
        {RecordKind::Return, "ret"},
        {RecordKind::Call, "call"},
        {RecordKind::GetElementPtr, "getelementptr"},
    };

    constexpr std::uint64_t NoVirtualSlot = std::numeric_limits<std::uint64_t>::max();

    // Array positions are the v1 wire values; native enum ordinals are deliberately irrelevant.
    constexpr SymbolLinkage SymbolLinkages[] = {
        SymbolLinkage::Local,
        SymbolLinkage::Import,
        SymbolLinkage::Export,
    };

    constexpr LanguageLinkage LanguageLinkages[] = {
        LanguageLinkage::Ink,
        LanguageLinkage::C,
    };

    constexpr FunctionVisibility Visibilities[] = {
        FunctionVisibility::Public,
        FunctionVisibility::Private,
    };

    constexpr MethodDispatch DispatchKinds[] = {
        MethodDispatch::Direct,
        MethodDispatch::Virtual,
        MethodDispatch::Abstract,
    };

    constexpr ICmpPredicate Predicates[] = {
        ICmpPredicate::Equal,
        ICmpPredicate::NotEqual,
        ICmpPredicate::UnsignedGreaterThan,
        ICmpPredicate::UnsignedGreaterEqual,
        ICmpPredicate::UnsignedLessThan,
        ICmpPredicate::UnsignedLessEqual,
        ICmpPredicate::SignedGreaterThan,
        ICmpPredicate::SignedGreaterEqual,
        ICmpPredicate::SignedLessThan,
        ICmpPredicate::SignedLessEqual,
    };

    constexpr target::ByteOrder ByteOrders[] = {
        target::ByteOrder::LittleEndian,
        target::ByteOrder::BigEndian,
    };

    template <typename T, std::size_t Count>
    std::uint64_t encodeEnum(T Value, const T (&Entries)[Count]) noexcept
    {
      for (std::size_t Index = 0; Index < Count; ++Index)
      {
        if (Value == Entries[Index])
        {
          return Index;
        }
      }
      return NoVirtualSlot;
    }

    std::string nameText(core::Name Name)
    {
      return std::string(core::NamePool::instance().text(Name));
    }

    std::array<std::uint64_t, 5> targetAlignments(const target::TargetContext &Target)
    {
      return {Target.integerAlignment(8), Target.integerAlignment(32), Target.integerAlignment(64), Target.integerAlignment(128), Target.floatAlignment(64)};
    }

    bool isNominal(RecordKind Kind) noexcept
    {
      return Kind == RecordKind::ClassType || Kind == RecordKind::InterfaceType || Kind == RecordKind::StructType;
    }

    bool isInstruction(RecordKind Kind) noexcept
    {
      return Kind >= RecordKind::Alloca && Kind <= RecordKind::GetElementPtr;
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
        auto Shape = [&](std::size_t Numbers, std::size_t References, std::size_t Strings)
        {
          return N.size() == Numbers && R.size() == References && S.size() == Strings;
        };
        auto Flag = [&](std::size_t Position)
        {
          return Position < N.size() && N[Position] <= 1;
        };
        bool Valid = false;
        switch (Entry.Kind)
        {
        case RecordKind::Module:
          Valid = Index == 0 && N.empty() && R.size() == 2;
          break;
        case RecordKind::BoolType:
        case RecordKind::VoidType:
        case RecordKind::NullPointerType:
          Valid = Shape(1, 0, 0) && Flag(0);
          break;
        case RecordKind::IntegerType:
          Valid = Shape(3, 0, 0) && Flag(0) && Flag(2) && N[1] <= core::IntegerBits::MaxBitWidth && N[1] > 0;
          break;
        case RecordKind::FloatType:
          Valid = Shape(2, 0, 0) && Flag(0) && (N[1] == 32 || N[1] == 64);
          break;
        case RecordKind::MetaType:
        case RecordKind::VariadicType:
          Valid = Shape(0, 0, 0);
          break;
        case RecordKind::PointerType:
        case RecordKind::ReferenceType:
        case RecordKind::QualifiedType:
          Valid = Shape(1, 1, 0) && Flag(0);
          break;
        case RecordKind::ArrayType:
          Valid = Shape(2, 1, 0) && Flag(0);
          break;
        case RecordKind::FunctionType:
          Valid = !N.empty() && N.size() == R.size() && S.empty() && std::all_of(N.begin(), N.end(), [](std::uint64_t Value)
                                                                                 {
                                                                                   return Value <= 1;
                                                                                 });
          break;
        case RecordKind::ClassType:
        case RecordKind::InterfaceType:
        case RecordKind::StructType:
          Valid = Shape(3, 0, 1) && Flag(2) && N[1] != 0 && (N[1] & (N[1] - 1)) == 0;
          break;
        case RecordKind::Field:
          Valid = Shape(1, 3, 1);
          break;
        case RecordKind::Function:
          Valid = Shape(7, 2, 1) && N[0] <= 2 && N[1] <= 1 && N[2] <= 1 && N[3] <= 2 && Flag(4) && Flag(5) && (N[6] <= NullId || N[6] == NoVirtualSlot);
          break;
        case RecordKind::Parameter:
          Valid = Shape(1, 1, 1) && N[0] <= std::numeric_limits<std::size_t>::max();
          break;
        case RecordKind::Block:
          Valid = Shape(0, 1, 1);
          break;
        case RecordKind::Global:
          Valid = Shape(1, 2, 1) && Flag(0);
          break;
        case RecordKind::Import:
          Valid = N.empty() && R.size() == 1 && !S.empty();
          break;
        case RecordKind::Export:
          Valid = Shape(0, 1, 1);
          break;
        case RecordKind::BoolConstant:
          Valid = Shape(1, 1, 0) && Flag(0);
          break;
        case RecordKind::IntegerConstant:
          Valid = !N.empty() && R.size() == 1 && S.empty();
          break;
        case RecordKind::FloatConstant:
          Valid = Shape(1, 1, 0);
          break;
        case RecordKind::NullPointerConstant:
          Valid = Shape(0, 1, 0);
          break;
        case RecordKind::StringConstant:
          Valid = Shape(0, 1, 1);
          break;
        case RecordKind::ArrayConstant:
          Valid = N.empty() && !R.empty() && S.empty();
          break;
        case RecordKind::Alloca:
          Valid = Shape(1, 2, 1) && Flag(0);
          break;
        case RecordKind::Load:
        case RecordKind::Neg:
        case RecordKind::Branch:
        case RecordKind::Return:
          Valid = Shape(0, 3, 0);
          break;
        case RecordKind::Store:
          Valid = Shape(0, 4, 0);
          break;
        case RecordKind::ICmp:
          Valid = Shape(1, 4, 0) && N[0] <= 9;
          break;
        case RecordKind::CondBranch:
          Valid = Shape(0, 5, 0);
          break;
        case RecordKind::Call:
          Valid = N.empty() && R.size() >= 4 && S.size() == R.size() - 4;
          break;
        case RecordKind::GetElementPtr:
          Valid = N.size() == 1 && Flag(0) && R.size() >= 4 && S.empty();
          break;
        default:
          return Reject("Unknown IR record kind", IRSerializationErrorCode::UnsupportedKind);
        }
        if (!Valid)
        {
          return Reject("Malformed IR record attributes");
        }
        for (std::size_t Operand = 0; Operand < R.size(); ++Operand)
        {
          const bool Nullable = Entry.Kind == RecordKind::Module || (Entry.Kind == RecordKind::Function && Operand == 1) || (Entry.Kind == RecordKind::Field && Operand == 2) || (Entry.Kind == RecordKind::Global && Operand == 1) || (Entry.Kind == RecordKind::Return && Operand == 2);
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
          Data.ByteOrder = static_cast<std::uint32_t>(encodeEnum(Owner.targetContext().byteOrder(), ByteOrders));
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
            Result.Kind = RecordKind::Module;
            Result.References = {reference(Owner.initializationFunction()), reference(Owner.deinitializationFunction())};
            for (core::Name Component : Owner.moduleID().Components)
            {
              Result.Strings.push_back(nameText(Component));
            }
            for (const auto &Import : Owner.imports())
            {
              Record Entry{RecordKind::Import, {}, {reference(Import.SymbolType)}, {}};
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
                appendRecord({RecordKind::Export, {}, {reference(Entry.Declaration)}, {nameText(Export.Symbol.SymbolName)}});
              }
            }
            break;
          case ValueKind::Function:
          {
            const auto &FunctionValue = static_cast<const Function &>(Value);
            Result.Kind = RecordKind::Function;
            Result.References = {reference(FunctionValue.functionType()), reference(FunctionOwners[&FunctionValue])};
            Result.Numbers = {encodeEnum(FunctionValue.symbolLinkage(), SymbolLinkages), encodeEnum(FunctionValue.languageLinkage(), LanguageLinkages), encodeEnum(FunctionValue.visibility(), Visibilities), 0, 0, 0, NoVirtualSlot};
            Result.Strings = {nameText(FunctionValue.name())};
            if (auto It = Methods.find(&FunctionValue); It != Methods.end())
            {
              const auto &Info = It->second->virtualInfo();
              Result.Numbers[3] = encodeEnum(Info.Dispatch, DispatchKinds);
              Result.Numbers[4] = Info.Override;
              Result.Numbers[5] = Info.Final;
              Result.Numbers[6] = Info.Slot ? static_cast<std::uint64_t>(*Info.Slot) : NoVirtualSlot;
            }
            break;
          }
          case ValueKind::Parameter:
          {
            const auto &ParameterValue = static_cast<const Parameter &>(Value);
            Result = {RecordKind::Parameter, {ParameterValue.index()}, {reference(ParameterValue.function())}, {nameText(ParameterValue.name())}};
            break;
          }
          case ValueKind::BasicBlock:
          {
            const auto &Block = static_cast<const BasicBlock &>(Value);
            Result = {RecordKind::Block, {}, {reference(Block.function())}, {nameText(Block.name())}};
            break;
          }
          case ValueKind::GlobalVariable:
          {
            const auto &Global = static_cast<const GlobalVariable &>(Value);
            Result = {RecordKind::Global, {IRModelAccess::isStringStorage(Owner, Global)}, {reference(Global.type()), reference(Global.initializer())}, {nameText(Global.name())}};
            break;
          }
          default:
            fail("Unsupported IR value kind", IRSerializationErrorCode::UnsupportedKind);
            break;
          }
        }

        void collectType(const Type &Value, Record &Result)
        {
          Result.Numbers = {Value.isMutable()};
          if (NominalType::classof(&Value))
          {
            const auto &Nominal = static_cast<const NominalType &>(Value);
            if (Nominal.declarationType() != &Nominal)
            {
              Result.Kind = RecordKind::QualifiedType;
              Result.References = {reference(Nominal.declarationType())};
              return;
            }
            Result.Kind = Value.kind() == ValueKind::Class ? RecordKind::ClassType : Value.kind() == ValueKind::Interface ? RecordKind::InterfaceType
                                                                                                                          : RecordKind::StructType;
            Result.Numbers = {Nominal.size(), Nominal.alignment(), Nominal.isFrozen()};
            Result.Strings = {nameText(Nominal.name())};
            for (const auto &Field : Nominal.fields())
            {
              appendRecord({RecordKind::Field, {Field.offset()}, {reference(&Nominal), reference(Field.type()), reference(Field.initializer())}, {nameText(Field.name())}});
            }
            return;
          }
          switch (Value.kind())
          {
          case ValueKind::Bool:
            Result.Kind = RecordKind::BoolType;
            break;
          case ValueKind::Void:
            Result.Kind = RecordKind::VoidType;
            break;
          case ValueKind::NullPointer:
            Result.Kind = RecordKind::NullPointerType;
            break;
          case ValueKind::Integer:
          {
            const auto &Integer = static_cast<const IntegerType &>(Value);
            Result.Kind = RecordKind::IntegerType;
            Result.Numbers = {Value.isMutable(), Integer.width(), Integer.isSigned()};
            break;
          }
          case ValueKind::Float:
            Result.Kind = RecordKind::FloatType;
            Result.Numbers.push_back(static_cast<const FloatType &>(Value).width());
            break;
          case ValueKind::MetaType:
            Result.Kind = RecordKind::MetaType;
            Result.Numbers.clear();
            break;
          case ValueKind::Variadic:
            Result.Kind = RecordKind::VariadicType;
            Result.Numbers.clear();
            break;
          case ValueKind::Pointer:
            Result.Kind = RecordKind::PointerType;
            Result.References = {reference(static_cast<const PointerType &>(Value).pointeeType())};
            break;
          case ValueKind::Reference:
            Result.Kind = RecordKind::ReferenceType;
            Result.References = {reference(static_cast<const ReferenceType &>(Value).referencedType())};
            break;
          case ValueKind::Array:
          {
            const auto &Array = static_cast<const ArrayType &>(Value);
            Result.Kind = RecordKind::ArrayType;
            Result.Numbers.push_back(Array.elementCount());
            Result.References = {reference(Array.elementType())};
            break;
          }
          case ValueKind::FunctionType:
          {
            const auto &Signature = static_cast<const FunctionType &>(Value);
            Result.Kind = RecordKind::FunctionType;
            Result.References = {reference(Signature.returnType())};
            for (const auto &Parameter : Signature.parameters())
            {
              Result.Numbers.push_back(Parameter.Variadic);
              Result.References.push_back(reference(Parameter.ParameterType));
            }
            break;
          }
          default:
            fail("Unsupported IR type kind", IRSerializationErrorCode::UnsupportedKind);
            break;
          }
        }

        void collectConstant(const Constant &Value, Record &Result)
        {
          Result.References = {reference(Value.type())};
          switch (Value.kind())
          {
          case ValueKind::BoolConstant:
            Result.Kind = RecordKind::BoolConstant;
            Result.Numbers = {static_cast<const BoolConstant &>(Value).value()};
            break;
          case ValueKind::IntegerConstant:
          {
            Result.Kind = RecordKind::IntegerConstant;
            const auto Words = static_cast<const IntegerConstant &>(Value).bits().words();
            Result.Numbers.assign(Words.begin(), Words.end());
            break;
          }
          case ValueKind::FloatConstant:
            Result.Kind = RecordKind::FloatConstant;
            Result.Numbers = {static_cast<const FloatConstant &>(Value).bits()};
            break;
          case ValueKind::NullPointerConstant:
            Result.Kind = RecordKind::NullPointerConstant;
            break;
          case ValueKind::StringConstant:
            Result.Kind = RecordKind::StringConstant;
            Result.Strings = {std::string(static_cast<const StringConstant &>(Value).value())};
            break;
          case ValueKind::ArrayConstant:
            Result.Kind = RecordKind::ArrayConstant;
            for (const auto *Element : static_cast<const ArrayConstant &>(Value).elements())
            {
              Result.References.push_back(reference(Element));
            }
            break;
          default:
            fail("Unsupported IR constant kind", IRSerializationErrorCode::UnsupportedKind);
            break;
          }
        }

        void collectInstruction(const Instruction &Value, Record &Result)
        {
          Result.References = {reference(Value.type()), reference(Value.parentBlock())};
          auto Add = [&](const ink::ir::Value *Operand)
          {
            Result.References.push_back(reference(Operand));
          };
          switch (Value.kind())
          {
          case ValueKind::AllocaInstruction:
          {
            const auto &Instruction = static_cast<const AllocaInstruction &>(Value);
            Result.Kind = RecordKind::Alloca;
            Result.Numbers = {Instruction.isInitialized()};
            Result.Strings = {nameText(Instruction.name())};
            break;
          }
          case ValueKind::LoadInstruction:
            Result.Kind = RecordKind::Load;
            Add(static_cast<const LoadInstruction &>(Value).pointer());
            break;
          case ValueKind::StoreInstruction:
          {
            const auto &Instruction = static_cast<const StoreInstruction &>(Value);
            Result.Kind = RecordKind::Store;
            Add(Instruction.storedValue());
            Add(Instruction.pointer());
            break;
          }
          case ValueKind::NegInstruction:
            Result.Kind = RecordKind::Neg;
            Add(static_cast<const NegInstruction &>(Value).operand());
            break;
          case ValueKind::ICmpInstruction:
          {
            const auto &Instruction = static_cast<const ICmpInstruction &>(Value);
            Result.Kind = RecordKind::ICmp;
            Result.Numbers = {encodeEnum(Instruction.predicate(), Predicates)};
            Add(Instruction.left());
            Add(Instruction.right());
            break;
          }
          case ValueKind::BranchInstruction:
            Result.Kind = RecordKind::Branch;
            Add(static_cast<const BranchInstruction &>(Value).target());
            break;
          case ValueKind::ConditionalBranchInstruction:
          {
            const auto &Instruction = static_cast<const ConditionalBranchInstruction &>(Value);
            Result.Kind = RecordKind::CondBranch;
            Add(Instruction.condition());
            Add(Instruction.trueTarget());
            Add(Instruction.falseTarget());
            break;
          }
          case ValueKind::ReturnInstruction:
            Result.Kind = RecordKind::Return;
            Add(static_cast<const ReturnInstruction &>(Value).returnedValue());
            break;
          case ValueKind::CallInstruction:
          {
            const auto &Instruction = static_cast<const CallInstruction &>(Value);
            Result.Kind = RecordKind::Call;
            Add(Instruction.functionType());
            Add(Instruction.callee());
            for (const auto &Argument : Instruction.arguments())
            {
              Add(Argument.value());
              Result.Strings.push_back(nameText(Argument.name()));
            }
            break;
          }
          case ValueKind::GetElementPtrInstruction:
          {
            const auto &Instruction = static_cast<const GetElementPtrInstruction &>(Value);
            Result.Kind = RecordKind::GetElementPtr;
            Result.Numbers = {Instruction.isInBounds()};
            Add(Instruction.sourceElementType());
            Add(Instruction.pointer());
            for (const auto *Index : Instruction.indices())
            {
              Add(Index);
            }
            break;
          }
          default:
            fail("Unsupported IR instruction kind", IRSerializationErrorCode::UnsupportedKind);
            break;
          }
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
              auto Name = intern(Entry.Strings[0]);
              NominalType *Value = Entry.Kind == RecordKind::ClassType ? static_cast<NominalType *>(Result.createClassType(Entry.Numbers[0], Entry.Numbers[1], Name)) : Entry.Kind == RecordKind::InterfaceType ? static_cast<NominalType *>(Result.createInterfaceType(Entry.Numbers[0], Entry.Numbers[1], Name))
                                                                                                                                                                                                                : static_cast<NominalType *>(Result.createStructType(Entry.Numbers[0], Entry.Numbers[1], Name));
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
            resolve(Entry.References[0]);
            const auto *Signature = as<FunctionType>(Entry.References[0]);
            if (!Signature)
            {
              break;
            }
            auto Callable = std::make_unique<Function>(*Signature, intern(Entry.Strings[0]), SymbolLinkages[Entry.Numbers[0]], LanguageLinkages[Entry.Numbers[1]], Visibilities[Entry.Numbers[2]]);
            set(Index, Callable.get());
            if (Entry.References[1] == NullId)
            {
              if (Entry.Numbers[3] || Entry.Numbers[4] || Entry.Numbers[5] || Entry.Numbers[6] != NoVirtualSlot)
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
              as<NominalType>(Entry.References[1]);
              Functions[Index] = std::move(Callable);
            }
          }
          std::unordered_set<const Parameter *> SeenParameters;
          for (std::size_t Index = 1; Index < Data.Records.size() && !Error; ++Index)
          {
            const auto &Entry = Data.Records[Index];
            if (Entry.Kind == RecordKind::Parameter)
            {
              auto *Parent = mutableAs<Function>(Entry.References[0]);
              if (!Parent)
              {
                break;
              }
              auto *Value = Parent->parameter(static_cast<std::size_t>(Entry.Numbers[0]));
              if (!Value || !SeenParameters.insert(Value).second)
              {
                fail("Invalid or duplicate formal parameter index");
                break;
              }
              Value->setName(intern(Entry.Strings[0]));
              set(Index, Value);
            }
            else if (Entry.Kind == RecordKind::Block)
            {
              auto *Parent = mutableAs<Function>(Entry.References[0]);
              if (Parent)
              {
                set(Index, Parent->createBlock(intern(Entry.Strings[0])));
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
              resolve(Entry.References[0]);
              resolve(Entry.References[1]);
              const auto *Address = as<PointerType>(Entry.References[0]);
              const auto *Initializer = as<Constant>(Entry.References[1], true);
              if (Error || !Address)
              {
                break;
              }
              auto Variable = std::make_unique<GlobalVariable>(*Address, intern(Entry.Strings[0]));
              if (!Variable->setInitializer(Initializer))
              {
                fail("Incompatible global initializer");
                break;
              }
              auto *Value = Result.appendGlobalVariable(std::move(Variable));
              set(Index, Value);
              if (Value && Entry.Numbers[0] && !IRModelAccess::registerStringStorage(Result, *Value))
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
              auto *Parent = mutableAs<BasicBlock>(Data.Records[Index].References[1]);
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
              auto *Owner = mutableAs<NominalType>(Entry.References[0]);
              const auto *Type = as<ink::ir::Type>(Entry.References[1]);
              const auto *Initializer = as<Constant>(Entry.References[2], true);
              if (!Error && !Owner->addField(Field(Type, intern(Entry.Strings[0]), Entry.Numbers[0], Initializer)))
              {
                fail("Invalid nominal field metadata");
              }
            }
            else if (Entry.Kind == RecordKind::Function && Functions[Index])
            {
              auto *Owner = mutableAs<NominalType>(Entry.References[1]);
              MethodVirtualInfo Info{DispatchKinds[Entry.Numbers[3]], Entry.Numbers[4] != 0, Entry.Numbers[5] != 0, std::nullopt};
              if (Entry.Numbers[6] != NoVirtualSlot)
              {
                Info.Slot = static_cast<std::uint32_t>(Entry.Numbers[6]);
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
              Import.SymbolType = as<Type>(Entry.References[0]);
              for (std::size_t Part = 0; Part + 1 < Entry.Strings.size(); ++Part)
              {
                Import.Target.Module.Components.push_back(intern(Entry.Strings[Part]));
              }
              Import.Target.Symbol.SymbolName = intern(Entry.Strings.back());
              Result.imports().push_back(std::move(Import));
            }
            else if (Entry.Kind == RecordKind::Export)
            {
              const auto *Declaration = value(Entry.References[0]);
              if (!Error && !Result.addExport({intern(Entry.Strings[0])}, *Declaration))
              {
                fail("Invalid exported declaration");
              }
            }
          }
          for (std::size_t Index = 1; Index < Data.Records.size() && !Error; ++Index)
          {
            const auto &Entry = Data.Records[Index];
            if (isNominal(Entry.Kind) && Entry.Numbers[2])
            {
              mutableAs<NominalType>(static_cast<std::uint32_t>(Index))->freeze();
            }
          }
          if (Error)
          {
            return std::unexpected(std::move(*Error));
          }
          auto *Initialization = mutableAs<Function>(Data.Records[0].References[0], true);
          auto *Deinitialization = mutableAs<Function>(Data.Records[0].References[1], true);
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
            Constructed = Pool.get<BoolType>(N[0] != 0);
            break;
          case RecordKind::IntegerType:
            Constructed = Pool.get<IntegerType>(static_cast<std::uint32_t>(N[1]), N[2] != 0, N[0] != 0);
            break;
          case RecordKind::FloatType:
            Constructed = Pool.get<FloatType>(static_cast<std::uint32_t>(N[1]), N[0] != 0);
            break;
          case RecordKind::VoidType:
            Constructed = Pool.get<VoidType>(N[0] != 0);
            break;
          case RecordKind::NullPointerType:
            Constructed = Pool.get<NullPointerType>(N[0] != 0);
            break;
          case RecordKind::MetaType:
            Constructed = Pool.get<MetaType>();
            break;
          case RecordKind::VariadicType:
            Constructed = Pool.get<VariadicType>();
            break;
          case RecordKind::QualifiedType:
            Constructed = Pool.get(as<NominalType>(R[0]), N[0] != 0);
            break;
          case RecordKind::PointerType:
            Constructed = Pool.get<PointerType>(as<Type>(R[0]), N[0] != 0);
            break;
          case RecordKind::ReferenceType:
            Constructed = Pool.get<ReferenceType>(as<Type>(R[0]), N[0] != 0);
            break;
          case RecordKind::ArrayType:
            Constructed = Pool.get<ArrayType>(as<Type>(R[0]), N[1], N[0] != 0);
            break;
          case RecordKind::FunctionType:
          {
            std::vector<FunctionTypeParameter> Parameters;
            Parameters.reserve(R.size() - 1);
            for (std::size_t Index = 1; Index < R.size(); ++Index)
            {
              Parameters.emplace_back(as<Type>(R[Index]), N[Index] != 0);
            }
            Constructed = Pool.get<FunctionType>(as<Type>(R[0]), std::span<const FunctionTypeParameter>(Parameters), N[0] != 0);
            break;
          }
          case RecordKind::BoolConstant:
            Constructed = Constants.get<BoolConstant>(N[0] != 0);
            break;
          case RecordKind::IntegerConstant:
          {
            const auto *Type = as<IntegerType>(R[0]);
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
            const auto *Type = as<FloatType>(R[0]);
            if (Type)
            {
              const auto Bits = core::IntegerBits::fromUnsigned(Type->width(), N[0]);
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
            Constructed = Constants.get<StringConstant>(Entry.Strings[0]);
            break;
          case RecordKind::ArrayConstant:
          {
            std::vector<const Constant *> Elements;
            Elements.reserve(R.size() - 1);
            for (std::size_t Index = 1; Index < R.size(); ++Index)
            {
              Elements.push_back(as<Constant>(R[Index]));
            }
            Constructed = Constants.get<ArrayConstant>(as<ArrayType>(R[0]), std::move(Elements));
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
          if (Constructed && Constant::classof(Constructed) && Constructed->type() != as<Type>(R[0]))
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
          const auto *Type = as<ink::ir::Type>(R[0]);
          as<BasicBlock>(R[1]);
          if (Error || !Type)
          {
            return;
          }
          std::unique_ptr<Instruction> InstructionValue;
          switch (Entry.Kind)
          {
          case RecordKind::Alloca:
          {
            const auto *Pointer = as<PointerType>(R[0]);
            if (Pointer)
            {
              auto Value = std::make_unique<AllocaInstruction>(*Pointer);
              IRModelAccess::setInitialized(*Value, N[0] != 0);
              Value->setName(intern(Entry.Strings[0]));
              InstructionValue = std::move(Value);
            }
            break;
          }
          case RecordKind::Load:
            InstructionValue = std::make_unique<LoadInstruction>(*Type, *value(R[2]));
            break;
          case RecordKind::Store:
          {
            const auto *Void = as<VoidType>(R[0]);
            if (Void)
            {
              InstructionValue = std::make_unique<StoreInstruction>(*Void, *value(R[2]), *value(R[3]));
            }
            break;
          }
          case RecordKind::Neg:
            InstructionValue = std::make_unique<NegInstruction>(*value(R[2]));
            break;
          case RecordKind::ICmp:
          {
            const auto *Bool = as<BoolType>(R[0]);
            if (Bool)
            {
              InstructionValue = std::make_unique<ICmpInstruction>(*Bool, Predicates[N[0]], *value(R[2]), *value(R[3]));
            }
            break;
          }
          case RecordKind::Branch:
          {
            const auto *Void = as<VoidType>(R[0]);
            auto *Target = mutableAs<BasicBlock>(R[2]);
            if (!Error)
            {
              InstructionValue = std::make_unique<BranchInstruction>(*Void, *Target);
            }
            break;
          }
          case RecordKind::CondBranch:
          {
            const auto *Void = as<VoidType>(R[0]);
            auto *True = mutableAs<BasicBlock>(R[3]);
            auto *False = mutableAs<BasicBlock>(R[4]);
            if (!Error)
            {
              InstructionValue = std::make_unique<ConditionalBranchInstruction>(*Void, *value(R[2]), *True, *False);
            }
            break;
          }
          case RecordKind::Return:
          {
            const auto *Void = as<VoidType>(R[0]);
            if (Void)
            {
              InstructionValue = std::make_unique<ReturnInstruction>(*Void, value(R[2], true));
            }
            break;
          }
          case RecordKind::Call:
          {
            const auto *Signature = as<FunctionType>(R[2]);
            std::vector<Argument> Arguments;
            Arguments.reserve(R.size() - 4);
            for (std::size_t Index = 4; Index < R.size(); ++Index)
            {
              Arguments.emplace_back(*value(R[Index]), intern(Entry.Strings[Index - 4]));
            }
            if (!Error)
            {
              InstructionValue = std::make_unique<CallInstruction>(*Signature, *value(R[3]), std::move(Arguments));
            }
            break;
          }
          case RecordKind::GetElementPtr:
          {
            const auto *Pointer = as<PointerType>(R[0]);
            const auto *Source = as<ink::ir::Type>(R[2]);
            std::vector<const Value *> Indices;
            Indices.reserve(R.size() - 4);
            for (std::size_t Index = 4; Index < R.size(); ++Index)
            {
              Indices.push_back(value(R[Index]));
            }
            if (!Error)
            {
              InstructionValue = std::make_unique<GetElementPtrInstruction>(*Pointer, *Source, *value(R[3]), std::move(Indices), N[0] != 0);
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

  std::string_view recordKindName(RecordKind Kind) noexcept
  {
    for (const auto &Entry : KindNames)
    {
      if (Entry.Kind == Kind)
      {
        return Entry.Name;
      }
    }
    return {};
  }

  IRSerializationResult<Model> collectModel(const Module &ModuleValue)
  {
    return Collector(ModuleValue).run();
  }

  IRSerializationResult<std::unique_ptr<Module>> decodeModel(IRContext &Context, const Model &Data, const IRReadOptions &Options)
  {
    const auto &Target = Context.targetContext();
    if (Data.PointerBits != static_cast<std::uint32_t>(Target.pointerWidth()) || Data.ByteOrder != encodeEnum(Target.byteOrder(), ByteOrders) || Data.NativeAbi != Target.isNativeAbiCompatible() || Data.Alignments != targetAlignments(Target))
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
