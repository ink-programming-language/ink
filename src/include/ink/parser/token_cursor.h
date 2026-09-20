#pragma once
#include "ink/parser/ast.h"
#include "ink/tokenizer/tokenizer.h"
#include <memory>
namespace ink::parser
{
  class TokenBuffer
  {
    public:
      explicit TokenBuffer(tokenizer::TokenizedBuffer Input);
      const tokenizer::Token &token(TokenId Id) const noexcept;
      std::string_view spelling(TokenId Id) const noexcept;
      const tokenizer::TokenizedBuffer &lexedFile() const noexcept
      {
        return Input;
      }
      std::size_t size() const noexcept
      {
        return Input.tokens().size();
      }

    private:
      tokenizer::TokenizedBuffer Input;
      tokenizer::Token End;
  };
  class TokenCursor
  {
    public:
      explicit TokenCursor(const TokenBuffer &Buffer)
          : Buffer(Buffer)
      {
      }
      const tokenizer::Token &peek(std::size_t Ahead = 0) const noexcept;
      TokenId bump() noexcept;
      TokenId position() const noexcept
      {
        return Position;
      }
      void restore(TokenId Saved) noexcept
      {
        Position = Saved;
      }

    private:
      const TokenBuffer &Buffer;
      TokenId Position = 0;
  };
} // namespace ink::parser
