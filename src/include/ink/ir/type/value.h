#ifndef INK_IR_TYPE_VALUE_H
#define INK_IR_TYPE_VALUE_H

#include <cstdint>
#include <string_view>

namespace ink::ir
{
  enum class ValueKind : std::uint8_t
  {
#define INK_VALUE(Kind, Name, FixedSize) Kind,
#include "ink/ir/type/Value.def"
#undef INK_VALUE
  };

  std::string_view valueKindName(ValueKind Kind) noexcept;

  class Value
  {
    public:
      virtual ~Value() noexcept = 0;

      Value(const Value &) = delete;
      Value &operator=(const Value &) = delete;

      ValueKind kind() const noexcept;

    protected:
      explicit Value(ValueKind Kind) noexcept;

    private:
      ValueKind Kind = ValueKind::Bool;
  };
} // namespace ink::ir

#endif
