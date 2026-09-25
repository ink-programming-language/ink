#include "ink/semantic/model/instruction/return_instruction.h"

#include "ink/semantic/context.h"

namespace ink::semantic
{
  ReturnInstruction::ReturnInstruction(const SemanticContext &Context, const Value *ReturnedValue) noexcept
      : Value(Context, ValueKind::ReturnInstruction, Context.getVoidType()),
        ReturnedValue(ReturnedValue)
  {
  }

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
} // namespace ink::semantic
