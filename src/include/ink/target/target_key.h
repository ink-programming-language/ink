#ifndef INK_TARGET_TARGET_KEY_H
#define INK_TARGET_TARGET_KEY_H

#include <cstdint>
#include <string>

namespace ink::target
{
  enum class TargetEndianness : std::uint8_t
  {
    Unknown,
    Little,
    Big,
  };

  struct TargetKey
  {
    std::string Triple;
    std::string Cpu;
    std::string Features;
    std::uint32_t PointerBitWidth = 0;
    TargetEndianness Endianness = TargetEndianness::Unknown;

    bool isValid() const noexcept;
    std::string canonicalString() const;
  };

  bool operator==(const TargetKey &Left, const TargetKey &Right) noexcept;
  bool operator!=(const TargetKey &Left, const TargetKey &Right) noexcept;
  const char *targetEndiannessName(TargetEndianness Endianness) noexcept;
} // namespace ink::target

#endif
