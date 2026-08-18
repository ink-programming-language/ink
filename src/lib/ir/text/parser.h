#ifndef INK_LIB_IR_TEXT_PARSER_H
#define INK_LIB_IR_TEXT_PARSER_H

#include "ink/core/diagnostic.h"
#include "token.h"

#include <vector>

namespace ink::ir::text
{
  class ModuleDraft;

  bool parse(ModuleDraft &Draft, std::vector<Token> Tokens, core::Diagnostic &Error);
} // namespace ink::ir::text

#endif
