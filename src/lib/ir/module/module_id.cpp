#include "ink/ir/module/module_id.h"

namespace ink::ir
{
  bool operator==(const ModuleID &Left, const ModuleID &Right) noexcept
  {
    return Left.Components == Right.Components;
  }

  bool operator!=(const ModuleID &Left, const ModuleID &Right) noexcept
  {
    return !(Left == Right);
  }
} // namespace ink::ir
