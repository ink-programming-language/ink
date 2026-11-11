#pragma once

#include "ink/backend/vm/module/native_data_import.h"
#include "ink/backend/vm/module/native_function_import.h"
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

  // Name is the VM link name; Import.Name is the host resolver's lookup name.
  struct MachineNativeFunctionDeclaration
  {
      core::Name Name = core::NAME_None;
      NativeFunctionImport Import;
  };

  // Name is independent of the host data import identity stored in Import.Name.
  struct MachineNativeDataDeclaration
  {
      core::Name Name = core::NAME_None;
      NativeDataImport Import;
  };
} // namespace ink::vm
