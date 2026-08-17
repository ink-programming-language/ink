#include "ink/ir/model/attribute.h"

namespace ink::ir
{
  const char *attributeKindName(AttributeKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_ATTRIBUTE(Name, Spelling) \
  case AttributeKind::Name:              \
    return #Name;
#include "ink/ir/attribute.def"
    case AttributeKind::Count:
      return "Count";
    }
    return "Unknown";
  }

  const char *attributeKindSpelling(AttributeKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_ATTRIBUTE(Name, Spelling) \
  case AttributeKind::Name:              \
    return Spelling;
#include "ink/ir/attribute.def"
    case AttributeKind::Count:
      return "unknown";
    }
    return "unknown";
  }

  std::optional<AttributeKind> attributeKindFromSpelling(std::string_view Spelling) noexcept
  {
    for (std::uint8_t Value = 0; Value < static_cast<std::uint8_t>(AttributeKind::Count); ++Value)
    {
      const AttributeKind Kind = static_cast<AttributeKind>(Value);
      if (Spelling == attributeKindSpelling(Kind))
      {
        return Kind;
      }
    }
    return std::nullopt;
  }

  AttributeArgument::AttributeArgument(ink::ir::Name Key, const Constant &Value) noexcept
      : Key(std::move(Key)),
        Value(Value)
  {
  }

  const ink::ir::Name &AttributeArgument::key() const noexcept
  {
    return Key;
  }

  const Constant &AttributeArgument::value() const noexcept
  {
    return Value.get();
  }

  Attribute::Attribute(AttributeKind Kind, std::vector<AttributeArgument> Arguments) noexcept
      : Kind(Kind),
        Arguments(std::move(Arguments))
  {
  }

  AttributeKind Attribute::kind() const noexcept
  {
    return Kind;
  }

  const std::vector<AttributeArgument> &Attribute::arguments() const noexcept
  {
    return Arguments;
  }
} // namespace ink::ir
