#ifndef INK_IR_ARITHMETIC_H
#define INK_IR_ARITHMETIC_H

#include "ink/ir/instruction/instruction.h"

#include <memory>

namespace ink::ir
{
  class AddInstruction final : public Instruction
  {
    public:
      explicit AddInstruction(const Type &ResultType) noexcept
          : Instruction(InstructionKind::Add),
            ResultType(&ResultType)
      {
      }

      ValueId Result;
      const Type *ResultType;
      std::unique_ptr<Value> Left;
      std::unique_ptr<Value> Right;
  };
} // namespace ink::ir

#endif
