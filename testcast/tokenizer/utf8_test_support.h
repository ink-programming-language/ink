#ifndef INK_TESTCAST_TOKENIZER_UTF8_TEST_SUPPORT_H
#define INK_TESTCAST_TOKENIZER_UTF8_TEST_SUPPORT_H

#include <string>

namespace ink::tokenizer {

#if defined(__cpp_char8_t)
inline std::string Utf8(const char8_t* value) { return std::string(reinterpret_cast<const char*>(value), std::char_traits<char8_t>::length(value)); }
#else
inline std::string Utf8(const char* value) { return std::string(value); }
#endif

}  // namespace ink::tokenizer

#endif
