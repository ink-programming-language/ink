#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/type/type.h"

namespace ink::ir
{
  // Copies an interned name handle; the borrowed type must remain alive whenever the field is accessed.
  class Field
  {
    public:
      Field(core::Name Name, ValueRef FieldType) noexcept
          : Name(Name),
            FieldType(&FieldType.get())
      {
      }

      core::Name name() const noexcept
      {
        return Name;
      }

      const Value &type() const noexcept
      {
        return *FieldType;
      }

    private:
      core::Name Name;
      const Value *FieldType;
  };
} // namespace ink::ir
