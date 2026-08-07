#ifndef INK_LIB_TOKENIZER_UNICODE_H
#define INK_LIB_TOKENIZER_UNICODE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace ink::tokenizer::unicode {

struct DecodeResult {
  char32_t value = 0;
  std::size_t length = 0;
  bool valid = false;
};

DecodeResult decode(std::string_view source, std::size_t offset) noexcept;
bool is_xid_start(char32_t value) noexcept;
bool is_xid_continue(char32_t value) noexcept;
bool is_nfc(std::string_view source);
bool is_default_ignorable(char32_t value) noexcept;
bool is_unicode_whitespace(char32_t value) noexcept;
void append_utf8(std::string& output, char32_t value);

}  // namespace ink::tokenizer::unicode

#endif
