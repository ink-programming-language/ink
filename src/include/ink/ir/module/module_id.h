#ifndef INK_IR_MODULE_MODULE_ID_H
#define INK_IR_MODULE_MODULE_ID_H

#include "ink/core/name/name.h"

#include <vector>

namespace ink::ir
{
  // An absolute module identifier of validated, canonical name segments, including the module name.
  // Components are ordered from the root; the last segment is the module name, and an empty list denotes an unnamed module.
  // Relative imports are resolved before constructing this value.
  class ModuleID final
  {
    public:
      std::vector<core::Name> Components;
  };

  // Identifiers are equal exactly when they contain the same name segments in the same order.
  bool operator==(const ModuleID &Left, const ModuleID &Right) noexcept;
  bool operator!=(const ModuleID &Left, const ModuleID &Right) noexcept;
} // namespace ink::ir

#endif
