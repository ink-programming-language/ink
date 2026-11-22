#pragma once

#include "ink/core/diagnostic/diagnostic.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace ink::vm::detail
{
    enum class TokenKind : std::uint8_t
    {
        Identifier,
        Symbol,
        Number,
        String,
        Colon,
        Comma,
        LeftBracket,
        RightBracket,
        Plus,
        Minus,
        EndOfLine,
        EndOfFile,
        Error,
    };

    struct Token
    {
            TokenKind Kind = TokenKind::EndOfFile;
            std::string_view View;
            std::string Decoded;
            bool UsesDecoded = false;
            std::size_t BeginOffset = 0;
            std::size_t EndOffset = 0;

            std::string_view text() const noexcept
            {
                return UsesDecoded ? std::string_view(Decoded) : View;
            }
    };

    // Single-pass streaming lexer using TokenKind::Error plus a sticky diagnostic slot,
    // keeping lookahead operations (peek/at/eat/consume) infallible under -fno-exceptions.
    class AsmLexer final
    {
        public:
            AsmLexer(std::string_view Text, std::optional<core::Diagnostic>& ErrorSlot) noexcept;

            const Token& peek() noexcept;
            Token take() noexcept;
            void consume() noexcept;

            bool at(TokenKind Kind) noexcept;
            bool atLineEnd() noexcept;
            bool eat(TokenKind Kind) noexcept;
            std::optional<Token> eatToken(TokenKind Kind) noexcept;
            void skipBlankLines() noexcept;

            bool hasError() const noexcept
            {
                return ErrorSlot.has_value();
            }

            std::size_t lastTokenEnd() const noexcept
            {
                return LastTokenEnd;
            }

            std::size_t endOffset() const noexcept
            {
                return Text.size();
            }

        private:
            Token lexNext() noexcept;
            Token lexSymbol(std::size_t AtStart) noexcept;
            bool decodeQuotedString(std::size_t QuoteStart, std::string& Decoded) noexcept;
            Token makeErrorToken(std::size_t BeginOffset, std::size_t EndOffset, std::string_view Description) noexcept;

            std::string_view Text;
            std::optional<core::Diagnostic>& ErrorSlot;
            std::size_t Offset = 0;
            std::size_t LastTokenEnd = 0;
            Token Current;
            bool HasCurrent = false;
    };
} // namespace ink::vm::detail
