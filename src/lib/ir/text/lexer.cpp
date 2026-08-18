#include "lexer.h"

#include "ink/ir/model/name.h"
#include "ink/tokenizer/unicode.h"

#include <cctype>
#include <cstddef>
#include <string>
#include <utility>

namespace ink::ir::text
{
  namespace
  {
    int hexadecimalValue(char Character)
    {
      if (Character >= '0' && Character <= '9')
      {
        return Character - '0';
      }
      if (Character >= 'a' && Character <= 'f')
      {
        return Character - 'a' + 10;
      }
      if (Character >= 'A' && Character <= 'F')
      {
        return Character - 'A' + 10;
      }
      return -1;
    }

    class TextLexer final
    {
      public:
        explicit TextLexer(std::string_view Text)
            : Text(Text)
        {
        }

        bool tokenize(std::vector<Token> &Tokens, core::Diagnostic &Error)
        {
          while (true)
          {
            skipTrivia();
            if (Position == Text.size())
            {
              Tokens.push_back({TokenKind::End, {}, {Position, Position}});
              return true;
            }

            const std::size_t Start = Position;
            const char Character = Text[Position];
            const tokenizer::unicode::DecodeResult Decoded = tokenizer::unicode::decode(Text, Position);
            if (Decoded.Valid && Name::isStartCharacter(Decoded.Value))
            {
              Tokens.push_back(readName(TokenKind::Identifier));
              continue;
            }
            if (Character == '@')
            {
              advance();
              const tokenizer::unicode::DecodeResult NameStart = tokenizer::unicode::decode(Text, Position);
              if (!NameStart.Valid || !Name::isStartCharacter(NameStart.Value))
              {
                Error = core::makeDiagnostic<core::DiagnosticKind::IrExpectedGlobalNameAfterAt>({Start, Position});
                return false;
              }
              Token NameToken = readName(TokenKind::GlobalName);
              NameToken.Span.Start = Start;
              Tokens.push_back(std::move(NameToken));
              continue;
            }
            if (Character == '%')
            {
              advance();
              const tokenizer::unicode::DecodeResult NameStart = tokenizer::unicode::decode(Text, Position);
              if (NameStart.Valid && Name::isStartCharacter(NameStart.Value))
              {
                Token NameToken = readName(TokenKind::TypeName);
                NameToken.Span.Start = Start;
                Tokens.push_back(std::move(NameToken));
                continue;
              }
              const std::size_t DigitsStart = Position;
              while (Position < Text.size() && std::isdigit(static_cast<unsigned char>(Text[Position])) != 0)
              {
                advance();
              }
              if (DigitsStart == Position)
              {
                Error = core::makeDiagnostic<core::DiagnosticKind::IrExpectedTypeOrSsaNameAfterPercent>({Start, Position});
                return false;
              }
              Tokens.push_back({TokenKind::ValueName, std::string(Text.substr(DigitsStart, Position - DigitsStart)), {Start, Position}});
              continue;
            }
            if (Character == '0' && Position + 1 < Text.size() && (Text[Position + 1] == 'x' || Text[Position + 1] == 'X'))
            {
              advance();
              advance();
              const std::size_t DigitsStart = Position;
              while (Position < Text.size() && std::isxdigit(static_cast<unsigned char>(Text[Position])) != 0)
              {
                advance();
              }
              if (DigitsStart == Position)
              {
                Error = core::makeDiagnostic<core::DiagnosticKind::IrFloatBitPatternRequiresDigits>({Start, Position});
                return false;
              }
              Tokens.push_back({TokenKind::Hexadecimal, std::string(Text.substr(DigitsStart, Position - DigitsStart)), {Start, Position}});
              continue;
            }
            if (Character == '-' || std::isdigit(static_cast<unsigned char>(Character)) != 0)
            {
              if (Character == '-' && (Position + 1 == Text.size() || std::isdigit(static_cast<unsigned char>(Text[Position + 1])) == 0))
              {
                Error = core::makeDiagnostic<core::DiagnosticKind::IrMinusRequiresDecimalInteger>({Start, Start + 1});
                return false;
              }
              advance();
              while (Position < Text.size() && std::isdigit(static_cast<unsigned char>(Text[Position])) != 0)
              {
                advance();
              }
              Tokens.push_back({TokenKind::Integer, std::string(Text.substr(Start, Position - Start)), {Start, Position}});
              continue;
            }
            if (Character == '"')
            {
              Token StringToken;
              if (!readString(StringToken, Error))
              {
                return false;
              }
              Tokens.push_back(std::move(StringToken));
              continue;
            }

            TokenKind Kind;
            switch (Character)
            {
            case '(':
              Kind = TokenKind::LeftParenthesis;
              break;
            case ')':
              Kind = TokenKind::RightParenthesis;
              break;
            case '{':
              Kind = TokenKind::LeftBrace;
              break;
            case '}':
              Kind = TokenKind::RightBrace;
              break;
            case '[':
              Kind = TokenKind::LeftBracket;
              break;
            case ']':
              Kind = TokenKind::RightBracket;
              break;
            case ',':
              Kind = TokenKind::Comma;
              break;
            case ':':
              Kind = TokenKind::Colon;
              break;
            case '=':
              Kind = TokenKind::Equal;
              break;
            case '*':
              Kind = TokenKind::Star;
              break;
            default:
              Error = core::makeDiagnostic<core::DiagnosticKind::IrUnexpectedCharacter>({Start, Start + 1}, std::string(1, Character));
              return false;
            }
            advance();
            Tokens.push_back({Kind, std::string(1, Character), {Start, Position}});
          }
        }

