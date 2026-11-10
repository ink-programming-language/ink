#pragma once

#include <cstdint>

namespace ink::vm
{
  enum class ExecutableSectionKind : std::uint8_t
  {
    Code,
    ReadOnlyData,
    WritableData,
  };

  // Describes an image range, including any data-alignment padding inside that range.
  // Code ranges contain encoded instructions; data ranges are never decoded as instructions.
  struct ExecutableSection
  {
      ExecutableSectionKind Kind = ExecutableSectionKind::Code;
      std::uint64_t ImageOffset = 0;
      std::uint64_t ByteSize = 0;
  };
} // namespace ink::vm
