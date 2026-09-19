#ifndef INK_TOKENIZER_UNICODE_H
#define INK_TOKENIZER_UNICODE_H

#include <cstddef>
#include <optional>
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

  DecodeResult decode(std::string_view Source, std::size_t Offset) noexcept;
  bool isXidStart(char32_t Value) noexcept;
  bool isXidContinue(char32_t Value) noexcept;
  bool normalizeNfc(std::string_view Source, std::string &Output);
  // ASCII case-insensitive, but spaces and hyphens must match exactly.
  std::optional<char32_t> lookupName(std::string_view Name);
  void appendUtf8(std::string &Output, char32_t Value);
} // namespace ink::tokenizer::unicode

#endif
