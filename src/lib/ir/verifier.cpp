#include "ink/ir/verifier.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <limits>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace ink::ir
{
  using core::Diagnostic;
  using core::DiagnosticClass;
  using core::DiagnosticKind;

  namespace
  {
    class DiagnosticCollector
    {
    public:
      explicit DiagnosticCollector(DiagnosticClass Class) : Class(Class)
      {
      }

      template <DiagnosticKind Kind, typename... ArgumentTypes>
      void add(ArgumentTypes &&...Arguments)
      {
        Diagnostic DiagnosticEntry = core::makeDiagnostic<Kind>({}, std::forward<ArgumentTypes>(Arguments)...);
        DiagnosticEntry.Class = Class;
        Diagnostics.push_back(std::move(DiagnosticEntry));
      }

      void report(IRContext &Context) const
      {
        for (const Diagnostic &DiagnosticEntry : Diagnostics)
        {
          Context.diagnosticEngine().report(DiagnosticEntry);
        }
      }

      std::vector<Diagnostic> takeDiagnostics()
      {
        return std::move(Diagnostics);
      }

    private:
      DiagnosticClass Class;
      std::vector<Diagnostic> Diagnostics;
    };

    bool isValidType(const Type *TypeValue)
    {
      if (TypeValue == nullptr)
      {
        return false;
      }
      switch (TypeValue->kind())
      {
#define INK_IR_TYPE(Name, Spelling) \
  case TypeKind::Name:              \
    return true;
#include "ink/ir/ir.def"
      case TypeKind::Struct:
        return true;
      case TypeKind::Count:
        return false;
      }
      return false;
    }

    bool isValidType(const Module &ModuleValue, const Type *TypeValue)
    {
      if (!isValidType(TypeValue))
      {
        return false;
      }
      if (TypeValue->kind() != TypeKind::Struct)
      {
        return true;
      }
      return std::find(ModuleValue.StructTypes.begin(), ModuleValue.StructTypes.end(), static_cast<const StructType *>(TypeValue)) != ModuleValue.StructTypes.end();
    }

    bool isNameStart(char Character)
    {
      const unsigned char Value = static_cast<unsigned char>(Character);
      return std::isalpha(Value) != 0 || Character == '_' || Character == '.' || Character == '$';
    }

    bool isNameContinue(char Character)
    {
      const unsigned char Value = static_cast<unsigned char>(Character);
      return std::isalnum(Value) != 0 || Character == '_' || Character == '.' || Character == '$';
    }

    bool isValidName(const std::string &Name)
    {
      if (Name.empty() || !isNameStart(Name.front()))
      {
        return false;
      }
      for (const char Character : Name)
      {
        if (!isNameContinue(Character))
        {
          return false;
        }
      }
      return true;
    }

    void verifyIntegerConstant(const IntegerConstant &Constant, const std::string &FunctionName, DiagnosticCollector &Diagnostics)
    {
      const std::int64_t Value = Constant.value();
      switch (Constant.type().kind())
      {
      case TypeKind::Bool:
        if (Value != 0 && Value != 1)
        {
          Diagnostics.add<DiagnosticKind::IrBoolConstantOutOfRange>(FunctionName, Value);
        }
        return;
      case TypeKind::Byte:
        if (Value < 0 || Value > 255)
        {
          Diagnostics.add<DiagnosticKind::IrByteConstantOutOfRange>(FunctionName, Value);
        }
        return;
      case TypeKind::I32:
        if (Value < std::numeric_limits<std::int32_t>::min() || Value > std::numeric_limits<std::int32_t>::max())
        {
          Diagnostics.add<DiagnosticKind::IrI32ConstantOutOfRange>(FunctionName, Value);
        }
        return;
      case TypeKind::PointerSize:
        if (Value < 0)
        {
          Diagnostics.add<DiagnosticKind::IrPointerSizeConstantNegative>(FunctionName, Value);
        }
        return;
      case TypeKind::Void:
      case TypeKind::BytePointer:
      case TypeKind::ConstBytePointer:
      case TypeKind::Struct:
        Diagnostics.add<DiagnosticKind::IrIntegerConstantInvalidType>(FunctionName, typeKindName(Constant.type().kind()));
        return;
      case TypeKind::Count:
        break;
      }
      Diagnostics.add<DiagnosticKind::IrUnknownIntegerConstantType>(FunctionName);
    }

    void verifyOperand(const Module &ModuleValue, const Value &OperandValue, const std::unordered_map<std::size_t, const Type *> &AvailableValues, const std::string &FunctionName, DiagnosticCollector &Diagnostics)
    {
      if (!isValidType(ModuleValue, &OperandValue.type()) || OperandValue.type().kind() == TypeKind::Void)
      {
        Diagnostics.add<DiagnosticKind::IrOperandInvalidType>(FunctionName);
        return;
      }

      if (OperandValue.kind() == ValueKind::IntegerConstant)
      {
        verifyIntegerConstant(static_cast<const IntegerConstant &>(OperandValue), FunctionName, Diagnostics);
        return;
      }

      if (OperandValue.kind() == ValueKind::ValueOperand)
      {
        const ValueId Id = static_cast<const ValueOperand &>(OperandValue).id();
        const auto Definition = AvailableValues.find(Id.value());
        if (!Id.valid() || Definition == AvailableValues.end())
        {
          Diagnostics.add<DiagnosticKind::IrUnavailableSsaValue>(FunctionName, Id.value());
          return;
        }
        if (Definition->second != &OperandValue.type())
        {
          Diagnostics.add<DiagnosticKind::IrSsaOperandTypeMismatch>(FunctionName, Id.value());
        }
        return;
      }

      if (OperandValue.kind() == ValueKind::ZeroInitializer)
      {
        return;
      }

      if (OperandValue.kind() != ValueKind::GlobalAddressOperand)
      {
        Diagnostics.add<DiagnosticKind::IrUnknownOperandKind>(FunctionName);
        return;
      }

      const GlobalAddressOperand &Address = static_cast<const GlobalAddressOperand &>(OperandValue);
      if (OperandValue.type().kind() != TypeKind::ConstBytePointer)
      {
        Diagnostics.add<DiagnosticKind::IrGlobalAddressWrongType>(FunctionName);
        return;
      }
      if (!Address.global().valid() || Address.global().value() >= ModuleValue.ByteConstants.size())
      {
        Diagnostics.add<DiagnosticKind::IrInvalidGlobalByteConstantReference>(FunctionName, Address.global().value());
        return;
      }
      if (Address.byteOffset() > ModuleValue.ByteConstants[Address.global().value()].Data.size())
      {
        Diagnostics.add<DiagnosticKind::IrGlobalAddressOffsetOutOfRange>(FunctionName, Address.byteOffset(), ModuleValue.ByteConstants[Address.global().value()].Data.size());
      }
    }

    void verifyFunctionSignature(const Module &ModuleValue, const Function &FunctionValue, DiagnosticCollector &Diagnostics)
    {
      if (!isValidType(ModuleValue, FunctionValue.ResultType))
      {
        Diagnostics.add<DiagnosticKind::IrFunctionUnknownResultType>(FunctionValue.Name);
      }
      for (std::size_t ParameterIndex = 0; ParameterIndex < FunctionValue.ParameterTypes.size(); ++ParameterIndex)
      {
        const Type *ParameterType = FunctionValue.ParameterTypes[ParameterIndex];
        if (!isValidType(ModuleValue, ParameterType) || ParameterType->kind() == TypeKind::Void)
        {
          Diagnostics.add<DiagnosticKind::IrFunctionInvalidParameterType>(FunctionValue.Name, ParameterIndex);
        }
      }
      if (FunctionValue.Kind != FunctionKind::Definition && FunctionValue.Kind != FunctionKind::External)
      {
        Diagnostics.add<DiagnosticKind::IrFunctionUnknownKind>(FunctionValue.Name);
        return;
      }
      if (FunctionValue.Kind == FunctionKind::External)
      {
        if (FunctionValue.Convention != CallingConvention::C)
        {
          Diagnostics.add<DiagnosticKind::IrExternalFunctionWrongCallingConvention>(FunctionValue.Name);
        }
        if (!FunctionValue.Blocks.empty())
        {
          Diagnostics.add<DiagnosticKind::IrExternalFunctionHasBasicBlocks>(FunctionValue.Name);
        }
      }
      else
      {
        if (FunctionValue.Convention != CallingConvention::Ink)
        {
          Diagnostics.add<DiagnosticKind::IrDefinedFunctionWrongCallingConvention>(FunctionValue.Name);
        }
        if (FunctionValue.HasSideEffects)
        {
          Diagnostics.add<DiagnosticKind::IrDefinedFunctionHasExternalSideEffects>(FunctionValue.Name);
        }
        if (FunctionValue.Blocks.empty())
        {
          Diagnostics.add<DiagnosticKind::IrDefinedFunctionHasNoBasicBlocks>(FunctionValue.Name);
        }
      }
    }

    bool verifySsaResult(ValueId Id, const char *Operation, const std::string &FunctionName, std::size_t &ExpectedValueId, std::unordered_set<std::size_t> &DefinedValues, DiagnosticCollector &Diagnostics)
    {
      const bool IsValidAndUnique = Id.valid() && DefinedValues.insert(Id.value()).second;
      if (!IsValidAndUnique)
      {
        Diagnostics.add<DiagnosticKind::IrInvalidOrDuplicateSsaResult>(Operation, FunctionName, Id.value());
        ++ExpectedValueId;
        return false;
      }
      if (Id.value() != ExpectedValueId)
      {
        Diagnostics.add<DiagnosticKind::IrNonConsecutiveSsaResult>(Operation, FunctionName, ExpectedValueId, Id.value());
      }
      ++ExpectedValueId;
      return IsValidAndUnique;
    }

    void verifyFunctionBody(const Module &ModuleValue, const Function &FunctionValue, DiagnosticCollector &Diagnostics)
    {
      if (FunctionValue.Kind != FunctionKind::Definition)
      {
        return;
      }
      if (!isValidType(ModuleValue, FunctionValue.ResultType))
      {
        return;
      }

      std::unordered_set<std::string> BlockNames;
      std::unordered_set<std::size_t> DefinedValues;
      for (std::size_t ParameterIndex = 0; ParameterIndex < FunctionValue.ParameterTypes.size(); ++ParameterIndex)
      {
        DefinedValues.insert(ParameterIndex);
      }
      std::size_t ExpectedValueId = FunctionValue.ParameterTypes.size();

      for (const BasicBlock &Block : FunctionValue.Blocks)
      {
        if (!isValidName(Block.Name))
        {
          Diagnostics.add<DiagnosticKind::IrInvalidBasicBlockName>(FunctionValue.Name, Block.Name);
        }
        else if (!BlockNames.insert(Block.Name).second)
        {
          Diagnostics.add<DiagnosticKind::IrDuplicateBasicBlockName>(FunctionValue.Name, Block.Name);
        }
        if (Block.Instructions.empty())
        {
          Diagnostics.add<DiagnosticKind::IrEmptyBasicBlock>(FunctionValue.Name, Block.Name);
          continue;
        }

        std::unordered_map<std::size_t, const Type *> AvailableValues;
        for (std::size_t ParameterIndex = 0; ParameterIndex < FunctionValue.ParameterTypes.size(); ++ParameterIndex)
        {
          AvailableValues.emplace(ParameterIndex, FunctionValue.ParameterTypes[ParameterIndex]);
        }

        for (std::size_t InstructionIndex = 0; InstructionIndex < Block.Instructions.size(); ++InstructionIndex)
        {
          const std::unique_ptr<Instruction> &InstructionPointer = Block.Instructions[InstructionIndex];
          if (!InstructionPointer)
          {
            Diagnostics.add<DiagnosticKind::IrNullInstruction>(FunctionValue.Name, Block.Name);
            continue;
          }
          const Instruction &InstructionValue = *InstructionPointer;
          const bool IsLast = InstructionIndex + 1 == Block.Instructions.size();
          const InstructionKind Kind = InstructionValue.kind();
          if (Kind != InstructionKind::Call && Kind != InstructionKind::InsertValue && Kind != InstructionKind::ExtractValue && Kind != InstructionKind::Return)
          {
            Diagnostics.add<DiagnosticKind::IrUnknownInstructionKind>(FunctionValue.Name, Block.Name);
            continue;
          }
          if (isTerminator(Kind) != IsLast)
          {
            if (IsLast)
            {
              Diagnostics.add<DiagnosticKind::IrBlockMissingTerminator>(FunctionValue.Name, Block.Name);
            }
            else
            {
              Diagnostics.add<DiagnosticKind::IrEarlyTerminator>(FunctionValue.Name, Block.Name);
            }
          }

          if (Kind == InstructionKind::Call)
          {
            const CallInstruction *Call = static_cast<const CallInstruction *>(&InstructionValue);
            if (!Call->Callee.valid() || Call->Callee.value() >= ModuleValue.Functions.size())
            {
              Diagnostics.add<DiagnosticKind::IrCallInvalidCallee>(FunctionValue.Name);
              continue;
            }
            const Function &Callee = ModuleValue.Functions[Call->Callee.value()];
            if (!isValidType(ModuleValue, Call->ResultType))
            {
              Diagnostics.add<DiagnosticKind::IrCallUnknownResultType>(Callee.Name);
              continue;
            }
            if (Call->ResultType != Callee.ResultType)
            {
              Diagnostics.add<DiagnosticKind::IrCallResultTypeMismatch>(Callee.Name);
            }
            if ((Call->ResultType->kind() == TypeKind::Void) == Call->Result.has_value())
            {
              if (Call->ResultType->kind() == TypeKind::Void)
              {
                Diagnostics.add<DiagnosticKind::IrVoidCallDefinesResult>(Callee.Name);
              }
              else
              {
                Diagnostics.add<DiagnosticKind::IrNonVoidCallMissingResult>(Callee.Name);
              }
            }
            if (Call->Arguments.size() != Callee.ParameterTypes.size())
            {
              Diagnostics.add<DiagnosticKind::IrCallArgumentCountMismatch>(Callee.Name, Callee.ParameterTypes.size(), Call->Arguments.size());
            }
            const std::size_t CheckedArgumentCount = Call->Arguments.size() < Callee.ParameterTypes.size() ? Call->Arguments.size() : Callee.ParameterTypes.size();
            for (std::size_t ArgumentIndex = 0; ArgumentIndex < Call->Arguments.size(); ++ArgumentIndex)
            {
              if (!Call->Arguments[ArgumentIndex])
              {
                Diagnostics.add<DiagnosticKind::IrNullCallArgument>(Callee.Name, ArgumentIndex);
                continue;
              }
              verifyOperand(ModuleValue, *Call->Arguments[ArgumentIndex], AvailableValues, FunctionValue.Name, Diagnostics);
              if (ArgumentIndex < CheckedArgumentCount && &Call->Arguments[ArgumentIndex]->type() != Callee.ParameterTypes[ArgumentIndex])
              {
                Diagnostics.add<DiagnosticKind::IrCallArgumentTypeMismatch>(Callee.Name, ArgumentIndex);
              }
            }
            if (Call->Result.has_value())
            {
              if (verifySsaResult(*Call->Result, "call", FunctionValue.Name, ExpectedValueId, DefinedValues, Diagnostics))
              {
                AvailableValues.emplace(Call->Result->value(), Call->ResultType);
              }
            }
            continue;
          }

          if (Kind == InstructionKind::InsertValue)
          {
            const InsertValueInstruction &Insert = static_cast<const InsertValueInstruction &>(InstructionValue);
            const StructType *AggregateType = nullptr;
            if (!isValidType(ModuleValue, Insert.ResultType) || Insert.ResultType->kind() != TypeKind::Struct)
            {
              Diagnostics.add<DiagnosticKind::IrInsertResultMustBeStruct>(FunctionValue.Name);
            }
            else
            {
              AggregateType = static_cast<const StructType *>(Insert.ResultType);
            }
            if (!Insert.Aggregate)
            {
              Diagnostics.add<DiagnosticKind::IrInsertNullAggregate>(FunctionValue.Name);
            }
            else
            {
              verifyOperand(ModuleValue, *Insert.Aggregate, AvailableValues, FunctionValue.Name, Diagnostics);
              if (&Insert.Aggregate->type() != Insert.ResultType)
              {
                Diagnostics.add<DiagnosticKind::IrInsertAggregateTypeMismatch>(FunctionValue.Name);
              }
            }
            if (!Insert.Element)
            {
              Diagnostics.add<DiagnosticKind::IrInsertNullElement>(FunctionValue.Name);
            }
            else
            {
              verifyOperand(ModuleValue, *Insert.Element, AvailableValues, FunctionValue.Name, Diagnostics);
            }
            if (AggregateType != nullptr)
            {
              if (Insert.FieldIndex >= AggregateType->fieldTypes().size())
              {
                Diagnostics.add<DiagnosticKind::IrInsertFieldIndexOutOfRange>(FunctionValue.Name, Insert.FieldIndex, AggregateType->fieldTypes().size());
              }
              else if (Insert.Element && &Insert.Element->type() != AggregateType->fieldTypes()[Insert.FieldIndex])
              {
                Diagnostics.add<DiagnosticKind::IrInsertElementTypeMismatch>(FunctionValue.Name);
              }
            }
            if (verifySsaResult(Insert.Result, "insertvalue", FunctionValue.Name, ExpectedValueId, DefinedValues, Diagnostics) && isValidType(ModuleValue, Insert.ResultType))
            {
              AvailableValues.emplace(Insert.Result.value(), Insert.ResultType);
            }
            continue;
          }

          if (Kind == InstructionKind::ExtractValue)
          {
            const ExtractValueInstruction &Extract = static_cast<const ExtractValueInstruction &>(InstructionValue);
            const StructType *AggregateType = nullptr;
            if (!Extract.Aggregate)
            {
              Diagnostics.add<DiagnosticKind::IrExtractNullAggregate>(FunctionValue.Name);
            }
            else
            {
              verifyOperand(ModuleValue, *Extract.Aggregate, AvailableValues, FunctionValue.Name, Diagnostics);
              if (Extract.Aggregate->type().kind() != TypeKind::Struct)
              {
                Diagnostics.add<DiagnosticKind::IrExtractAggregateMustBeStruct>();
              }
              else
              {
                AggregateType = static_cast<const StructType *>(&Extract.Aggregate->type());
              }
            }
            if (!isValidType(ModuleValue, Extract.ResultType) || Extract.ResultType->kind() == TypeKind::Void)
            {
              Diagnostics.add<DiagnosticKind::IrExtractInvalidResultType>(FunctionValue.Name);
            }
            if (AggregateType != nullptr)
            {
              if (Extract.FieldIndex >= AggregateType->fieldTypes().size())
              {
                Diagnostics.add<DiagnosticKind::IrExtractFieldIndexOutOfRange>(FunctionValue.Name, Extract.FieldIndex, AggregateType->fieldTypes().size());
              }
              else if (Extract.ResultType != AggregateType->fieldTypes()[Extract.FieldIndex])
              {
                Diagnostics.add<DiagnosticKind::IrExtractResultTypeMismatch>(FunctionValue.Name);
              }
            }
            if (verifySsaResult(Extract.Result, "extractvalue", FunctionValue.Name, ExpectedValueId, DefinedValues, Diagnostics) && isValidType(ModuleValue, Extract.ResultType))
            {
              AvailableValues.emplace(Extract.Result.value(), Extract.ResultType);
            }
            continue;
          }

          const ReturnInstruction &Return = static_cast<const ReturnInstruction &>(InstructionValue);
          if (FunctionValue.ResultType->kind() == TypeKind::Void)
          {
            if (Return.ReturnValue)
            {
              Diagnostics.add<DiagnosticKind::IrVoidFunctionReturnsValue>(FunctionValue.Name);
            }
            continue;
          }
          if (!Return.ReturnValue)
          {
            Diagnostics.add<DiagnosticKind::IrNonVoidFunctionMissingReturnValue>(FunctionValue.Name);
            continue;
          }
          verifyOperand(ModuleValue, *Return.ReturnValue, AvailableValues, FunctionValue.Name, Diagnostics);
          if (&Return.ReturnValue->type() != FunctionValue.ResultType)
          {
            Diagnostics.add<DiagnosticKind::IrReturnTypeMismatch>(FunctionValue.Name);
          }
        }
      }
    }
  } // namespace

  VerificationResult verify(IRContext &Context, const Module &ModuleValue)
  {
    return verify(Context, ModuleValue, DiagnosticClass::InternalCompilerError);
  }

  VerificationResult verify(IRContext &Context, const Module &ModuleValue, DiagnosticClass Class)
  {
    DiagnosticCollector Diagnostics(Class);
    std::unordered_set<std::string> GlobalNames;
    std::unordered_set<std::string> TypeNames;
    std::unordered_set<const StructType *> DeclaredStructTypes;

    for (const StructType *TypeValue : ModuleValue.StructTypes)
    {
      if (TypeValue == nullptr)
      {
        Diagnostics.add<DiagnosticKind::IrNullStructTypeDeclaration>();
        continue;
      }
      if (!DeclaredStructTypes.insert(TypeValue).second)
      {
        Diagnostics.add<DiagnosticKind::IrStructTypeDeclaredMoreThanOnce>(TypeValue->name());
      }
      if (!isValidName(TypeValue->name()))
      {
        Diagnostics.add<DiagnosticKind::IrInvalidStructTypeName>(TypeValue->name());
      }
      else if (!TypeNames.insert(TypeValue->name()).second)
      {
        Diagnostics.add<DiagnosticKind::IrDuplicateStructType>(TypeValue->name());
      }
      if (TypeValue->fieldTypes().empty())
      {
        Diagnostics.add<DiagnosticKind::IrEmptyStructType>(TypeValue->name());
      }
      for (std::size_t FieldIndex = 0; FieldIndex < TypeValue->fieldTypes().size(); ++FieldIndex)
      {
        const Type *FieldType = TypeValue->fieldTypes()[FieldIndex];
        if (!isValidType(ModuleValue, FieldType) || FieldType->kind() == TypeKind::Void)
        {
          Diagnostics.add<DiagnosticKind::IrInvalidStructFieldType>(TypeValue->name(), FieldIndex);
        }
        else if (FieldType->kind() == TypeKind::Struct && (FieldType == TypeValue || DeclaredStructTypes.find(static_cast<const StructType *>(FieldType)) == DeclaredStructTypes.end()))
        {
          Diagnostics.add<DiagnosticKind::IrStructFieldForwardOrSelfReference>(TypeValue->name(), FieldIndex);
        }
      }
    }

    for (const ByteConstant &Constant : ModuleValue.ByteConstants)
    {
      if (!isValidName(Constant.Name))
      {
        Diagnostics.add<DiagnosticKind::IrInvalidGlobalByteConstantName>(Constant.Name);
      }
      else if (!GlobalNames.insert(Constant.Name).second)
      {
        Diagnostics.add<DiagnosticKind::IrDuplicateGlobalSymbol>(Constant.Name);
      }
    }

    for (const Function &FunctionValue : ModuleValue.Functions)
    {
      if (!isValidName(FunctionValue.Name))
      {
        Diagnostics.add<DiagnosticKind::IrInvalidFunctionName>(FunctionValue.Name);
      }
      else if (!GlobalNames.insert(FunctionValue.Name).second)
      {
        Diagnostics.add<DiagnosticKind::IrDuplicateGlobalSymbol>(FunctionValue.Name);
      }
      verifyFunctionSignature(ModuleValue, FunctionValue, Diagnostics);
    }
    for (const Function &FunctionValue : ModuleValue.Functions)
    {
      verifyFunctionBody(ModuleValue, FunctionValue, Diagnostics);
    }
    Diagnostics.report(Context);
    return VerificationResult(Diagnostics.takeDiagnostics());
  }

  VerificationResult verify(const Module &ModuleValue)
  {
    return verify(ModuleValue.context(), ModuleValue);
  }
} // namespace ink::ir
