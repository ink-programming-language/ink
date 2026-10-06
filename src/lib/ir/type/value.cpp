#include "ink/ir/type/value.h"

namespace ink::ir
{
  std::string_view valueKindName(ValueKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_VALUE(Kind, Name, FixedSize) \
  case ValueKind::Kind:                  \
    return Name;
#include "ink/ir/type/Value.def"
#undef INK_VALUE
    }
    return "unknown";
  }

  Value::Value(ValueKind Kind) noexcept
      : Kind(Kind)
  {
  }

  Value::~Value() noexcept = default;

  ValueKind Value::kind() const noexcept
  {
    return Kind;
  }
} // namespace ink::ir
