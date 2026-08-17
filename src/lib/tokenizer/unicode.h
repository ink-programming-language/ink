#ifndef INK_LIB_TOKENIZER_UNICODE_H
#define INK_LIB_TOKENIZER_UNICODE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace ink::tokenizer::unicode
{
  struct DecodeResult
  {
    char32_t Value = 0;
    std::size_t Length = 0;
    bool Valid = false;
  };

  enum class NfcCheckResult
  {
    Normalized,
    NotNormalized,
    Failed,
  };

  DecodeResult decode(std::string_view Source, std::size_t Offset) noexcept;
  bool isXidStart(char32_t Value) noexcept;
  bool isXidContinue(char32_t Value) noexcept;
  NfcCheckResult checkNfc(std::string_view Source) noexcept;
  bool isDefaultIgnorable(char32_t Value) noexcept;
  bool isUnicodeWhitespace(char32_t Value) noexcept;
  void appendUtf8(std::string &Output, char32_t Value);
} // namespace ink::tokenizer::unicode

#endif
