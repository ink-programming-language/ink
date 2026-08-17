#ifndef INK_IR_ATTRIBUTE_H
#define INK_IR_ATTRIBUTE_H

#include "ink/ir/model/constant.h"
#include "ink/ir/model/name.h"

#include <cstdint>
#include <functional>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace ink::ir
{
  enum class AttributeKind : std::uint8_t
  {
#define INK_IR_ATTRIBUTE(Name, Spelling) Name,
#include "ink/ir/attribute.def"
    Count,
  };

  const char *attributeKindName(AttributeKind Kind) noexcept;
  const char *attributeKindSpelling(AttributeKind Kind) noexcept;
  std::optional<AttributeKind> attributeKindFromSpelling(std::string_view Spelling) noexcept;

  class AttributeArgument final
  {
    public:
      AttributeArgument(ink::ir::Name Key, const Constant &Value) noexcept;

      const ink::ir::Name &key() const noexcept;
      const Constant &value() const noexcept;

    private:
      ink::ir::Name Key;
      std::reference_wrapper<const Constant> Value;
  };

  class Attribute final
  {
    public:
      explicit Attribute(AttributeKind Kind, std::vector<AttributeArgument> Arguments = {}) noexcept;

      AttributeKind kind() const noexcept;
      const std::vector<AttributeArgument> &arguments() const noexcept;

    private:
      AttributeKind Kind;
      std::vector<AttributeArgument> Arguments;
  };
} // namespace ink::ir

#endif
