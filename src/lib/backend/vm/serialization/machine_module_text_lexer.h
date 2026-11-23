#pragma once

#include "machine_module_text_format.h"

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
            // Always borrows a slice of the original input buffer passed to AsmLexer.
            std::string_view View;
            std::string Decoded;
            bool UsesDecoded = false;
            TextSpan Span{};

            std::string_view text() const noexcept
            {
                return UsesDecoded ? std::string_view(Decoded) : View;
            }

            // Returns the borrowed slice in the original input text, valid for the lifetime of the input buffer.
            std::string_view rawView() const noexcept
            {
                return View;
            }
    };

    // Single-pass streaming lexer modeled after LLVM's LLLexer: lookahead and consumption are
    // direct non-expected operations, while ErrorPriority (Lexer > Parser > None) ensures
    // lexer diagnostics are never overwritten by downstream parser errors.
    class AsmLexer final
    {
        public:
            explicit AsmLexer(std::string_view Text) noexcept;

            const Token& current();
            const Token& peekNext();
            TokenKind kind();
            Token take();
            void consume();

            bool atLineEnd();
            bool atWord(std::string_view Word);
            bool atLabel();
            bool atDirective(std::string_view Directive);
            bool eat(TokenKind Kind);
            bool eatToken(TokenKind Kind, Token& Out);
            void skipBlankLines();

            // LLParser-style expectations returning true on error and false on success.
            bool expect(TokenKind Kind, std::string_view Message);
            bool expect(TokenKind Kind, std::string_view Message, Token& Out);
            bool expectEndOfLine(std::string_view Message = "unexpected trailing token");

            // Records a parser diagnostic (returning true) unless an equal- or higher-priority error already exists.
            bool parseError(TextSpan Span, std::string_view Description);
            bool recordParserDiagnostic(core::Diagnostic Diagnostic) noexcept;
            core::Diagnostic takeDiagnostic();

            TextSpan endSpan() const noexcept
            {
                return TextSpan{Text.size(), Text.size()};
            }

        private:
            enum class ErrorPriority : std::uint8_t
            {
                None,
                Parser,
                Lexer,
            };

            void popCurrent() noexcept;
            bool recordDiagnostic(core::Diagnostic Diagnostic, ErrorPriority Origin) noexcept;
            Token makeLexError(TextSpan Span, std::string_view Description);
            Token lexNext();
            Token lexSymbol(std::size_t AtStart);
            bool decodeQuotedString(std::size_t QuoteStart, std::string& Decoded);

            std::string_view Text;
            std::size_t Offset = 0;
            std::optional<Token> Current;
            std::optional<Token> Lookahead;
            std::optional<core::Diagnostic> Diagnostic;
            ErrorPriority Priority = ErrorPriority::None;
    };
} // namespace ink::vm::detail
