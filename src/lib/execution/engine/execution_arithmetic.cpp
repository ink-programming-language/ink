#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/value/execution_integer.h"

#include "ink/ir/constant/bool_constant.h"
#include "ink/ir/constant/integer_constant.h"
#include "ink/ir/context.h"
#include "ink/tokenizer/token.h"

#include <cstdint>

namespace ink::execution
{
  namespace
  {
    ExecutionResult integerResult(ir::IRContext &Context, const ir::IntegerType &Type, ExecutionStatus Status, const ExecutionInteger &Bits)
    {
      if (Status != ExecutionStatus::Success)
      {
        return {Status};
      }
      const ir::IntegerConstant *Result = Context.constantPool().getIntegerConstant(Type, Bits.bits());
      return {Result ? ExecutionStatus::Success : ExecutionStatus::TypeMismatch, Result};
    }

    ExecutionResult boolResult(ir::IRContext &Context, bool Value)
    {
      return {ExecutionStatus::Success, &Context.constantPool().getBoolConstant(Value)};
    }
  } // namespace

  ExecutionResult ExecutionEngine::evaluateUnary(tokenizer::TokenKind Op, const ir::Constant &Operand)
  {
    const ExecutionStatus StepStatus = consumeStep();
    if (StepStatus != ExecutionStatus::Success)
    {
      return {StepStatus};
    }
    if (&Operand.context() != &context())
    {
      return {ExecutionStatus::ForeignContext};
    }
    if (ir::BoolConstant::classof(&Operand))
    {
      if (Op == tokenizer::TokenKind::Bang)
      {
        return boolResult(context(), !static_cast<const ir::BoolConstant &>(Operand).value());
      }
      return {ExecutionStatus::UnsupportedOperation};
    }
    if (!ir::IntegerConstant::classof(&Operand))
    {
      return {ExecutionStatus::UnsupportedOperation};
    }
    const auto &Type = static_cast<const ir::IntegerType &>(Operand.type());
    const ExecutionInteger Bits(static_cast<const ir::IntegerConstant &>(Operand).value());
    ExecutionInteger Result(Type.bitWidth());
    ExecutionStatus Status;
    switch (Op)
    {
    case tokenizer::TokenKind::Plus:
      return {ExecutionStatus::Success, &Operand};
    case tokenizer::TokenKind::Minus:
      Status = Bits.negate(Result);
      break;
    case tokenizer::TokenKind::Tilde:
      Status = Bits.invert(Result);
      break;
    default:
      return {ExecutionStatus::UnsupportedOperation};
    }
    return integerResult(context(), Type, Status, Result);
  }

  ExecutionResult ExecutionEngine::evaluateBinary(tokenizer::TokenKind Op, const ir::Constant &Left, const ir::Constant &Right)
  {
    const ExecutionStatus StepStatus = consumeStep();
    if (StepStatus != ExecutionStatus::Success)
    {
      return {StepStatus};
    }
    if (&Left.context() != &context() || &Right.context() != &context())
    {
      return {ExecutionStatus::ForeignContext};
    }
    if (&Left.type() != &Right.type())
    {
      return {ExecutionStatus::TypeMismatch};
    }
    if (ir::BoolConstant::classof(&Left) && ir::BoolConstant::classof(&Right))
    {
      const bool LeftValue = static_cast<const ir::BoolConstant &>(Left).value();
      const bool RightValue = static_cast<const ir::BoolConstant &>(Right).value();
      switch (Op)
      {
      case tokenizer::TokenKind::AmpAmp:
        return boolResult(context(), LeftValue && RightValue);
      case tokenizer::TokenKind::PipePipe:
        return boolResult(context(), LeftValue || RightValue);
      case tokenizer::TokenKind::EqualEqual:
        return boolResult(context(), LeftValue == RightValue);
      case tokenizer::TokenKind::BangEqual:
        return boolResult(context(), LeftValue != RightValue);
      default:
        return {ExecutionStatus::UnsupportedOperation};
      }
    }
    if (!ir::IntegerConstant::classof(&Left) || !ir::IntegerConstant::classof(&Right))
    {
      return {ExecutionStatus::UnsupportedOperation};
    }
    const auto &Type = static_cast<const ir::IntegerType &>(Left.type());
    const ExecutionInteger LeftBits(static_cast<const ir::IntegerConstant &>(Left).value());
    const ExecutionInteger RightBits(static_cast<const ir::IntegerConstant &>(Right).value());
    ExecutionInteger Result(Type.bitWidth());
    ExecutionStatus Status;
    switch (Op)
    {
    case tokenizer::TokenKind::Plus:
      Status = LeftBits.add(RightBits, Result);
      break;
    case tokenizer::TokenKind::Minus:
      Status = LeftBits.subtract(RightBits, Result);
      break;
    case tokenizer::TokenKind::Star:
      Status = LeftBits.multiply(RightBits, Result);
      break;
    case tokenizer::TokenKind::Slash:
      Status = LeftBits.divide(RightBits, Type.isSigned(), Result);
      break;
    case tokenizer::TokenKind::Percent:
      Status = LeftBits.remainder(RightBits, Type.isSigned(), Result);
      break;
    case tokenizer::TokenKind::Amp:
      Status = LeftBits.bitwiseAnd(RightBits, Result);
      break;
    case tokenizer::TokenKind::Pipe:
      Status = LeftBits.bitwiseOr(RightBits, Result);
      break;
    case tokenizer::TokenKind::Caret:
      Status = LeftBits.bitwiseXor(RightBits, Result);
      break;
    case tokenizer::TokenKind::ShiftLeft:
    case tokenizer::TokenKind::ShiftRight:
    {
      std::uint32_t Amount = 0;
      Status = RightBits.shiftAmount(Type.bitWidth(), Type.isSigned(), Amount);
      if (Status != ExecutionStatus::Success)
      {
        return {Status};
      }
      if (Op == tokenizer::TokenKind::ShiftLeft)
      {
        Status = LeftBits.shiftLeft(Amount, Result);
      }
      else
      {
        Status = LeftBits.shiftRight(Amount, Type.isSigned(), Result);
      }
      break;
    }
    case tokenizer::TokenKind::Less:
    case tokenizer::TokenKind::LessEqual:
    case tokenizer::TokenKind::Greater:
    case tokenizer::TokenKind::GreaterEqual:
    case tokenizer::TokenKind::EqualEqual:
    case tokenizer::TokenKind::BangEqual:
    {
      int Comparison = 0;
      Status = LeftBits.compare(RightBits, Type.isSigned(), Comparison);
      if (Status != ExecutionStatus::Success)
      {
        return {Status};
      }
      switch (Op)
      {
      case tokenizer::TokenKind::Less:
        return boolResult(context(), Comparison < 0);
      case tokenizer::TokenKind::LessEqual:
        return boolResult(context(), Comparison <= 0);
      case tokenizer::TokenKind::Greater:
        return boolResult(context(), Comparison > 0);
      case tokenizer::TokenKind::GreaterEqual:
        return boolResult(context(), Comparison >= 0);
      case tokenizer::TokenKind::EqualEqual:
        return boolResult(context(), Comparison == 0);
      default:
        return boolResult(context(), Comparison != 0);
      }
    }
    default:
      return {ExecutionStatus::UnsupportedOperation};
    }
    return integerResult(context(), Type, Status, Result);
  }
} // namespace ink::execution
