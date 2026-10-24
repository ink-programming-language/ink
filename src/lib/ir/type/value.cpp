#include "ink/ir/type/value.h"

#include "../value_string_builder.h"

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

  const Type *Value::type() const noexcept
  {
    return nullptr;
  }

  bool Value::isMutable() const noexcept
  {
    return false;
  }

  std::string Value::toString() const
  {
    return std::string(valueKindName(kind()));
  }

  void Value::appendToString(ValueStringBuilder &Builder) const
  {
    Builder.append(toString());
  }
} // namespace ink::ir
