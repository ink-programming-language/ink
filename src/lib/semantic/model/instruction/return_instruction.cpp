#include "ink/semantic/model/instruction/return_instruction.h"

#include "ink/semantic/model/context.h"

namespace ink::semantic
{
  const Function *ReturnInstruction::function() const noexcept
  {
    const Value *Block = outer();
    if (!BasicBlock::classof(Block))
    {
      return nullptr;
    }
    const Value *Parent = Block->outer();
    return Function::classof(Parent) ? static_cast<const Function *>(Parent) : nullptr;
  }

  const Type &ReturnInstruction::type() const noexcept
  {
    return context().getVoidType();
  }
} // namespace ink::semantic
