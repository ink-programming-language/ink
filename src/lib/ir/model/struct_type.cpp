#include "ink/ir/model/struct_type.h"

#include <cassert>

namespace ink::ir
{
  StructField::StructField(ink::ir::Name Name, const Type *ValueType, std::vector<Attribute> Attributes, FieldLayoutConstraints LayoutConstraints) noexcept
      : Name(std::move(Name)),
        ValueType(ValueType),
        Attributes(std::move(Attributes)),
        LayoutConstraints(std::move(LayoutConstraints))
  {
  }

  StructField::StructField(const Type *ValueType) noexcept
      : ValueType(ValueType)
  {
  }

  const ink::ir::Name &StructField::name() const noexcept
  {
    return Name;
  }

  const Type *StructField::type() const noexcept
  {
    return ValueType;
  }

  const std::vector<Attribute> &StructField::attributes() const noexcept
  {
    return Attributes;
  }

  const FieldLayoutConstraints &StructField::layoutConstraints() const noexcept
  {
    return LayoutConstraints;
  }

  StructType::StructType(ink::ir::Name Name, std::vector<StructField> Fields, StructLayoutConstraints LayoutConstraints) noexcept
      : Type(TypeKind::Struct),
        Name(std::move(Name)),
        Fields(std::move(Fields)),
        LayoutConstraints(std::move(LayoutConstraints))
  {
  }

  const ink::ir::Name &StructType::name() const noexcept
  {
    return Name;
  }

  const std::vector<StructField> &StructType::fields() const noexcept
  {
    return Fields;
  }

  const StructLayoutConstraints &StructType::layoutConstraints() const noexcept
  {
    return LayoutConstraints;
  }

  std::size_t StructType::fieldCount() const noexcept
  {
    return Fields.size();
  }

  const StructField &StructType::field(std::size_t FieldIndex) const noexcept
  {
    assert(FieldIndex < Fields.size());
    return Fields[FieldIndex];
  }

  const Type *StructType::fieldType(std::size_t FieldIndex) const noexcept
  {
    return field(FieldIndex).type();
  }
} // namespace ink::ir
