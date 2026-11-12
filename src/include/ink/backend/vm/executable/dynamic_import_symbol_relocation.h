#pragma once

#include <cstdint>

namespace ink::vm
{
  // ImageOffset identifies the first byte of an eight-byte host symbol address field in the image.
  // SymbolIndex indexes ExecutableFile's DynamicImportSymbols; Addend is a signed byte displacement.
  // Loading patches the resolved symbol address plus Addend, for either function or data address references.
  // Bound addresses share the runtime address representation used by image and stack storage; callable addresses still require a matching call contract.
  struct DynamicImportSymbolRelocation
  {
      static constexpr std::uint8_t ByteSize = 8;

      std::uint64_t ImageOffset = 0;
      std::uint32_t SymbolIndex = 0;
      std::int64_t Addend = 0;
  };
} // namespace ink::vm
