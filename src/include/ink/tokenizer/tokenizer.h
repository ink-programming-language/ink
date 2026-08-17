#ifndef INK_TOKENIZER_TOKENIZER_H
#define INK_TOKENIZER_TOKENIZER_H

#include "ink/core/context.h"
#include "ink/tokenizer/token.h"

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace ink::tokenizer
{
  struct TokenizerOptions
  {
      std::size_t MaxBlockCommentDepth = 1024;
  };

  class TokenizedBuffer
  {
    public:
      const std::string &source() const noexcept
      {
        return Source;
      }

      const std::vector<Token> &tokens() const noexcept
      {
        return Tokens;
      }

      const std::vector<core::Diagnostic> &diagnostics() const noexcept
      {
        return Diagnostics;
      }

      const std::vector<std::size_t> &lineStarts() const noexcept
      {
        return LineStarts;
      }

      std::string_view raw(const Token &Token) const noexcept;
      std::size_t lineNumber(std::size_t ByteOffset) const noexcept;
      bool succeeded() const noexcept;

    private:
      TokenizedBuffer() = default;

      std::string Source;
      std::vector<Token> Tokens;
      std::vector<core::Diagnostic> Diagnostics;
      std::vector<std::size_t> LineStarts;

      friend class Tokenizer;
  };

  class Tokenizer
  {
    public:
      explicit Tokenizer(core::FrontendContext &Context, TokenizerOptions Options = {});
      TokenizedBuffer tokenize(std::string Source) const;

    private:
      core::FrontendContext &Context;
      TokenizerOptions Options;
  };

  TokenizedBuffer tokenize(core::FrontendContext &Context, std::string Source, TokenizerOptions Options = {});
  TokenizedBuffer tokenize(std::string Source, TokenizerOptions Options = {});
} // namespace ink::tokenizer

#endif
