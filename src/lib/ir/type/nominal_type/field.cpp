#include "ink/ir/type/nominal_type/field.h"

namespace ink::ir
{
  Field::Field(const Type *FieldType, core::Name Name, std::uint64_t Offset, const Constant *Initializer) noexcept
      : FieldType(FieldType),
        Name(Name),
        Offset(Offset),
        Initializer(Initializer)
  {
  }

  const Type *Field::type() const noexcept
  {
    return FieldType;
  }

  core::Name Field::name() const noexcept
  {
    return Name;
  }

  bool Field::hasName() const noexcept
  {
    return Name.valid();
  }

  std::uint64_t Field::offset() const noexcept
  {
    return Offset;
  }

  const Constant *Field::initializer() const noexcept
  {
    return Initializer;
  }

  bool Field::hasInitializer() const noexcept
  {
    return Initializer != nullptr;
  }

  bool Field::valid() const noexcept
  {
    return FieldType && hasName() && FieldType->hasLayout() && (!Initializer || Initializer->canInitialize(FieldType));
  }
} // namespace ink::ir
