#ifndef INK_IR_TYPE_H
#define INK_IR_TYPE_H

#include <cstddef>
#include <cstdint>
#include <string>
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
    const std::string &name() const noexcept
    {
      return Name;
    }

    const std::vector<const Type *> &fieldTypes() const noexcept
    {
      return FieldTypes;
    }

  private:
    StructType(std::string Name, std::vector<const Type *> FieldTypes)
        : Type(TypeKind::Struct),
          Name(std::move(Name)),
          FieldTypes(std::move(FieldTypes))
    {
    }

    std::string Name;
    std::vector<const Type *> FieldTypes;

    friend class IRContext;
  };
} // namespace ink::ir

#endif
