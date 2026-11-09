#pragma once

#include <cstdint>

namespace ink::vm
{
  // ImageOffset identifies the first byte of an eight-byte host data address field in the image, including data fields.
  // SymbolIndex indexes ExecutableFile's NativeData only, never NativeFunctions; Addend is a signed byte displacement.
  // Loading resolves and patches this field. A host address field does not permit ordinary VM loads to dereference host memory.
  struct NativeDataRelocation
  {
      static constexpr std::uint8_t ByteSize = 8;

      std::uint64_t ImageOffset = 0;
      std::uint32_t SymbolIndex = 0;
      std::int64_t Addend = 0;
  };
} // namespace ink::vm
