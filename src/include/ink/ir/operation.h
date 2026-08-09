#ifndef INK_IR_OPERATION_H
#define INK_IR_OPERATION_H

#include "ink/ir/ids.h"
#include "ink/ir/opcode.h"

#include <cstdint>
#include <variant>

namespace ink::ir
{
  enum class IrComparePredicate : std::uint8_t
  {
    Equal,
    NotEqual,
    SignedLess,
    SignedLessEqual,
    SignedGreater,
    SignedGreaterEqual,
    UnsignedLess,
    UnsignedLessEqual,
    UnsignedGreater,
    UnsignedGreaterEqual,
  };

  enum class IrTrapKind : std::uint8_t
  {
    User,
    Bounds,
    DivisionByZero,
    Overflow,
    Unreachable,
  };

  struct IrNoPayload
  {
  };

  struct IrConstantPayload
  {
    IrConstantId Constant;
  };

  struct IrComparePayload
  {
    IrComparePredicate Predicate = IrComparePredicate::Equal;
  };

  struct IrTypePayload
  {
    IrTypeId Type;
  };

  struct IrDirectCallPayload
  {
    IrFunctionId Callee;
  };

  struct IrTrapPayload
  {
    IrTrapKind Kind = IrTrapKind::User;
  };

  using IrOperationPayload = std::variant<IrNoPayload, IrConstantPayload, IrComparePayload, IrTypePayload, IrDirectCallPayload, IrTrapPayload>;

  struct IrSuccessor
  {
    IrBlockId Block;
    IrTableRange Arguments;
  };

  struct IrOperation
  {
    IrOpcode Opcode = IrOpcode::Unknown;
    IrFunctionId OwnerFunction;
    IrBlockId OwnerBlock;
    IrOriginId Origin;
    IrTableRange Operands;
    IrTableRange Results;
    IrTableRange Successors;
    IrOperationPayload Payload = IrNoPayload{};
  };
} // namespace ink::ir

#endif
