#ifndef INK_SEMANTIC_MODEL_CONSTANT_FLOAT_CONSTANT_H
#define INK_SEMANTIC_MODEL_CONSTANT_FLOAT_CONSTANT_H

#include "ink/semantic/model/constant/constant.h"
#include "ink/semantic/model/type/float_type.h"

#include <cstdint>

namespace ink::semantic
{
  // Exact IEEE binary16/32/64 encoding, independent of host floating-point types.
  // This is a storage representation; arithmetic, conversion and rounding are separate.
  class FloatBits final
  {
    public:
      constexpr FloatBits(std::uint32_t BitWidth, std::uint64_t Bits) noexcept
          : BitWidth(BitWidth),
            Bits(Bits)
      {
      }

      constexpr std::uint32_t bitWidth() const noexcept
      {
        return BitWidth;
      }

      constexpr std::uint64_t bits() const noexcept
      {
        return Bits;
      }

      // Reject unsupported formats and excess high bits instead of silently truncating.
      constexpr bool valid() const noexcept
      {
        switch (BitWidth)
        {
        case 16:
        case 32:
          return (Bits >> BitWidth) == 0;
        case 64:
          return true;
        default:
          return false;
        }
      }

      constexpr bool operator==(const FloatBits &) const noexcept = default;

    private:
      std::uint32_t BitWidth;
      std::uint64_t Bits;
  };

  class FloatConstant final : public Constant
  {
    public:
      // Exact IEEE binary16/32/64 payload, including signed zero and NaN payload bits.
      const FloatBits &value() const noexcept
      {
        return Payload;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::FloatConstant;
      }

    private:
      FloatConstant(const FloatType &ValueType, FloatBits Payload) noexcept
          : Constant(ValueKind::FloatConstant, ValueType),
            Payload(Payload)
      {
      }

      FloatBits Payload;

      friend class ConstantPool;
  };
} // namespace ink::semantic

#endif
