#ifndef INK_LIB_IR_TEXT_LEXER_H
#define INK_LIB_IR_TEXT_LEXER_H

#include "ink/core/diagnostic.h"
#include "token.h"

#include <string_view>
#include <vector>

namespace ink::ir::text
{
  bool tokenize(std::string_view Text, std::vector<Token> &Tokens, core::Diagnostic &Error);
} // namespace ink::ir::text

#endif
