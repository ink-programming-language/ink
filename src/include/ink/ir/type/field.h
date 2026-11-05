#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/type/type.h"

namespace ink::ir
{
  // Copies an interned name handle; the borrowed type must remain alive whenever the field is accessed.
  class Field
  {
    public:
      Field(core::Name Name, const Type &FieldType) noexcept
          : Name(Name),
            FieldType(&FieldType)
      {
      }

      Field(core::Name, const Type &&) = delete;

      core::Name name() const noexcept
      {
        return Name;
      }

      const Type &type() const noexcept
      {
        return *FieldType;
      }

    private:
      core::Name Name;
      const Type *FieldType;
  };
} // namespace ink::ir
