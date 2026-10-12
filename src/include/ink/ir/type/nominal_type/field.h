#ifndef INK_IR_TYPE_NOMINAL_TYPE_FIELD_H
#define INK_IR_TYPE_NOMINAL_TYPE_FIELD_H

#include "ink/core/name/name.h"
#include "ink/ir/constant/constant.h"

#include <cstdint>

namespace ink::ir
{
  // A named data member with a borrowed type and an explicit byte offset in its enclosing value.
  // The type and optional constant initializer share a context and must outlive external copies.
  // Field records do not own runtime storage; absence of a constant does not imply zero initialization.
  class Field final
  {
    public:
      explicit Field(const Type *FieldType, core::Name Name = core::NAME_None, std::uint64_t Offset = 0, const Constant *Initializer = nullptr) noexcept;

      const Type *type() const noexcept;
      core::Name name() const noexcept;
      bool hasName() const noexcept;
      std::uint64_t offset() const noexcept;
      const Constant *initializer() const noexcept;
      bool hasInitializer() const noexcept;
      // Checks the name, runtime type and initializer compatibility, including the local nullptr value for pointer fields.
      // The enclosing nominal type checks ownership and storage bounds.
      bool valid() const noexcept;

    private:
      const Type *FieldType = nullptr;
      core::Name Name = core::NAME_None;
      std::uint64_t Offset = 0;
      const Constant *Initializer = nullptr;
  };
} // namespace ink::ir

#endif
