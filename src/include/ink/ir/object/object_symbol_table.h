#pragma once

#include "ink/ir/object/object_symbol.h"

#include <vector>

namespace ink::ir
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
} // namespace ink::ir
