#ifndef INK_TESTCAST_TOKENIZER_UTF8_TEST_SUPPORT_H
#define INK_TESTCAST_TOKENIZER_UTF8_TEST_SUPPORT_H

#include <string>

namespace ink::tokenizer
{
#if defined(__cpp_char8_t)
  inline std::string utf8(const char8_t *Value)
  {
    return std::string(reinterpret_cast<const char *>(Value), std::char_traits<char8_t>::length(Value));
  }
#else
  inline std::string utf8(const char *Value)
  {
    return std::string(Value);
  }
#endif
} // namespace ink::tokenizer

#endif
