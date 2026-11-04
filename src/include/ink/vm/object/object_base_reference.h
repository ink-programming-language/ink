#pragma once

#include <cstdint>
#include <optional>

namespace ink::vm
{
  struct ObjectBaseReference
  {
      // Vector index is the object's BaseIndex. The first three slots denote BP, SP, and PC and have no symbol.
      // Every later entry names an object-local symbol, including unresolved imports.
      std::optional<std::uint32_t> SymbolIndex;
  };
} // namespace ink::vm
