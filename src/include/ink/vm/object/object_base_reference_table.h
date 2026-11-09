#pragma once

#include "ink/vm/object/object_base_reference.h"

#include <vector>

namespace ink::vm
{
  class ObjectBaseReferenceTable final
  {
    public:
      auto &getEntries(this auto &&Self) noexcept
      {
        return Self.Entries;
      }

    private:
      std::vector<ObjectBaseReference> Entries;
  };
} // namespace ink::vm
