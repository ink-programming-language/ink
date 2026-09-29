#ifndef INK_IR_C_STRING_INSTRUCTION_H
#define INK_IR_C_STRING_INSTRUCTION_H

#include "ink/ir/constant/string_constant.h"
#include "ink/ir/type/pointer_type.h"

namespace ink::ir
{
  // Each execution allocates a distinct writable copy of Source plus its final NUL.
  // Storage lasts until the containing function returns; it never aliases the constant.
  // This is target-program storage, not the host pointer returned by tryGetCString().
  class CStringInstruction final : public Value
  {
    public:
      const StringConstant &source() const noexcept
      {
        return Source;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::CStringInstruction;
      }

    private:
      CStringInstruction(const PointerType &ResultType, const StringConstant &Source) noexcept
          : Value(ResultType.context(), ValueKind::CStringInstruction, ResultType),
            Source(Source)
      {
      }

      const StringConstant &Source;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
