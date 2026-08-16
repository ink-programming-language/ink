#include "ink/ir/type.h"

namespace ink::ir
{
  Type::~Type() = default;

  const char *typeKindName(TypeKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_TYPE(Name, Spelling) \
  case TypeKind::Name:              \
    return Spelling;
#define INK_IR_DERIVED_TYPE(Name, Spelling) \
  case TypeKind::Name:                      \
    return Spelling;
#include "ink/ir/ir.def"
    case TypeKind::Count:
      return "unknown";
    }
    return "unknown";
  }
} // namespace ink::ir
