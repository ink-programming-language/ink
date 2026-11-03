#pragma once

#include "ink/ir/instruction/address_operand.h"
#include "ink/ir/object/object_base_reference.h"

#include <vector>

namespace ink::ir
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
} // namespace ink::ir
