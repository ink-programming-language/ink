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
      virtual ~Value() noexcept;

      Value(const Value &) = delete;
      Value &operator=(const Value &) = delete;

      ValueKind kind() const noexcept;

      // The represented value's storage size and ABI alignment, both in bytes.
      // Concrete types compute these from their width, elements, fields and target.
      // Alignment must be a nonzero power of two, including for zero-sized values.
      virtual std::uint64_t size() const noexcept = 0;
      virtual std::uint64_t alignment() const noexcept = 0;

    protected:
      explicit Value(ValueKind Kind) noexcept;

    private:
      ValueKind Kind = ValueKind::Bool;
  };
} // namespace ink::ir

#endif
