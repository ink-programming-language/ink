#ifndef INK_TOKENIZER_TOKENIZER_H
#define INK_TOKENIZER_TOKENIZER_H

#include "ink/core/context.h"
#include "ink/tokenizer/token.h"

#include <cstddef>
#include <memory>
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
      const std::string &source() const noexcept;

      const std::string &sourceName() const noexcept;

      const std::vector<Token> &tokens() const noexcept
      {
        return Tokens;
      }

      core::SourceId sourceId() const noexcept;

      const std::vector<std::size_t> &lineStarts() const noexcept;

      std::string_view raw(const Token &Token) const noexcept;
      std::size_t lineNumber(std::size_t ByteOffset) const noexcept;
      bool isRegisteredWith(const core::SourceManager &Sources) const noexcept;
      bool succeeded() const noexcept;

    private:
      TokenizedBuffer() = default;

      std::shared_ptr<const core::SourceBuffer> Source;
      std::vector<Token> Tokens;
      bool Succeeded = false;

      friend class Tokenizer;
  };

  class Tokenizer
  {
    public:
      explicit Tokenizer(core::FrontendContext &Context, TokenizerOptions Options = {});
      TokenizedBuffer tokenize(std::string Source) const;
      TokenizedBuffer tokenizeSource(core::SourceId Source) const;

    private:
      core::FrontendContext &Context;
      TokenizerOptions Options;
  };

  TokenizedBuffer tokenize(core::FrontendContext &Context, std::string Source, TokenizerOptions Options = {});
  TokenizedBuffer tokenizeSource(core::FrontendContext &Context, core::SourceId Source, TokenizerOptions Options = {});
} // namespace ink::tokenizer

#endif
