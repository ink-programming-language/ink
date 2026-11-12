#pragma once

#include <cstdint>

namespace ink::vm
{
  // ImageOffset identifies the first byte of an eight-byte address field in the image.
  // TargetOffset is relative to the beginning of Image and may identify its one-past end.
  // Loading patches ImageBase + TargetOffset + Addend after validating the bound address arithmetic.
  struct ImageBaseAddressRelocation
  {
      static constexpr std::uint8_t ByteSize = 8;

      std::uint64_t ImageOffset = 0;
      std::uint64_t TargetOffset = 0;
      std::int64_t Addend = 0;
  };
} // namespace ink::vm
