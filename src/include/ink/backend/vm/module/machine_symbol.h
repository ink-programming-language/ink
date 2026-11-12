#pragma once

#include "ink/core/name/name.h"

#include <cstdint>

namespace ink::vm
{
  enum class MachineSymbolLinkage : std::uint8_t
  {
    Local,
    External,
  };

  enum class MachineSymbolKind : std::uint8_t
  {
    Function,
    GlobalVariable,
  };

  struct MachineSymbolDeclaration
  {
      core::Name Name = core::NAME_None;
      MachineSymbolKind Kind = MachineSymbolKind::Function;
      bool IsConstant = false;
  };
} // namespace ink::vm
