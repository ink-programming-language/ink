#include "ink/ir/verifier.h"

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
  using core::DiagnosticArgumentName;
  using core::DiagnosticBuilder;
  using core::DiagnosticKind;

  namespace
  {
    bool isValidType(TypeKind Kind)
    {
      switch (Kind)
      {
#define INK_IR_TYPE(Name, Spelling) \
  case TypeKind::Name:              \
    return true;
#include "ink/ir/ir.def"
      }
      return false;
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

    void addError(std::vector<Diagnostic> &Diagnostics, std::string Message)
    {
      Diagnostics.push_back(DiagnosticBuilder(DiagnosticKind::InvalidIrModule, {}).argument(DiagnosticArgumentName::Detail, std::move(Message)).build());
    }

    bool verifyIntegerConstant(const IntegerConstant &Constant, std::string &Message)
    {
      const std::int64_t Value = Constant.value();
      switch (Constant.type())
      {
      case TypeKind::Bool:
        if (Value != 0 && Value != 1)
        {
          Message = "bool integer constant must be 0 or 1";
          return false;
        }
        return true;
      case TypeKind::Byte:
        if (Value < 0 || Value > 255)
        {
          Message = "byte integer constant is outside the range 0...255";
          return false;
        }
        return true;
      case TypeKind::I32:
        if (Value < std::numeric_limits<std::int32_t>::min() || Value > std::numeric_limits<std::int32_t>::max())
        {
          Message = "i32 integer constant is outside the signed 32-bit range";
          return false;
        }
        return true;
      case TypeKind::PointerSize:
        if (Value < 0)
        {
          Message = "ptrsize integer constant cannot be negative";
          return false;
        }
        return true;
      case TypeKind::Void:
      case TypeKind::ConstBytePointer:
        Message = std::string("integer constant cannot have type '") + typeKindName(Constant.type()) + "'";
        return false;
      }
      Message = "integer constant has an unknown type";
      return false;
    }

    bool verifyOperand(const Module &ModuleValue, const Value &OperandValue, const std::unordered_map<std::size_t, TypeKind> &AvailableValues, std::string &Message)
    {
      if (!isValidType(OperandValue.type()) || OperandValue.type() == TypeKind::Void)
      {
        Message = "operand must have a valid non-void type";
        return false;
      }

      if (OperandValue.kind() == ValueKind::IntegerConstant)
      {
        return verifyIntegerConstant(static_cast<const IntegerConstant &>(OperandValue), Message);
      }

      if (OperandValue.kind() == ValueKind::ValueOperand)
      {
        const ValueId Id = static_cast<const ValueOperand &>(OperandValue).id();
        const auto Definition = AvailableValues.find(Id.value());
        if (!Id.valid() || Definition == AvailableValues.end())
        {
          Message = "operand references an unavailable SSA value %" + std::to_string(Id.value());
          return false;
        }
        if (Definition->second != OperandValue.type())
        {
          Message = "operand type does not match the type of SSA value %" + std::to_string(Id.value());
          return false;
        }
        return true;
      }

      if (OperandValue.kind() != ValueKind::GlobalAddressOperand)
      {
        Message = "operand has an unknown value kind";
        return false;
      }

      const GlobalAddressOperand &Address = static_cast<const GlobalAddressOperand &>(OperandValue);
      if (OperandValue.type() != TypeKind::ConstBytePointer)
      {
        Message = "global byte address must have type 'const byte*'";
        return false;
      }
      if (!Address.global().valid() || Address.global().value() >= ModuleValue.ByteConstants.size())
      {
        Message = "operand references an invalid global byte constant";
        return false;
      }
      if (Address.byteOffset() > ModuleValue.ByteConstants[Address.global().value()].Data.size())
      {
        Message = "global byte address offset is outside the constant";
        return false;
      }
      return true;
    }

    void verifyFunctionSignature(const Function &FunctionValue, std::vector<Diagnostic> &Diagnostics)
    {
      if (!isValidType(FunctionValue.ResultType))
      {
        addError(Diagnostics, "function @" + FunctionValue.Name + " has an unknown result type");
      }
      for (const TypeKind ParameterType : FunctionValue.ParameterTypes)
      {
        if (!isValidType(ParameterType) || ParameterType == TypeKind::Void)
        {
          addError(Diagnostics, "function @" + FunctionValue.Name + " has a void or unknown parameter type");
        }
      }
      if (FunctionValue.Kind != FunctionKind::Definition && FunctionValue.Kind != FunctionKind::External)
      {
        addError(Diagnostics, "function @" + FunctionValue.Name + " has an unknown function kind");
        return;
      }
      if (FunctionValue.Kind == FunctionKind::External)
      {
        if (FunctionValue.Convention != CallingConvention::C)
        {
          addError(Diagnostics, "external function @" + FunctionValue.Name + " must use the C calling convention");
        }
        if (!FunctionValue.Blocks.empty())
        {
          addError(Diagnostics, "external function @" + FunctionValue.Name + " cannot contain basic blocks");
        }
      }
      else
      {
        if (FunctionValue.Convention != CallingConvention::Ink)
        {
          addError(Diagnostics, "defined function @" + FunctionValue.Name + " must use the Ink calling convention");
        }
        if (FunctionValue.HasSideEffects)
        {
          addError(Diagnostics, "defined function @" + FunctionValue.Name + " cannot carry an external side-effect declaration");
        }
        if (FunctionValue.Blocks.empty())
        {
          addError(Diagnostics, "defined function @" + FunctionValue.Name + " must contain at least one basic block");
        }
      }
    }

    void verifyFunctionBody(const Module &ModuleValue, const Function &FunctionValue, std::vector<Diagnostic> &Diagnostics)
    {
      if (FunctionValue.Kind != FunctionKind::Definition)
      {
        return;
      }

      std::unordered_set<std::string> BlockNames;
      std::unordered_set<std::size_t> DefinedValues;
      for (std::size_t ParameterIndex = 0; ParameterIndex < FunctionValue.ParameterTypes.size(); ++ParameterIndex)
      {
        DefinedValues.insert(ParameterIndex);
      }

      for (const BasicBlock &Block : FunctionValue.Blocks)
      {
        if (!isValidName(Block.Name))
        {
          addError(Diagnostics, "function @" + FunctionValue.Name + " has an invalid basic block name '" + Block.Name + "'");
        }
        else if (!BlockNames.insert(Block.Name).second)
        {
          addError(Diagnostics, "function @" + FunctionValue.Name + " has duplicate basic block name '" + Block.Name + "'");
        }
        if (Block.Instructions.empty())
        {
          addError(Diagnostics, "basic block " + Block.Name + " in function @" + FunctionValue.Name + " is empty");
          continue;
        }

        std::unordered_map<std::size_t, TypeKind> AvailableValues;
        for (std::size_t ParameterIndex = 0; ParameterIndex < FunctionValue.ParameterTypes.size(); ++ParameterIndex)
        {
          AvailableValues.emplace(ParameterIndex, FunctionValue.ParameterTypes[ParameterIndex]);
        }

        for (std::size_t InstructionIndex = 0; InstructionIndex < Block.Instructions.size(); ++InstructionIndex)
        {
          const std::unique_ptr<Instruction> &InstructionPointer = Block.Instructions[InstructionIndex];
          if (!InstructionPointer)
          {
            addError(Diagnostics, "basic block " + Block.Name + " in function @" + FunctionValue.Name + " contains a null instruction");
            continue;
          }
          const Instruction &InstructionValue = *InstructionPointer;
          const bool IsLast = InstructionIndex + 1 == Block.Instructions.size();
          const InstructionKind Kind = InstructionValue.kind();
          if (Kind != InstructionKind::Call && Kind != InstructionKind::Return)
          {
            addError(Diagnostics, "basic block " + Block.Name + " in function @" + FunctionValue.Name + " contains an instruction with an unknown kind");
            continue;
          }
          if (isTerminator(Kind) != IsLast)
          {
            addError(Diagnostics, "basic block " + Block.Name + " in function @" + FunctionValue.Name + (IsLast ? " does not end with a terminator" : " contains a terminator before its final instruction"));
          }

          if (Kind == InstructionKind::Call)
          {
            const CallInstruction *Call = static_cast<const CallInstruction *>(&InstructionValue);
            if (!Call->Callee.valid() || Call->Callee.value() >= ModuleValue.Functions.size())
            {
              addError(Diagnostics, "call in function @" + FunctionValue.Name + " references an invalid callee");
              continue;
            }
            const Function &Callee = ModuleValue.Functions[Call->Callee.value()];
            if (Call->ResultType != Callee.ResultType)
            {
              addError(Diagnostics, "call to @" + Callee.Name + " has a result type that does not match the callee signature");
            }
            if ((Call->ResultType == TypeKind::Void) == Call->Result.has_value())
            {
              addError(Diagnostics, "call to @" + Callee.Name + (Call->ResultType == TypeKind::Void ? " cannot define an SSA result" : " must define an SSA result"));
            }
            if (Call->Arguments.size() != Callee.ParameterTypes.size())
            {
              addError(Diagnostics, "call to @" + Callee.Name + " has the wrong number of arguments");
            }
            const std::size_t CheckedArgumentCount = Call->Arguments.size() < Callee.ParameterTypes.size() ? Call->Arguments.size() : Callee.ParameterTypes.size();
            for (std::size_t ArgumentIndex = 0; ArgumentIndex < Call->Arguments.size(); ++ArgumentIndex)
            {
              if (!Call->Arguments[ArgumentIndex])
              {
                addError(Diagnostics, "argument " + std::to_string(ArgumentIndex) + " of call to @" + Callee.Name + " is null");
                continue;
              }
              std::string Message;
              if (!verifyOperand(ModuleValue, *Call->Arguments[ArgumentIndex], AvailableValues, Message))
              {
                addError(Diagnostics, "argument " + std::to_string(ArgumentIndex) + " of call to @" + Callee.Name + " is invalid: " + Message);
              }
              if (ArgumentIndex < CheckedArgumentCount && Call->Arguments[ArgumentIndex]->type() != Callee.ParameterTypes[ArgumentIndex])
              {
                addError(Diagnostics, "argument " + std::to_string(ArgumentIndex) + " of call to @" + Callee.Name + " does not match the parameter type");
              }
            }
            if (Call->Result.has_value())
            {
              if (!Call->Result->valid() || !DefinedValues.insert(Call->Result->value()).second)
              {
                addError(Diagnostics, "call to @" + Callee.Name + " defines an invalid or duplicate SSA value");
              }
              else
              {
                AvailableValues.emplace(Call->Result->value(), Call->ResultType);
              }
            }
            continue;
          }

          const ReturnInstruction &Return = static_cast<const ReturnInstruction &>(InstructionValue);
          if (FunctionValue.ResultType == TypeKind::Void)
          {
            if (Return.ReturnValue)
            {
              addError(Diagnostics, "void function @" + FunctionValue.Name + " cannot return a value");
            }
            continue;
          }
          if (!Return.ReturnValue)
          {
            addError(Diagnostics, "non-void function @" + FunctionValue.Name + " must return a value");
            continue;
          }
          std::string Message;
          if (!verifyOperand(ModuleValue, *Return.ReturnValue, AvailableValues, Message))
          {
            addError(Diagnostics, "return in function @" + FunctionValue.Name + " is invalid: " + Message);
          }
          if (Return.ReturnValue->type() != FunctionValue.ResultType)
          {
            addError(Diagnostics, "return type in function @" + FunctionValue.Name + " does not match the function result type");
          }
        }
      }
    }
  } // namespace

  VerificationResult verify(IRContext &Context, const Module &ModuleValue)
  {
    std::vector<Diagnostic> Diagnostics;
    std::unordered_set<std::string> GlobalNames;

    for (const ByteConstant &Constant : ModuleValue.ByteConstants)
    {
      if (!isValidName(Constant.Name))
      {
        addError(Diagnostics, "invalid global byte constant name '" + Constant.Name + "'");
      }
      else if (!GlobalNames.insert(Constant.Name).second)
      {
        addError(Diagnostics, "duplicate global symbol @" + Constant.Name);
      }
    }

    for (const Function &FunctionValue : ModuleValue.Functions)
    {
      if (!isValidName(FunctionValue.Name))
      {
        addError(Diagnostics, "invalid function name '" + FunctionValue.Name + "'");
      }
      else if (!GlobalNames.insert(FunctionValue.Name).second)
      {
        addError(Diagnostics, "duplicate global symbol @" + FunctionValue.Name);
      }
      verifyFunctionSignature(FunctionValue, Diagnostics);
    }
    for (const Function &FunctionValue : ModuleValue.Functions)
    {
      verifyFunctionBody(ModuleValue, FunctionValue, Diagnostics);
    }
    for (const Diagnostic &DiagnosticEntry : Diagnostics)
    {
      Context.diagnosticEngine().report(DiagnosticEntry);
    }
    return VerificationResult(std::move(Diagnostics));
  }

  VerificationResult verify(const Module &ModuleValue)
  {
    core::CompilationContext Compilation;
    IRContext Context(Compilation);
    return verify(Context, ModuleValue);
  }
} // namespace ink::ir
