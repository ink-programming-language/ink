#ifndef INK_LIB_ABI_CODEC_H
#define INK_LIB_ABI_CODEC_H

#include "ink/abi/linkage_identity.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace ink::abi::detail
{
  enum class NameValidationResult : std::uint8_t
  {
    Valid,
    Empty,
    InvalidUtf8,
    NotNormalized,
    NormalizationFailed,
  };

  NameValidationResult validateName(std::string_view Name) noexcept;
  void appendUnsigned(std::string &Output, std::uint64_t Value);
  void appendUpperHexadecimal(std::string &Output, std::string_view Bytes);
  std::uint8_t uppercaseHexadecimalValue(char Character) noexcept;
  std::size_t canonicalBitHexDigitCount(const Type &ValueType) noexcept;
  bool validateCanonicalBits(const Type &ValueType, std::string_view Bits) noexcept;
} // namespace ink::abi::detail

#endif
