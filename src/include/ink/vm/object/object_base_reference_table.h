#pragma once

#include "ink/vm/object/object_base_reference.h"

#include <cstdint>
#include <vector>

namespace ink::vm
{
  class ObjectBaseReferenceTable final
  {
    public:
      // The object format reserves the first three entries for BP, SP, and PC.
      static constexpr std::uint32_t ReservedSlotCount = 3;

      ObjectBaseReferenceTable()
          : Entries(ReservedSlotCount)
      {
      }

      auto &getEntries(this auto &&Self) noexcept
      {
        return Self.Entries;
      }

    private:
      std::vector<ObjectBaseReference> Entries;
  };
} // namespace ink::vm
