#ifndef INK_IR_STRUCT_TYPE_H
#define INK_IR_STRUCT_TYPE_H

#include "ink/ir/model/attribute.h"
#include "ink/ir/model/name.h"
#include "ink/ir/model/type.h"

#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace ink::ir
{
  class IRContext;

  struct FieldLayoutConstraints
  {
      std::optional<std::size_t> ExplicitAlignment;
      std::optional<std::size_t> ExplicitOffset;
  };

  struct StructLayoutConstraints
  {
      std::optional<std::size_t> ExplicitAlignment;
      std::optional<std::size_t> Packing;
  };

  class StructField final
  {
    public:
      StructField(ink::ir::Name Name, const Type *ValueType, std::vector<Attribute> Attributes = {}, FieldLayoutConstraints LayoutConstraints = {}) noexcept;
      explicit StructField(const Type *ValueType) noexcept;

      const ink::ir::Name &name() const noexcept;
      const Type *type() const noexcept;
      const std::vector<Attribute> &attributes() const noexcept;
      const FieldLayoutConstraints &layoutConstraints() const noexcept;

    private:
      ink::ir::Name Name;
      const Type *ValueType;
      std::vector<Attribute> Attributes;
      FieldLayoutConstraints LayoutConstraints;
  };

  class StructType final : public Type
  {
    public:
      const ink::ir::Name &name() const noexcept;
      const std::vector<StructField> &fields() const noexcept;
      const StructLayoutConstraints &layoutConstraints() const noexcept;
      std::size_t fieldCount() const noexcept;
      const StructField &field(std::size_t FieldIndex) const noexcept;
      const Type *fieldType(std::size_t FieldIndex) const noexcept;

    private:
      StructType(ink::ir::Name Name, std::vector<StructField> Fields, StructLayoutConstraints LayoutConstraints) noexcept;

      ink::ir::Name Name;
      std::vector<StructField> Fields;
      StructLayoutConstraints LayoutConstraints;

      friend class IRContext;
  };
} // namespace ink::ir

#endif
