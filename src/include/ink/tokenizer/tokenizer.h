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

      // Validate a saved lexical snapshot before registering its owned source.
      // This restores tokens without rerunning the tokenizer or emitting diagnostics.
      static std::optional<TokenizedBuffer> fromSnapshot(core::SourceManager &Sources, std::string Name, std::string Text, std::vector<Token> Tokens, bool Succeeded);

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
      explicit Tokenizer(core::FrontendContext &Context);
      TokenizedBuffer tokenize(std::string Source) const;
      TokenizedBuffer tokenizeSource(core::SourceId Source) const;

    private:
      core::FrontendContext &Context;
  };

  TokenizedBuffer tokenize(core::FrontendContext &Context, std::string Source);
  TokenizedBuffer tokenizeSource(core::FrontendContext &Context, core::SourceId Source);
} // namespace ink::tokenizer

#endif
