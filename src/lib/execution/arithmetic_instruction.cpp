#include "function_executor.h"

#include <cstddef>
#include <cstdint>
#include <optional>

namespace ink::execution
{
  namespace
  {
    std::uint64_t signedI32Payload(std::uint32_t Bits) noexcept
    {
      return (Bits & 0x80000000U) == 0 ? static_cast<std::uint64_t>(Bits) : 0xFFFFFFFF00000000ULL | static_cast<std::uint64_t>(Bits);
    }
  } // namespace

  FunctionExecutor::InstructionFlow FunctionExecutor::executeAddInstruction(const ir::AddInstruction &Add, FunctionExecutionState &State)
  {
    RuntimeValueRef Left = evaluateValue(*Add.Left, State.Module, State.Frame, State.FunctionValue.Name);
    RuntimeValueRef Right = evaluateValue(*Add.Right, State.Module, State.Frame, State.FunctionValue.Name);
    if (Left == nullptr || Right == nullptr || Add.ResultType == nullptr)
    {
      return InstructionFlow::Failed;
    }
    const std::optional<std::uint64_t> LeftInteger = Left->integer();
    const std::optional<std::uint64_t> RightInteger = Right->integer();
    if (!LeftInteger.has_value() || !RightInteger.has_value())
    {
      addFailure<core::DiagnosticKind::InvalidIntegerOperationDuringExecution>("add", State.FunctionValue.Name);
      return InstructionFlow::Failed;
    }

    std::uint64_t Payload = 0;
    switch (Add.ResultType->kind())
    {
    case ir::TypeKind::Byte:
      Payload = static_cast<std::uint8_t>(static_cast<std::uint8_t>(*LeftInteger) + static_cast<std::uint8_t>(*RightInteger));
      break;
    case ir::TypeKind::I32:
      Payload = signedI32Payload(static_cast<std::uint32_t>(*LeftInteger) + static_cast<std::uint32_t>(*RightInteger));
      break;
    case ir::TypeKind::PointerSize:
      if (Context.compilationContext().targetContext().pointerWidth() == core::PointerWidth::Bits32)
      {
        Payload = static_cast<std::uint32_t>(*LeftInteger) + static_cast<std::uint32_t>(*RightInteger);
      }
      else
      {
        Payload = *LeftInteger + *RightInteger;
      }
      break;
    case ir::TypeKind::Void:
    case ir::TypeKind::Bool:
    case ir::TypeKind::F16:
    case ir::TypeKind::F32:
    case ir::TypeKind::F64:
    case ir::TypeKind::BytePointer:
    case ir::TypeKind::ConstBytePointer:
    case ir::TypeKind::ByteSlice:
    case ir::TypeKind::ConstByteSlice:
    case ir::TypeKind::Struct:
    case ir::TypeKind::Count:
      addFailure<core::DiagnosticKind::InvalidIntegerOperationDuringExecution>("add", State.FunctionValue.Name);
      return InstructionFlow::Failed;
    }

    RuntimeValueRef Result = Values.integerValue(*Add.ResultType, Payload);
    if (Result == nullptr)
    {
      addFailure<core::DiagnosticKind::InvalidIntegerOperationDuringExecution>("add", State.FunctionValue.Name);
      return InstructionFlow::Failed;
    }
    if (!State.Frame.define(Add.Result, Result))
    {
      addFailure<core::DiagnosticKind::SsaValueRedefinedDuringExecution>("add", State.FunctionValue.Name, Add.Result.value());
      return InstructionFlow::Failed;
    }
    return InstructionFlow::Continue;
  }
} // namespace ink::execution
