#include "machine_module_text_lexer.h"

#include "machine_module_text_format.h"

#include <utility>

namespace ink::vm::detail
{
    namespace
    {
        constexpr bool isNumberBodyChar(char Character) noexcept
        {
            return isDecimalDigit(Character) || isAsciiLetter(Character) || Character == '.' || Character == '_';
        }

        constexpr std::optional<TokenKind> matchPunctuation(char Character) noexcept
        {
            switch (Character)
            {
            case ':':
                return TokenKind::Colon;
            case ',':
                return TokenKind::Comma;
            case '[':
                return TokenKind::LeftBracket;
            case ']':
                return TokenKind::RightBracket;
            case '+':
                return TokenKind::Plus;
            case '-':
                return TokenKind::Minus;
            default:
                return std::nullopt;
            }
        }
    } // namespace

    AsmLexer::AsmLexer(std::string_view Text, std::optional<core::Diagnostic>& ErrorSlot) noexcept
        : Text(Text),
          ErrorSlot(ErrorSlot)
    {
    }

    const Token& AsmLexer::peek() noexcept
    {
        if (!HasCurrent)
        {
            Current = lexNext();
            HasCurrent = true;
        }
        return Current;
    }

    Token AsmLexer::take() noexcept
    {
        static_cast<void>(peek());
        LastTokenEnd = Current.EndOffset;
        HasCurrent = false;
        return std::move(Current);
    }

    void AsmLexer::consume() noexcept
    {
        static_cast<void>(peek());
        LastTokenEnd = Current.EndOffset;
        HasCurrent = false;
    }

    bool AsmLexer::at(TokenKind Kind) noexcept
    {
        return peek().Kind == Kind;
    }

    bool AsmLexer::atLineEnd() noexcept
    {
        const TokenKind Kind = peek().Kind;
        return Kind == TokenKind::EndOfLine || Kind == TokenKind::EndOfFile;
    }

    bool AsmLexer::eat(TokenKind Kind) noexcept
    {
        if (peek().Kind != Kind)
        {
            return false;
        }
        consume();
        return true;
    }

    std::optional<Token> AsmLexer::eatToken(TokenKind Kind) noexcept
    {
        if (peek().Kind != Kind)
        {
            return std::nullopt;
        }
        return take();
    }

    void AsmLexer::skipBlankLines() noexcept
    {
        while (peek().Kind == TokenKind::EndOfLine)
        {
            consume();
        }
    }

    Token AsmLexer::makeErrorToken(std::size_t BeginOffset, std::size_t EndOffset, std::string_view Description) noexcept
    {
        if (!ErrorSlot)
        {
            ErrorSlot = makeTextDiagnostic(BeginOffset, EndOffset, Description);
        }
        return Token{TokenKind::Error, {}, {}, false, BeginOffset, EndOffset};
    }

    bool AsmLexer::decodeQuotedString(std::size_t QuoteStart, std::string& Decoded) noexcept
    {
        Decoded.clear();
        while (Offset < Text.size())
        {
            const unsigned char Next = static_cast<unsigned char>(Text[Offset++]);
            if (Next == '"')
            {
                return true;
            }
            if (Next == '\n' || Next == '\r')
            {
                static_cast<void>(makeErrorToken(QuoteStart, Offset - 1, "unterminated quoted string"));
                return false;
            }
            if (Next == '\\')
            {
                const std::size_t EscapeStart = Offset - 1;
                if (Offset == Text.size() || Text[Offset] == '\n' || Text[Offset] == '\r')
                {
                    static_cast<void>(makeErrorToken(EscapeStart, Offset, "incomplete string escape"));
                    return false;
                }
                const char Escaped = Text[Offset++];
                if (Escaped == '"' || Escaped == '\\')
                {
                    Decoded.push_back(Escaped);
                }
                else if (const int HighNibble = hexDigitValue(Escaped); HighNibble >= 0 && Offset < Text.size() && hexDigitValue(Text[Offset]) >= 0)
                {
                    const int ByteValue = HighNibble * 16 + hexDigitValue(Text[Offset++]);
                    Decoded.push_back(static_cast<char>(ByteValue));
                }
                else
                {
                    const bool ExtendSecondDigit = HighNibble >= 0 && Offset < Text.size() && Text[Offset] != '"' && Text[Offset] != '\n' && Text[Offset] != '\r';
                    const std::size_t EscapeEnd = ExtendSecondDigit ? Offset + 1 : Offset;
                    static_cast<void>(makeErrorToken(EscapeStart, EscapeEnd, "expected a two-digit hexadecimal byte escape"));
                    return false;
                }
            }
            else if (Next < 0x20 || Next > 0x7E)
            {
                static_cast<void>(makeErrorToken(Offset - 1, Offset, "bytes outside printable ASCII must be escaped"));
                return false;
            }
            else
            {
                Decoded.push_back(static_cast<char>(Next));
            }
        }
        static_cast<void>(makeErrorToken(QuoteStart, Offset, "unterminated quoted string"));
        return false;
    }

