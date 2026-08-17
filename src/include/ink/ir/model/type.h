#ifndef INK_IR_TYPE_H
#define INK_IR_TYPE_H

#include "ink/ir/model/name.h"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace ink::ir
{
  class IRContext;

  enum class TypeKind : std::uint8_t
  {
#define INK_IR_TYPE(Name, Spelling) Name,
#define INK_IR_DERIVED_TYPE(Name, Spelling) Name,
#include "ink/ir/ir.def"
    Count,
  };

  const char *typeKindName(TypeKind Kind) noexcept;
  bool isFloatingPointType(TypeKind Kind) noexcept;
  std::size_t floatingPointBitWidth(TypeKind Kind) noexcept;

  class Type
  {
    public:
      virtual ~Type();

      TypeKind kind() const noexcept
      {
        return Kind;
      }

    protected:
      explicit Type(TypeKind Kind) noexcept
          : Kind(Kind)
      {
      }

    private:
      TypeKind Kind;

      friend class IRContext;
  };

  class StructType final : public Type
  {
    public:
      const Name &name() const noexcept
      {
        return Name;
      }

      const std::vector<const Type *> &fieldTypes() const noexcept
      {
        return FieldTypes;
      }

    private:
      StructType(Name Name, std::vector<const Type *> FieldTypes)
          : Type(TypeKind::Struct),
            Name(std::move(Name)),
            FieldTypes(std::move(FieldTypes))
      {
      }

      ink::ir::Name Name;
      std::vector<const Type *> FieldTypes;

      friend class IRContext;
  };
} // namespace ink::ir

#endif
