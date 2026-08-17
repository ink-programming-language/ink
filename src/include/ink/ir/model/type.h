#ifndef INK_IR_TYPE_H
#define INK_IR_TYPE_H

#include <cstddef>
#include <cstdint>

namespace ink::ir
{
  class IRContext;
  class StructType;

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
} // namespace ink::ir

#endif