    Token AsmLexer::lexSymbol(std::size_t AtStart) noexcept
    {
        if (Offset < Text.size() && isIdentifierStart(Text[Offset]))
        {
            const std::size_t NameStart = Offset++;
            while (Offset < Text.size() && isIdentifierContinue(Text[Offset]))
            {
                ++Offset;
            }
            return Token{TokenKind::Symbol, Text.substr(NameStart, Offset - NameStart), {}, false, AtStart, Offset};
        }
        if (Offset < Text.size() && Text[Offset] == '"')
        {
            const std::size_t QuoteStart = Offset++;
            std::string Decoded;
            if (!decodeQuotedString(QuoteStart, Decoded))
            {
                return Token{TokenKind::Error, {}, {}, false, QuoteStart, Offset};
            }
            if (Decoded.empty())
            {
                return makeErrorToken(QuoteStart, Offset, "symbol name must be nonempty");
            }
            return Token{TokenKind::Symbol, Text.substr(AtStart, Offset - AtStart), std::move(Decoded), true, AtStart, Offset};
        }
        return makeErrorToken(AtStart, Offset, "expected a nonempty symbol name after '@'");
    }

    Token AsmLexer::lexNext() noexcept
    {
        if (ErrorSlot)
        {
            return Token{TokenKind::Error, {}, {}, false, Offset, Offset};
        }
        for (;;)
        {
            while (Offset < Text.size() && (Text[Offset] == ' ' || Text[Offset] == '\t'))
            {
                ++Offset;
            }
            if (Offset >= Text.size())
            {
                return Token{TokenKind::EndOfFile, {}, {}, false, Offset, Offset};
            }
            const char Character = Text[Offset];
            if (Character == ';')
            {
                while (Offset < Text.size() && Text[Offset] != '\n' && Text[Offset] != '\r')
                {
                    ++Offset;
                }
                continue;
            }
            if (Character == '\r')
            {
                const std::size_t Start = Offset++;
                if (Offset < Text.size() && Text[Offset] == '\n')
                {
                    ++Offset;
                }
                return Token{TokenKind::EndOfLine, Text.substr(Start, Offset - Start), {}, false, Start, Offset};
            }
            if (Character == '\n')
            {
                const std::size_t Start = Offset++;
                return Token{TokenKind::EndOfLine, Text.substr(Start, 1), {}, false, Start, Offset};
            }
            const std::size_t Start = Offset++;
            if (Character == '"')
            {
                std::string Decoded;
                if (!decodeQuotedString(Start, Decoded))
                {
                    return Token{TokenKind::Error, {}, {}, false, Start, Offset};
                }
                return Token{TokenKind::String, Text.substr(Start, Offset - Start), std::move(Decoded), true, Start, Offset};
            }
            if (Character == '@')
            {
                return lexSymbol(Start);
            }
            if (isDecimalDigit(Character))
            {
                const bool IsHex = Character == '0' && Offset < Text.size() && (Text[Offset] == 'x' || Text[Offset] == 'X');
                if (IsHex)
                {
                    ++Offset;
                }
                while (Offset < Text.size() && isNumberBodyChar(Text[Offset]))
                {
                    const char Body = Text[Offset++];
                    const bool IsExponent = IsHex ? (Body == 'p' || Body == 'P') : (Body == 'e' || Body == 'E');
                    if (IsExponent && Offset < Text.size() && (Text[Offset] == '+' || Text[Offset] == '-'))
                    {
                        ++Offset;
                    }
                }
                return Token{TokenKind::Number, Text.substr(Start, Offset - Start), {}, false, Start, Offset};
            }
            if (isIdentifierStart(Character))
            {
                while (Offset < Text.size() && isIdentifierContinue(Text[Offset]))
                {
                    ++Offset;
                }
                return Token{TokenKind::Identifier, Text.substr(Start, Offset - Start), {}, false, Start, Offset};
            }
            if (const auto Punctuation = matchPunctuation(Character))
            {
                return Token{*Punctuation, Text.substr(Start, 1), {}, false, Start, Offset};
            }
            return makeErrorToken(Start, Offset, "unexpected character");
        }
    }
} // namespace ink::vm::detail
