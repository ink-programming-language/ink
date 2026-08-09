#ifndef INK_TYPE_TYPE_H
#define INK_TYPE_TYPE_H

#include "ink/type/type_id.h"

#include <cstdint>
#include <variant>
#include <vector>

namespace ink::type
{
  enum class TypeKind : std::uint8_t
  {
#define INK_TYPE_KIND(Name) Name,
#include "ink/type/type_kind.def"
#undef INK_TYPE_KIND
  };

  const char *typeKindName(TypeKind Kind) noexcept;

  struct FunctionType
  {
    std::vector<TypeId> Parameters;
    TypeId Result;
  };

  bool operator==(const FunctionType &Left, const FunctionType &Right) noexcept;
  bool operator!=(const FunctionType &Left, const FunctionType &Right) noexcept;

  using TypePayload = std::variant<std::monostate, FunctionType>;

  struct Type
  {
    TypeKind Kind = TypeKind::Error;
    TypePayload Payload;
  };

  bool operator==(const Type &Left, const Type &Right) noexcept;
  bool operator!=(const Type &Left, const Type &Right) noexcept;
} // namespace ink::type

#endif
