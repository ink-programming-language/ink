#pragma once

#include <cstdint>

namespace ink::vm
{
  struct ObjectBaseReference
  {
      // Vector index is the object's symbol-only base-reference index, independent of RegisterId.
      // Every entry names an object-local symbol, including unresolved imports.
      std::uint32_t SymbolIndex = 0;
  };
} // namespace ink::vm
