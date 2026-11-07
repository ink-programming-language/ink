#include "ink/ir/ir_context.h"

namespace ink::ir
{
  IRContext::IRContext()
      : Constants(Types),
        Builder(Types)
  {
  }
} // namespace ink::ir
