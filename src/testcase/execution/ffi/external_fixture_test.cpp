#include <cstdint>

#ifdef _WIN32
#define INK_TEST_EXPORT extern "C" __declspec(dllexport)
#else
#define INK_TEST_EXPORT extern "C" __attribute__((visibility("default")))
#endif

namespace
{
  std::int32_t ExternalObservedValue = 0;
}

INK_TEST_EXPORT std::int32_t inkTestExternalZero() noexcept
{
  return 37;
}

INK_TEST_EXPORT void inkTestExternalSet(std::int32_t Value) noexcept
{
  ExternalObservedValue = Value;
}

INK_TEST_EXPORT std::int32_t inkTestExternalGet() noexcept
{
  return ExternalObservedValue;
}

INK_TEST_EXPORT std::int64_t inkTestExternalMix(std::int8_t Small, std::uint16_t Medium, std::int32_t Signed, std::uint64_t Wide) noexcept
{
  return static_cast<std::int64_t>(Small) + Medium + Signed + static_cast<std::int64_t>(Wide >> 60);
}

INK_TEST_EXPORT std::uint64_t inkTestExternalNarrow(std::uint8_t Byte, std::int16_t Half, std::uint32_t Word, std::int64_t Wide) noexcept
{
  return static_cast<std::uint64_t>(Byte) + static_cast<std::uint64_t>(Half + 30000) + Word + static_cast<std::uint64_t>(Wide + 10000000000LL);
}

INK_TEST_EXPORT std::int8_t inkTestExternalNegate(std::int8_t Value) noexcept
{
  return static_cast<std::int8_t>(-Value);
}

INK_TEST_EXPORT std::uint16_t inkTestExternalIncrement(std::uint16_t Value) noexcept
{
  return static_cast<std::uint16_t>(Value + 1);
}

INK_TEST_EXPORT std::int64_t inkTestExternalMany(std::int32_t A, std::int32_t B, std::int32_t C, std::int32_t D, std::int32_t E, std::int32_t F, std::int32_t G, std::int32_t H, std::int32_t I, std::int32_t J) noexcept
{
  return static_cast<std::int64_t>(A) + 2 * B + 3 * C + 4 * D + 5 * E + 6 * F + 7 * G + 8 * H + 9 * I + 10 * J;
}

INK_TEST_EXPORT double inkTestExternalFloat(float Left, double Right) noexcept
{
  return static_cast<double>(Left) * 2.0 + Right;
}

INK_TEST_EXPORT float inkTestExternalFloat32(float Value) noexcept
{
  return Value + 0.5F;
}

INK_TEST_EXPORT float inkTestExternalFloatSeed32() noexcept
{
  return 1.5F;
}

INK_TEST_EXPORT double inkTestExternalFloatSeed64() noexcept
{
  return 2.25;
}

INK_TEST_EXPORT bool inkTestExternalNot(bool Value) noexcept
{
  return !Value;
}

INK_TEST_EXPORT std::uint32_t inkTestExternalByte(std::uint8_t *Buffer, std::uint32_t Index) noexcept
{
  return Buffer[Index];
}

INK_TEST_EXPORT std::uint32_t inkTestExternalMutate(std::uint8_t *Buffer) noexcept
{
  Buffer[0] = 'Z';
  return Buffer[0];
}

#undef INK_TEST_EXPORT
