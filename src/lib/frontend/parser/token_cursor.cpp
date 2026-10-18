#include "ink/frontend/parser/token_cursor.h"
#include <limits>
namespace ink::frontend::parser
{
  TokenBuffer::TokenBuffer(tokenizer::TokenizedBuffer Input)
      : Input(std::move(Input))
  {
    const std::size_t Offset = this->Input.succeeded() ? this->Input.source().size() : this->Input.tokens().empty() ? 0
                                                                                                                    : this->Input.tokens().back().Span.getEnd().getByteOffset();
    End.Span = SourceRange::fromByteOffsets(Offset, Offset);
  }
  const tokenizer::Token &TokenBuffer::token(TokenId Id) const noexcept
  {
    return Id < Input.tokens().size() ? Input.tokens()[Id] : End;
  }
  const tokenizer::Token &TokenBuffer::requireToken(TokenId Id, const core::DiagnosticEngine &Diagnostics, SourceRange Range) const
  {
    if (Id >= size())
    {
      Diagnostics.report<core::DiagnosticKind::ParserInvalidTokenReference>(Input.sourceId(), Range, Id, size());
    }
    return Input.tokens()[Id];
  }
  std::string_view TokenBuffer::spelling(TokenId Id) const noexcept
  {
    return Input.raw(token(Id));
  }
  std::string_view TokenBuffer::spelling(const tokenizer::Token &Token) const noexcept
  {
    return Input.raw(Token);
  }
  const tokenizer::Token &TokenCursor::peek(std::size_t Ahead) const noexcept
  {
    return Buffer.token(Ahead > std::numeric_limits<std::size_t>::max() - Position ? InvalidTokenId : Position + Ahead);
  }
  TokenId TokenCursor::bump() noexcept
  {
    const TokenId Old = Position;
    if (peek().Kind != TokenKind::EndOfFile)
    {
      ++Position;
    }
    return Old;
  }
} // namespace ink::frontend::parser
