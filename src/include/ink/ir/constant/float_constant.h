#ifndef INK_IR_CONSTANT_FLOAT_CONSTANT_H
#define INK_IR_CONSTANT_FLOAT_CONSTANT_H

#include "ink/ir/constant/constant.h"
#include "ink/ir/type/float_type.h"

#include <cstdint>

namespace ink::ir
{
  class ConstantContext;

  // IEEE binary32/binary64 bits preserve signed zero, infinities and NaN payloads.
  // Bits use numeric bit positions, independent of host or target byte order.
  // The borrowed FloatType must outlive the constant.
  class FloatConstant final : public Constant
  {
    public:
      std::uint64_t bits() const noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class ConstantContext;

      FloatConstant(const FloatType *FloatTypeValue, std::uint64_t Bits) noexcept;

      std::uint64_t Bits = 0;
  };
} // namespace ink::ir

#endif
