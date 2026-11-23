#include "machine_module_text_lexer.h"

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

    AsmLexer::AsmLexer(std::string_view Text) noexcept
        : Text(Text)
    {
    }

    void AsmLexer::popCurrent() noexcept
    {
        if (Lookahead)
        {
            Current = std::move(Lookahead);
            Lookahead.reset();
        }
        else
        {
            Current.reset();
        }
    }

    const Token& AsmLexer::current()
    {
        if (!Current)
        {
            Current.emplace(lexNext());
        }
        return *Current;
    }

    const Token& AsmLexer::peekNext()
    {
        static_cast<void>(current());
        if (!Lookahead)
        {
            Lookahead.emplace(lexNext());
        }
        return *Lookahead;
    }

    TokenKind AsmLexer::kind()
    {
        return current().Kind;
    }

    Token AsmLexer::take()
    {
        if (!Current)
        {
            return lexNext();
        }
        Token Result = std::move(*Current);
        popCurrent();
        return Result;
    }

    void AsmLexer::consume()
    {
        static_cast<void>(current());
        popCurrent();
    }

    bool AsmLexer::atLineEnd()
    {
        const TokenKind CurrentKind = kind();
        return CurrentKind == TokenKind::EndOfLine || CurrentKind == TokenKind::EndOfFile;
    }

    bool AsmLexer::atWord(std::string_view Word)
    {
        const Token& Next = current();
        return Next.Kind == TokenKind::Identifier && Next.text() == Word;
    }

    bool AsmLexer::atLabel()
    {
        return kind() == TokenKind::Identifier && peekNext().Kind == TokenKind::Colon;
    }

    bool AsmLexer::atDirective(std::string_view Directive)
    {
        return atWord(Directive) && peekNext().Kind != TokenKind::Colon;
    }

    bool AsmLexer::eat(TokenKind Kind)
    {
        if (kind() != Kind)
        {
            return false;
        }
        popCurrent();
        return true;
    }

    bool AsmLexer::eatToken(TokenKind Kind, Token& Out)
    {
        if (kind() != Kind)
        {
            return false;
        }
        Out = std::move(*Current);
        popCurrent();
        return true;
    }

    void AsmLexer::skipBlankLines()
    {
        while (kind() == TokenKind::EndOfLine)
        {
            popCurrent();
        }
    }

    bool AsmLexer::expect(TokenKind Kind, std::string_view Message)
    {
        const Token& Next = current();
        if (Next.Kind != Kind)
        {
            return parseError(Next.Span, Message);
        }
        popCurrent();
        return false;
    }

    bool AsmLexer::expect(TokenKind Kind, std::string_view Message, Token& Out)
    {
        const Token& Next = current();
        if (Next.Kind != Kind)
        {
            return parseError(Next.Span, Message);
        }
        Out = std::move(*Current);
        popCurrent();
        return false;
    }

    bool AsmLexer::expectEndOfLine(std::string_view Message)
    {
        const Token& Next = current();
        if (Next.Kind == TokenKind::EndOfLine)
        {
            popCurrent();
            return false;
        }
        if (Next.Kind == TokenKind::EndOfFile)
        {
            return false;
        }
        return parseError(Next.Span, Message);
    }

    bool AsmLexer::recordDiagnostic(core::Diagnostic Value, ErrorPriority Origin) noexcept
    {
        if (Origin > Priority)
        {
            Diagnostic = std::move(Value);
            Priority = Origin;
        }
        return true;
    }

    bool AsmLexer::parseError(TextSpan Span, std::string_view Description)
    {
        return recordDiagnostic(makeTextDiagnostic(Span, Description), ErrorPriority::Parser);
    }

    bool AsmLexer::recordParserDiagnostic(core::Diagnostic Value) noexcept
    {
        return recordDiagnostic(std::move(Value), ErrorPriority::Parser);
    }

    core::Diagnostic AsmLexer::takeDiagnostic()
    {
        if (Diagnostic)
        {
            return std::move(*Diagnostic);
        }
        return makeTextDiagnostic(endSpan(), "invalid VM assembly text");
    }

    Token AsmLexer::makeLexError(TextSpan Span, std::string_view Description)
    {
        static_cast<void>(recordDiagnostic(makeTextDiagnostic(Span, Description), ErrorPriority::Lexer));
        return Token{TokenKind::Error, {}, {}, false, Span};
    }

    bool AsmLexer::decodeQuotedString(std::size_t QuoteStart, std::string& Decoded)
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
                static_cast<void>(makeLexError(TextSpan{QuoteStart, Offset - 1}, "unterminated quoted string"));
                return false;
            }
            if (Next == '\\')
            {
                const std::size_t EscapeStart = Offset - 1;
                if (Offset == Text.size() || Text[Offset] == '\n' || Text[Offset] == '\r')
                {
                    static_cast<void>(makeLexError(TextSpan{EscapeStart, Offset}, "incomplete string escape"));
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
                    static_cast<void>(makeLexError(TextSpan{EscapeStart, EscapeEnd}, "expected a two-digit hexadecimal byte escape"));
                    return false;
                }
            }
            else if (Next < 0x20 || Next > 0x7E)
            {
                static_cast<void>(makeLexError(TextSpan{Offset - 1, Offset}, "bytes outside printable ASCII must be escaped"));
                return false;
            }
            else
            {
                Decoded.push_back(static_cast<char>(Next));
            }
        }
        static_cast<void>(makeLexError(TextSpan{QuoteStart, Offset}, "unterminated quoted string"));
        return false;
    }

    Token AsmLexer::lexSymbol(std::size_t AtStart)
    {
        if (Offset < Text.size() && isIdentifierStart(Text[Offset]))
        {
            const std::size_t NameStart = Offset++;
            while (Offset < Text.size() && isIdentifierContinue(Text[Offset]))
            {
                ++Offset;
            }
            return Token{TokenKind::Symbol, Text.substr(NameStart, Offset - NameStart), {}, false, TextSpan{AtStart, Offset}};
        }
        if (Offset < Text.size() && Text[Offset] == '"')
        {
            const std::size_t QuoteStart = Offset++;
            std::string Decoded;
            if (!decodeQuotedString(QuoteStart, Decoded))
            {
                return Token{TokenKind::Error, {}, {}, false, TextSpan{QuoteStart, Offset}};
            }
            if (Decoded.empty())
            {
                return makeLexError(TextSpan{QuoteStart, Offset}, "symbol name must be nonempty");
            }
            return Token{TokenKind::Symbol, Text.substr(AtStart, Offset - AtStart), std::move(Decoded), true, TextSpan{AtStart, Offset}};
        }
        return makeLexError(TextSpan{AtStart, Offset}, "expected a nonempty symbol name after '@'");
    }

    Token AsmLexer::lexNext()
    {
        if (Priority == ErrorPriority::Lexer)
        {
            return Token{TokenKind::Error, {}, {}, false, TextSpan{Offset, Offset}};
        }
        for (;;)
        {
            while (Offset < Text.size() && (Text[Offset] == ' ' || Text[Offset] == '\t'))
            {
                ++Offset;
            }
            if (Offset >= Text.size())
            {
                return Token{TokenKind::EndOfFile, {}, {}, false, TextSpan{Offset, Offset}};
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
                return Token{TokenKind::EndOfLine, Text.substr(Start, Offset - Start), {}, false, TextSpan{Start, Offset}};
            }
            if (Character == '\n')
            {
                const std::size_t Start = Offset++;
                return Token{TokenKind::EndOfLine, Text.substr(Start, 1), {}, false, TextSpan{Start, Offset}};
            }
            const std::size_t Start = Offset++;
            if (Character == '"')
            {
                std::string Decoded;
                if (!decodeQuotedString(Start, Decoded))
                {
                    return Token{TokenKind::Error, {}, {}, false, TextSpan{Start, Offset}};
                }
                return Token{TokenKind::String, Text.substr(Start, Offset - Start), std::move(Decoded), true, TextSpan{Start, Offset}};
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
                return Token{TokenKind::Number, Text.substr(Start, Offset - Start), {}, false, TextSpan{Start, Offset}};
            }
            if (isIdentifierStart(Character))
            {
                while (Offset < Text.size() && isIdentifierContinue(Text[Offset]))
                {
                    ++Offset;
                }
                return Token{TokenKind::Identifier, Text.substr(Start, Offset - Start), {}, false, TextSpan{Start, Offset}};
            }
            if (const auto Punctuation = matchPunctuation(Character))
            {
                return Token{*Punctuation, Text.substr(Start, 1), {}, false, TextSpan{Start, Offset}};
            }
            return makeLexError(TextSpan{Start, Offset}, "unexpected character");
        }
    }
} // namespace ink::vm::detail
