#ifndef INK_IR_VALUE_H
#define INK_IR_VALUE_H

#include "ink/ir/coredefines.h"

namespace ink::ir
{
  class IRContext;
  class Type;

  // IR values keep their addresses while alive, including across ownership transfers.
  // Context owns shared types/constants and must outlive every detached or attached value that refers to it.
  class Value
  {
    public:
      virtual ~Value();
      Value(const Value &) = delete;
      Value &operator=(const Value &) = delete;
      Value(Value &&) = delete;
      Value &operator=(Value &&) = delete;

      ValueKind kind() const noexcept
      {
        return Kind;
      }

      bool isTerminator() const noexcept
      {
        return Kind == ValueKind::ReturnInstruction || Kind == ValueKind::BranchInstruction || Kind == ValueKind::ConditionalBranchInstruction;
      }

      const IRContext &context() const noexcept
      {
        return Context;
      }

      // Non-owning parent link. A structural parent owns this value; operands and types remain borrowed.
      Value *outer() noexcept
      {
        return Outer;
      }

      const Value *outer() const noexcept
      {
        return Outer;
      }

      const Type &type() const noexcept
      {
        return ValueType;
      }

    protected:
      Value(const IRContext &Context, ValueKind Kind, const Type &ValueType) noexcept
          : Context(Context),
            Kind(Kind),
            ValueType(ValueType)
      {
      }

    private:
      const IRContext &Context;
      ValueKind Kind;
      const Type &ValueType;
      Value *Outer = nullptr;

      friend class IRBuilder;
  };
} // namespace ink::ir

#endif
