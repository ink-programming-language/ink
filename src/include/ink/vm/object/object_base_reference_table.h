#pragma once

#include "ink/vm/instruction/address_operand.h"
#include "ink/vm/object/object_base_reference.h"

#include <vector>

namespace ink::vm
{
  class ObjectBaseReferenceTable final
  {
    public:
      ObjectBaseReferenceTable()
          : Entries(AddressOperand::ReservedSlotCount)
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
