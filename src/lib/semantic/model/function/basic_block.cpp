#include "ink/semantic/model/function/basic_block.h"

#include "ink/semantic/context.h"

namespace ink::semantic
{
  BasicBlock::BasicBlock(const SemanticContext &Context) noexcept
      : Value(Context, ValueKind::BasicBlock, Context.typePool().getType<TypeKind::Label>())
  {
  }
} // namespace ink::semantic
