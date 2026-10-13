#include "ink/ir/package/package_path.h"

namespace ink::ir
{
  bool operator==(const PackagePath &Left, const PackagePath &Right) noexcept
  {
    return Left.Components == Right.Components;
  }

  bool operator!=(const PackagePath &Left, const PackagePath &Right) noexcept
  {
    return !(Left == Right);
  }
} // namespace ink::ir
