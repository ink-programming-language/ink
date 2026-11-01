#include "IR/Type/type_kind.h"

namespace ink::ir
{
  std::string_view typeKindName(TypeKind Kind) noexcept
  {
    switch (Kind)
    {
#define TYPE(Kind, Class, Spelling) \
  case TypeKind::Kind:              \
    return Spelling;
#include "IR/Type/type.def"
#undef TYPE
    }
    return "InvalidTypeKind";
  }
} // namespace ink::ir
