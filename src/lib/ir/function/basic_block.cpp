#include "ink/ir/function/basic_block.h"

#include "ink/ir/context.h"

namespace ink::ir
{
  BasicBlock::BasicBlock(const IRContext &Context) noexcept
      : Value(Context, ValueKind::BasicBlock, Context.typePool().getType<TypeKind::Label>())
  {
  }
} // namespace ink::ir
