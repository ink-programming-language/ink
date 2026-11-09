#pragma once

#include "ink/backend/vm/object/object_symbol.h"

#include <vector>

namespace ink::vm
{
  class ObjectSymbolTable final
  {
    public:
      auto &getEntries(this auto &&Self) noexcept
      {
        return Self.Entries;
      }

    private:
      std::vector<ObjectSymbol> Entries;
  };
} // namespace ink::vm
