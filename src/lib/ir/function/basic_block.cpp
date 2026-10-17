#include "ink/ir/function/basic_block.h"

#include "ink/ir/instruction/instruction.h"

#include <utility>

namespace ink::ir
{
  BasicBlock::BasicBlock(core::Name Name) noexcept
      : Value(ValueKind::BasicBlock),
        Name(Name)
  {
  }

  BasicBlock::~BasicBlock() noexcept = default;

  core::Name BasicBlock::name() const noexcept
  {
    return Name;
  }

  bool BasicBlock::hasName() const noexcept
  {
    return Name.valid();
  }

  void BasicBlock::setName(core::Name NewName) noexcept
  {
    Name = NewName;
  }

  const std::vector<std::unique_ptr<Instruction>> &BasicBlock::instructions() const noexcept
  {
    return Instructions;
  }

  std::size_t BasicBlock::instructionCount() const noexcept
  {
    return Instructions.size();
  }

  bool BasicBlock::hasTerminator() const noexcept
  {
    return HasTerminator;
  }

  Instruction *BasicBlock::appendInstruction(std::unique_ptr<Instruction> InstructionValue)
  {
    if (!InstructionValue)
    {
      return nullptr;
    }
    Instruction *Result = InstructionValue.get();
    Instructions.push_back(std::move(InstructionValue));
    Result->Parent = this;
    HasTerminator = HasTerminator || Result->isTerminator();
    return Result;
  }

  bool BasicBlock::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::BasicBlock;
  }
} // namespace ink::ir
