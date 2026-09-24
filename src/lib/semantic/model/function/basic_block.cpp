#include "ink/semantic/model/function/basic_block.h"

#include "ink/semantic/model/context.h"

namespace ink::semantic
{
  const BuiltinType &BasicBlock::type() const noexcept
  {
    return context().getLabelType();
  }
} // namespace ink::semantic
