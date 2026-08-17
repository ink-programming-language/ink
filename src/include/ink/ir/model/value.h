#ifndef INK_IR_VALUE_H
#define INK_IR_VALUE_H

#include "ink/ir/model/type.h"

#include <cstdint>

namespace ink::ir
{
  enum class ValueKind : std::uint8_t
  {
#define INK_IR_VALUE(Name) Name,
#include "ink/ir/ir.def"
  };

  const char *valueKindName(ValueKind Kind) noexcept;

  class Value
  {
    public:
      virtual ~Value() = 0;

      ValueKind kind() const noexcept
      {
        return Kind;
      }

      const Type &type() const noexcept
      {
        return *ValueType;
      }

    private:
      Value(ValueKind Kind, const Type &ValueType) noexcept
          : Kind(Kind),
            ValueType(&ValueType)
      {
      }

      ValueKind Kind;
      const Type *ValueType;

      friend class Constant;
      friend class Operand;
  };
} // namespace ink::ir

#endif
