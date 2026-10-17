#ifndef INK_IR_CONSTANT_INTEGER_CONSTANT_H
#define INK_IR_CONSTANT_INTEGER_CONSTANT_H

#include "ink/ir/constant/constant.h"
#include "ink/ir/type/builtin_type/integer_type.h"

#include <cstdint>
#include <optional>

namespace ink::ir
{
  class ConstantPool;

  // Integer bits are stored by numeric bit position, independent of byte order.
  // The borrowed IntegerType must outlive the constant.
  class IntegerConstant final : public Constant
  {
    public:
      std::uint64_t bits() const noexcept;
      bool isNegative() const noexcept;
      // Returns the numeric value for nonnegative constants, including the full unsigned 64-bit range.
      [[nodiscard]] std::optional<std::uint64_t> tryGetNonNegativeUInt64() const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class ConstantPool;

      IntegerConstant(const IntegerType *IntegerTypeValue, std::uint64_t Bits) noexcept;

      std::uint64_t Bits = 0;
  };
} // namespace ink::ir

#endif
