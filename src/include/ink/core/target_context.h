#ifndef INK_CORE_TARGET_CONTEXT_H
#define INK_CORE_TARGET_CONTEXT_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>

namespace ink::core
{
  enum class ByteOrder : std::uint8_t
  {
    LittleEndian,
    BigEndian,
  };

  enum class PointerWidth : std::uint8_t
  {
    Bits32 = 32,
    Bits64 = 64,
  };

  class TargetContext final
  {
  public:
    constexpr TargetContext(PointerWidth Width, ByteOrder Order) noexcept : TargetContext(Width, Order, false)
    {
    }

    static TargetContext native() noexcept
    {
      static_assert(sizeof(void *) == 4 || sizeof(void *) == 8);
      const PointerWidth NativePointerWidth = sizeof(void *) == 4 ? PointerWidth::Bits32 : PointerWidth::Bits64;
      const std::uint16_t NativeValue = 1;
      std::uint8_t FirstByte = 0;
      std::memcpy(&FirstByte, &NativeValue, sizeof(FirstByte));
      return TargetContext(NativePointerWidth, FirstByte == 1 ? ByteOrder::LittleEndian : ByteOrder::BigEndian, true);
    }

    constexpr PointerWidth pointerWidth() const noexcept
    {
      return Width;
    }

    constexpr std::size_t pointerByteWidth() const noexcept
    {
      return static_cast<std::size_t>(Width) / 8U;
    }

    constexpr std::uint64_t maximumPointerSizeValue() const noexcept
    {
      return Width == PointerWidth::Bits32 ? std::numeric_limits<std::uint32_t>::max() : std::numeric_limits<std::uint64_t>::max();
    }

    constexpr ByteOrder byteOrder() const noexcept
    {
      return Order;
    }

    constexpr bool isNativeAbiCompatible() const noexcept
    {
      return NativeAbiCompatible;
    }

    constexpr bool operator==(const TargetContext &Other) const noexcept
    {
      return Width == Other.Width && Order == Other.Order && NativeAbiCompatible == Other.NativeAbiCompatible;
    }

    constexpr bool operator!=(const TargetContext &Other) const noexcept
    {
      return !(*this == Other);
    }

  private:
    constexpr TargetContext(PointerWidth Width, ByteOrder Order, bool NativeAbiCompatible) noexcept : Width(Width), Order(Order), NativeAbiCompatible(NativeAbiCompatible)
    {
    }

    PointerWidth Width;
    ByteOrder Order;
    bool NativeAbiCompatible;
  };
} // namespace ink::core

#endif
