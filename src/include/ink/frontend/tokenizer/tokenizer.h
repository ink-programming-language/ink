#ifndef INK_FRONTEND_TOKENIZER_TOKENIZER_H
#define INK_FRONTEND_TOKENIZER_TOKENIZER_H

#include "ink/frontend/context.h"
#include "ink/frontend/tokenizer/token.h"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace ink::frontend::tokenizer
{
  // Owns the source buffer and decoded values. Global encoding validation runs
  // first. Subsequent failures retain the completed prefix without an EOF/error token.
  class TokenizedBuffer
  {
    public:
      const std::string &source() const noexcept;
      const std::string &sourceName() const noexcept;
      core::SourceId sourceId() const noexcept;
      const std::vector<std::size_t> &lineStarts() const noexcept;
      std::string_view raw(const Token &Token) const noexcept;
      std::size_t lineNumber(std::size_t ByteOffset) const noexcept;
      bool isRegisteredWith(const core::SourceManager &Sources) const noexcept;

      const std::vector<Token> &tokens() const noexcept
      {
        return Tokens;
      }

      bool succeeded() const noexcept
      {
        return Succeeded;
      }

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
      explicit Tokenizer(frontend::FrontendContext &Context);
      TokenizedBuffer tokenize(std::string Source) const;
      TokenizedBuffer tokenizeSource(core::SourceId Source) const;

    private:
      frontend::FrontendContext &Context;
  };

  TokenizedBuffer tokenize(frontend::FrontendContext &Context, std::string Source);
  TokenizedBuffer tokenizeSource(frontend::FrontendContext &Context, core::SourceId Source);
} // namespace ink::frontend::tokenizer

#endif
