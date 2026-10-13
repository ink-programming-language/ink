#ifndef INK_IR_PACKAGE_PACKAGE_PATH_H
#define INK_IR_PACKAGE_PACKAGE_PATH_H

#include "ink/core/name/name.h"

#include <vector>

namespace ink::ir
{
  // An absolute package path of validated, canonical name segments, excluding the module name.
  // Components are ordered from the root; an empty path denotes the root package.
  // Relative imports are resolved before constructing this value.
  struct PackagePath final
  {
      std::vector<core::Name> Components;
  };

  // Paths are equal exactly when they contain the same name segments in the same order.
  bool operator==(const PackagePath &Left, const PackagePath &Right) noexcept;
  bool operator!=(const PackagePath &Left, const PackagePath &Right) noexcept;
} // namespace ink::ir

#endif
