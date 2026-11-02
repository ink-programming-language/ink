#pragma once

#include <cstdint>
#include <string_view>

namespace ink::ir
{
  enum class TypeKind : std::uint8_t
  {
#define TYPE(Kind, Class, Spelling) Kind,
#include "ink/ir/Type/type.def"
#undef TYPE
  };

  std::string_view typeKindName(TypeKind Kind) noexcept;
} // namespace ink::ir
