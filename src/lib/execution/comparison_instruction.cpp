#include "function_executor.h"

#include <cstdint>
#include <optional>

namespace ink::execution
{
  namespace
  {
    template <typename ValueType>
    std::optional<bool> compareOrdered(ir::ComparePredicate Predicate, ValueType Left, ValueType Right) noexcept
    {
      switch (Predicate)
      {
      case ir::ComparePredicate::Equal:
        return Left == Right;
      case ir::ComparePredicate::NotEqual:
        return Left != Right;
      case ir::ComparePredicate::LessThan:
        return Left < Right;
      case ir::ComparePredicate::LessEqual:
        return Left <= Right;
      case ir::ComparePredicate::GreaterThan:
        return Left > Right;
      case ir::ComparePredicate::GreaterEqual:
        return Left >= Right;
      case ir::ComparePredicate::Count:
        return std::nullopt;
      }
      return std::nullopt;
    }

    std::optional<bool> compareBool(ir::ComparePredicate Predicate, bool Left, bool Right) noexcept
    {
      if (Predicate == ir::ComparePredicate::Equal)
      {
        return Left == Right;
      }
      if (Predicate == ir::ComparePredicate::NotEqual)
      {
        return Left != Right;
      }
      return std::nullopt;
    }

    std::int64_t signedI32Value(std::uint64_t Payload) noexcept
    {
      const std::uint32_t Bits = static_cast<std::uint32_t>(Payload);
      return (Bits & 0x80000000U) == 0 ? static_cast<std::int64_t>(Bits) : static_cast<std::int64_t>(Bits) - 0x100000000LL;
    }
  } // namespace

  FunctionExecutor::InstructionFlow FunctionExecutor::executeCompareInstruction(const ir::CompareInstruction &Compare, FunctionExecutionState &State)
  {
    RuntimeValueRef Left = evaluateValue(*Compare.Left, State.Module, State.Frame, State.FunctionValue.Name);
    RuntimeValueRef Right = evaluateValue(*Compare.Right, State.Module, State.Frame, State.FunctionValue.Name);
    if (Left == nullptr || Right == nullptr || Compare.ResultType == nullptr || Compare.ResultType->kind() != ir::TypeKind::Bool)
    {
      return InstructionFlow::Failed;
    }

    std::optional<bool> Comparison;
    switch (Compare.Left->type().kind())
    {
    case ir::TypeKind::Bool:
    {
      const std::optional<std::uint64_t> LeftInteger = Left->integer();
      const std::optional<std::uint64_t> RightInteger = Right->integer();
      if (LeftInteger.has_value() && RightInteger.has_value())
      {
        Comparison = compareBool(Compare.Predicate, *LeftInteger != 0, *RightInteger != 0);
      }
      break;
    }
    case ir::TypeKind::Byte:
    {
      const std::optional<std::uint64_t> LeftInteger = Left->integer();
      const std::optional<std::uint64_t> RightInteger = Right->integer();
      if (LeftInteger.has_value() && RightInteger.has_value())
      {
        Comparison = compareOrdered(Compare.Predicate, static_cast<std::uint8_t>(*LeftInteger), static_cast<std::uint8_t>(*RightInteger));
      }
      break;
    }
    case ir::TypeKind::I32:
    {
      const std::optional<std::uint64_t> LeftInteger = Left->integer();
      const std::optional<std::uint64_t> RightInteger = Right->integer();
      if (LeftInteger.has_value() && RightInteger.has_value())
      {
        Comparison = compareOrdered(Compare.Predicate, signedI32Value(*LeftInteger), signedI32Value(*RightInteger));
      }
      break;
    }
    case ir::TypeKind::PointerSize:
    {
      const std::optional<std::uint64_t> LeftInteger = Left->integer();
      const std::optional<std::uint64_t> RightInteger = Right->integer();
      if (LeftInteger.has_value() && RightInteger.has_value())
      {
        Comparison = compareOrdered(Compare.Predicate, *LeftInteger, *RightInteger);
      }
      break;
    }
    case ir::TypeKind::BytePointer:
    case ir::TypeKind::ConstBytePointer:
    {
      const std::optional<bool> Equal = runtimePointersEqual(*Left, *Right);
      if (Equal.has_value())
      {
        Comparison = compareBool(Compare.Predicate, *Equal, true);
      }
      break;
    }
    case ir::TypeKind::Void:
    case ir::TypeKind::F16:
    case ir::TypeKind::F32:
    case ir::TypeKind::F64:
    case ir::TypeKind::ByteSlice:
    case ir::TypeKind::ConstByteSlice:
    case ir::TypeKind::Struct:
    case ir::TypeKind::Count:
      break;
    }
    if (!Comparison.has_value())
    {
      addFailure<core::DiagnosticKind::InvalidComparisonDuringExecution>(State.FunctionValue.Name);
      return InstructionFlow::Failed;
    }

    RuntimeValueRef Result = Values.integerValue(*Compare.ResultType, *Comparison ? 1 : 0);
    if (Result == nullptr)
    {
      addFailure<core::DiagnosticKind::InvalidComparisonDuringExecution>(State.FunctionValue.Name);
      return InstructionFlow::Failed;
    }
    if (!State.Frame.define(Compare.Result, Result))
    {
      addFailure<core::DiagnosticKind::SsaValueRedefinedDuringExecution>("icmp", State.FunctionValue.Name, Compare.Result.value());
      return InstructionFlow::Failed;
    }
    return InstructionFlow::Continue;
  }
} // namespace ink::execution
