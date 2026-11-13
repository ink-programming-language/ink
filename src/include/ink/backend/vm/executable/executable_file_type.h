#pragma once

#include "ink/core/name/name.h"

#include <cstdint>

namespace ink::vm
{
  // Identifies an unresolved host symbol independently of whether a reference calls it or takes its address.
  // Name is its unique lookup identity; call contracts and bound runtime addresses are outside this record.
  struct DynamicImportSymbol
  {
      core::Name Name = core::NAME_None;
  };

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

  // ImageOffset identifies the first byte of an eight-byte address field in the image.
  // TargetOffset is relative to the beginning of Image and may identify its one-past end.
  // Loading patches the runtime image's host base + TargetOffset + Addend after checking address arithmetic.
  struct ImageBaseAddressRelocation
  {
      static constexpr std::uint8_t ByteSize = 8;

      std::uint64_t ImageOffset = 0;
      std::uint64_t TargetOffset = 0;
      std::int64_t Addend = 0;
  };

  // Names an exported image address. Offset is relative to the beginning of Image, never a host pointer.
  struct ExportSymbol
  {
      core::Name Name = core::NAME_None;
      std::uint64_t Offset = 0;
  };
} // namespace ink::vm
