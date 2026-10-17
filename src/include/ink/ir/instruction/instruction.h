#ifndef INK_IR_INSTRUCTION_INSTRUCTION_H
#define INK_IR_INSTRUCTION_INSTRUCTION_H

#include "ink/ir/type/value.h"

#include <utility>

namespace ink::ir
{
  class BasicBlock;
  class Type;

  // Common base for executable IR instructions; operation-specific data belongs to derived classes.
  // Types, operands and branch destinations are borrowed and must outlive their users.
  // Construction records resolved IR; operand type compatibility and control flow are validated separately.
  class Instruction : public Value
  {
    public:
      ~Instruction() noexcept override = 0;

      // Concrete instructions have a result type, including void for branches and returns.
      // A generic instruction subclass may leave its result type unspecified.
      const Type *type() const noexcept override;
      // Detached instructions have no parent; appending transfers ownership to that block.
      auto *parentBlock(this auto &&Self) noexcept
      {
        return Self.Parent ? &std::forward_like<decltype(Self) &>(*Self.Parent) : nullptr;
      }

      bool isTerminator() const noexcept;
      static bool classof(const Value *Value) noexcept;

    protected:
      explicit Instruction(ValueKind Kind = ValueKind::Instruction, const Type *ResultType = nullptr) noexcept;

    private:
      friend class BasicBlock;

      const Type *ResultType = nullptr;
      BasicBlock *Parent = nullptr;
  };
} // namespace ink::ir

#endif
