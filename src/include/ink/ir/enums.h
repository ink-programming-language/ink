#pragma once

#include <cstdint>

namespace ink::ir
{
  // Applies to both method and field declarations.
  enum class MemberKind : std::uint8_t
  {
    Instance,
    Static,
  };

  // Source-level access is independent of native symbol import/export.
  enum class SymbolVisibility : std::uint8_t
  {
    Private,
    Public,
  };

  // Describes native symbol exposure for function and global variable declarations.
  // Independent of Ink visibility and object-file linkage.
  enum class NativeLinkage : std::uint8_t
  {
    None,
    Import,
    Export,
  };
} // namespace ink::ir