      private:
        void advance(std::size_t Length = 1)
        {
          Position += Length;
        }

        void skipTrivia()
        {
          while (Position < Text.size())
          {
            if (std::isspace(static_cast<unsigned char>(Text[Position])) != 0)
            {
              advance();
              continue;
            }
            if (Text[Position] == ';')
            {
              while (Position < Text.size() && Text[Position] != '\n')
              {
                advance();
              }
              continue;
            }
            break;
          }
        }

        Token readName(TokenKind Kind)
        {
          const std::size_t Start = Position;
          const tokenizer::unicode::DecodeResult First = tokenizer::unicode::decode(Text, Position);
          advance(First.Length);
          while (Position < Text.size())
          {
            const tokenizer::unicode::DecodeResult Continued = tokenizer::unicode::decode(Text, Position);
            if (!Continued.Valid || !Name::isContinueCharacter(Continued.Value))
            {
              break;
            }
            advance(Continued.Length);
          }
          return {Kind, std::string(Text.substr(Start, Position - Start)), {Start, Position}};
        }

        bool readString(Token &Result, core::Diagnostic &Error)
        {
          const std::size_t Start = Position;
          advance();
          std::string Value;
          while (Position < Text.size() && Text[Position] != '"')
          {
            const char Character = Text[Position];
            if (Character == '\n' || Character == '\r')
            {
              Error = core::makeDiagnostic<core::DiagnosticKind::IrStringLiteralRawLineBreak>({Start, Position + 1});
              return false;
            }
            if (Character != '\\')
            {
              Value.push_back(Character);
              advance();
              continue;
            }

            advance();
            if (Position == Text.size())
            {
              Error = core::makeDiagnostic<core::DiagnosticKind::IrUnterminatedStringEscape>({Start, Position});
              return false;
            }
            const int FirstHexadecimal = hexadecimalValue(Text[Position]);
            if (FirstHexadecimal >= 0)
            {
              if (Position + 1 >= Text.size())
              {
                Error = core::makeDiagnostic<core::DiagnosticKind::IrInvalidHexByteEscape>({Position, Text.size()});
                return false;
              }
              const int SecondHexadecimal = hexadecimalValue(Text[Position + 1]);
              if (SecondHexadecimal < 0)
              {
                Error = core::makeDiagnostic<core::DiagnosticKind::IrInvalidHexByteEscape>({Position, Position + 2});
                return false;
              }
              Value.push_back(static_cast<char>((FirstHexadecimal << 4) | SecondHexadecimal));
              advance();
              advance();
              continue;
            }

            switch (Text[Position])
            {
            case 'n':
              Value.push_back('\n');
              break;
            case 'r':
              Value.push_back('\r');
              break;
            case 't':
              Value.push_back('\t');
              break;
            case '\\':
              Value.push_back('\\');
              break;
            case '"':
              Value.push_back('"');
              break;
            default:
              Error = core::makeDiagnostic<core::DiagnosticKind::IrUnknownStringEscape>({Position, Position + 1}, std::string(1, Text[Position]));
              return false;
            }
            advance();
          }
          if (Position == Text.size())
          {
            Error = core::makeDiagnostic<core::DiagnosticKind::IrUnterminatedStringLiteral>({Start, Position});
            return false;
          }
          advance();
          Result = {TokenKind::String, std::move(Value), {Start, Position}};
          return true;
        }

        std::string_view Text;
        std::size_t Position = 0;
    };
  } // namespace

  bool tokenize(std::string_view Text, std::vector<Token> &Tokens, core::Diagnostic &Error)
  {
    return TextLexer(Text).tokenize(Tokens, Error);
  }
} // namespace ink::ir::text
