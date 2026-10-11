#ifndef INK_IR_INSTRUCTION_BASIC_BLOCK_H
#define INK_IR_INSTRUCTION_BASIC_BLOCK_H

#include "ink/core/name.h"
#include "ink/ir/type/value.h"

namespace ink::ir
{
  // Identity of an IR control-flow block; function ownership and instruction storage are not modeled yet.
  class BasicBlock final : public Value
  {
    public:
      explicit BasicBlock(core::Name Name = core::NAME_None) noexcept;

      core::Name name() const noexcept;
      bool hasName() const noexcept;
      void setName(core::Name NewName) noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      core::Name Name = core::NAME_None;
  };
} // namespace ink::ir

#endif
