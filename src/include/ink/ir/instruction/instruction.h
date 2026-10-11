#ifndef INK_IR_INSTRUCTION_INSTRUCTION_H
#define INK_IR_INSTRUCTION_INSTRUCTION_H

#include "ink/ir/type/value.h"

namespace ink::ir
{
  // Common base for executable IR instructions; operation-specific data belongs to derived classes.
  class Instruction : public Value
  {
    public:
      ~Instruction() noexcept override = 0;

      static bool classof(const Value *Value) noexcept;

    protected:
      Instruction() noexcept;
  };
} // namespace ink::ir

#endif
