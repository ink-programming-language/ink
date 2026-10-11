#include "ink/ir/instruction/basic_block.h"

namespace ink::ir
{
  BasicBlock::BasicBlock(core::Name Name) noexcept
      : Value(ValueKind::BasicBlock),
        Name(Name)
  {
  }

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

  bool BasicBlock::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::BasicBlock;
  }
} // namespace ink::ir
