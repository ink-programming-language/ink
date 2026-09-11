#include "ink/tokenizer/tokenizer.h"

#include "ink/tokenizer/unicode.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ink::tokenizer
{
  namespace
  {
    using core::Diagnostic;
    using core::DiagnosticKind;
    using core::DiagnosticRelatedKind;
    using core::makeDiagnosticBuilder;
    using unicode::DecodeResult;

    bool startsWith(std::string_view Source, std::size_t Offset, std::string_view Expected) noexcept
    {
      return Offset <= Source.size() && Expected.size() <= Source.size() - Offset && Source.compare(Offset, Expected.size(), Expected) == 0;
    }

    bool isAsciiDigit(char Value) noexcept
    {
      return Value >= '0' && Value <= '9';
    }

    int digitValue(char Value) noexcept
    {
      if (Value >= '0' && Value <= '9')
      {
        return Value - '0';
      }
      if (Value >= 'a' && Value <= 'f')
      {
        return Value - 'a' + 10;
      }
      if (Value >= 'A' && Value <= 'F')
      {
        return Value - 'A' + 10;
      }
      return -1;
    }

    bool isDigitInBase(char Value, unsigned Base) noexcept
    {
      const int Digit = digitValue(Value);
      return Digit >= 0 && static_cast<unsigned>(Digit) < Base;
    }

    bool isScalarValue(char32_t Value) noexcept
    {
      return Value <= 0x10FFFF && !(Value >= 0xD800 && Value <= 0xDFFF);
    }

    class Scanner
    {
      public:
        Scanner(const std::string &Source, std::vector<Token> &Tokens, std::vector<Diagnostic> &Diagnostics, TokenizerOptions Options)
            : Source(Source),
              Tokens(Tokens),
              Diagnostics(Diagnostics),
              Options(Options)
        {
        }

        void run()
        {
          while (Position < Source.size())
          {
            const std::size_t Before = Position;
            Token TokenValue = scanToken();
            if (Position <= Before)
            {
              Position = Before + 1;
              addDiagnostic<DiagnosticKind::InvalidCharacter>({Before, Position});
              TokenValue = makeToken(TokenKind::InvalidCharacter, Before, Position);
            }
            if (Options.PreserveTrivia || !TokenValue.isTrivia())
            {
              Tokens.push_back(std::move(TokenValue));
            }
          }
          Tokens.push_back(makeToken(TokenKind::EndOfFile, Source.size(), Source.size()));
        }

      private:
        Token scanToken()
        {
          const std::size_t Start = Position;
          const DecodeResult Decoded = unicode::decode(Source, Position);
          if (!Decoded.Valid)
          {
            consumeInvalidEncoding(Position, Decoded);
            return makeToken(TokenKind::InvalidEncoding, Start, Position);
          }

          if (Decoded.Value == U'\r' || Decoded.Value == U'\n')
          {
            Position += startsWith(Source, Position, "\r\n") ? 2 : 1;
            return makeToken(TokenKind::LineBreak, Start, Position);
          }
          if (Decoded.Value == U' ' || Decoded.Value == U'\t')
          {
            do
            {
              ++Position;
            } while (Position < Source.size() && (Source[Position] == ' ' || Source[Position] == '\t'));
            return makeToken(TokenKind::SpacesAndTabs, Start, Position);
          }
          if (startsWith(Source, Position, "//"))
          {
            return scanLineComment();
          }
          if (startsWith(Source, Position, "/*"))
          {
            return scanBlockComment();
          }
          if (startsWith(Source, Position, "r\"\"\""))
          {
            return scanString(true, true);
          }
          if (startsWith(Source, Position, "r\""))
          {
            return scanString(true, false);
          }
          if (startsWith(Source, Position, "\"\"\""))
          {
            return scanString(false, true);
          }
          if (Decoded.Value == U'"')
          {
            return scanString(false, false);
          }
          if (Decoded.Value >= U'0' && Decoded.Value <= U'9')
          {
            return scanNumber();
          }
          if (Decoded.Value == U'_' || unicode::isXidStart(Decoded.Value))
          {
            return scanIdentifier();
          }
          if (const auto Symbol = matchSymbolPrefix(std::string_view(Source).substr(Position)))
          {
            Position += symbolSpelling(*Symbol).size();
            return makeToken(TokenKind::Symbol, Start, Position, *Symbol);
          }

          Position += Decoded.Length;
          if (Decoded.Value == 0xFEFF)
          {
            addDiagnostic<DiagnosticKind::UnexpectedBom>({Start, Position});
          }
          else if (unicode::isUnicodeWhitespace(Decoded.Value))
          {
            addDiagnostic<DiagnosticKind::NonAsciiWhitespace>({Start, Position});
          }
          else
          {
            addDiagnostic<DiagnosticKind::InvalidCharacter>({Start, Position});
          }
          return makeToken(TokenKind::InvalidCharacter, Start, Position);
        }

        Token scanLineComment()
        {
          const std::size_t Start = Position;
          Position += 2;
          bool Valid = true;
          while (Position < Source.size() && Source[Position] != '\n' && Source[Position] != '\r')
          {
            const DecodeResult Decoded = unicode::decode(Source, Position);
            if (!Decoded.Valid)
            {
              consumeInvalidEncoding(Position, Decoded);
              Valid = false;
            }
            else
            {
              Position += Decoded.Length;
            }
          }
          return makeToken(Valid ? TokenKind::LineComment : TokenKind::InvalidEncoding, Start, Position);
        }

        Token scanBlockComment()
        {
          const std::size_t Start = Position;
          std::size_t Depth = 1;
          std::vector<std::size_t> OpeningPositions = {Start};
          bool NestingLimitExceeded = false;
          bool ValidEncoding = true;
          Position += 2;
          while (Position < Source.size() && Depth != 0)
          {
            if (startsWith(Source, Position, "/*"))
            {
              ++Depth;
              if (Options.MaxBlockCommentDepth == 0 || Depth <= Options.MaxBlockCommentDepth)
              {
                OpeningPositions.push_back(Position);
              }
              else if (!NestingLimitExceeded)
              {
                NestingLimitExceeded = true;
                addDiagnostic<DiagnosticKind::BlockCommentNestingLimit>({Position, Position + 2});
              }
              Position += 2;
            }
            else if (startsWith(Source, Position, "*/"))
            {
              if (Options.MaxBlockCommentDepth == 0 || Depth <= Options.MaxBlockCommentDepth)
              {
                OpeningPositions.pop_back();
              }
              --Depth;
              Position += 2;
            }
            else
            {
              const DecodeResult Decoded = unicode::decode(Source, Position);
              if (!Decoded.Valid)
              {
                consumeInvalidEncoding(Position, Decoded);
                ValidEncoding = false;
              }
              else
              {
                Position += Decoded.Length;
              }
            }
          }
          if (Depth != 0)
          {
            auto Builder = makeDiagnosticBuilder<DiagnosticKind::UnterminatedBlockComment>({Start, Start + 2}, static_cast<std::uint64_t>(Depth));
            if (Options.MaxBlockCommentDepth == 0 || Depth <= Options.MaxBlockCommentDepth)
            {
              const std::size_t MostRecentOpening = OpeningPositions.back();
              if (MostRecentOpening != Start)
              {
                Builder.related(DiagnosticRelatedKind::MostRecentUnclosedBlockComment, {MostRecentOpening, MostRecentOpening + 2});
              }
            }
            else
            {
              Builder.related(DiagnosticRelatedKind::MostRecentBlockCommentOpeningUnavailable, {});
            }
            Diagnostics.push_back(std::move(Builder).build());
            return makeToken(TokenKind::UnterminatedBlockComment, Start, Position);
          }
          if (!ValidEncoding)
          {
            return makeToken(TokenKind::InvalidEncoding, Start, Position);
          }
          return makeToken(NestingLimitExceeded ? TokenKind::InvalidCharacter : TokenKind::BlockComment, Start, Position);
        }

        Token scanIdentifier()
        {
          const std::size_t Start = Position;
          Position += unicode::decode(Source, Position).Length;
          while (Position < Source.size())
          {
            const DecodeResult Decoded = unicode::decode(Source, Position);
            if (!Decoded.Valid || (Decoded.Value != U'_' && !unicode::isXidContinue(Decoded.Value)))
            {
              break;
            }
            Position += Decoded.Length;
          }
          const std::string_view Spelling(Source.data() + Start, Position - Start);
          const unicode::NfcCheckResult NfcResult = unicode::checkNfc(Spelling);
          if (NfcResult == unicode::NfcCheckResult::Failed)
          {
            addDiagnostic<DiagnosticKind::IdentifierNormalizationFailed>({Start, Position});
            return makeToken(TokenKind::InvalidIdentifier, Start, Position);
          }
          if (NfcResult == unicode::NfcCheckResult::NotNormalized)
          {
            addDiagnostic<DiagnosticKind::IdentifierNotNfc>({Start, Position});
            return makeToken(TokenKind::InvalidIdentifier, Start, Position);
          }
          const auto Keyword = lookupKeyword(Spelling);
          if (Keyword.has_value())
          {
            return makeToken(TokenKind::Keyword, Start, Position, *Keyword);
          }
          if (Spelling == "_")
          {
            return makeToken(TokenKind::Symbol, Start, Position, SymbolKind::Underscore);
          }
          return makeToken(TokenKind::Identifier, Start, Position);
        }

        Token scanNumber()
        {
          const std::size_t Start = Position;
          unsigned Base = 10;
          if (Source[Position] == '0' && Source.size() - Position >= 3)
          {
            switch (Source[Position + 1])
            {
            case 'b':
            case 'B':
              Base = 2;
              break;
            case 'o':
            case 'O':
              Base = 8;
              break;
            case 'x':
            case 'X':
              Base = 16;
              break;
            default:
              break;
            }
            if (Base != 10 && isDigitInBase(Source[Position + 2], Base))
            {
              Position += 3;
              while (Position < Source.size() && isDigitInBase(Source[Position], Base))
              {
                ++Position;
              }
              return makeToken(TokenKind::IntegerLiteral, Start, Position, NumericInfo{Base});
            }
          }

          scanDecimalDigits();
          bool Floating = false;
          if (Source.size() - Position >= 2 && Source[Position] == '.' && isAsciiDigit(Source[Position + 1]))
          {
            Floating = true;
            ++Position;
            scanDecimalDigits();
          }
          if (Position < Source.size() && (Source[Position] == 'e' || Source[Position] == 'E'))
          {
            std::size_t ExponentDigits = Position + 1;
            if (ExponentDigits < Source.size() && (Source[ExponentDigits] == '+' || Source[ExponentDigits] == '-'))
            {
              ++ExponentDigits;
            }
            if (ExponentDigits < Source.size() && isAsciiDigit(Source[ExponentDigits]))
            {
              Floating = true;
              Position = ExponentDigits;
              scanDecimalDigits();
            }
          }
          return makeToken(Floating ? TokenKind::FloatLiteral : TokenKind::IntegerLiteral, Start, Position, NumericInfo{10});
        }

        void scanDecimalDigits()
        {
          while (Position < Source.size() && isAsciiDigit(Source[Position]))
          {
            ++Position;
          }
        }

        Token scanString(bool RawMode, bool Multiline)
        {
          const std::size_t Start = Position;
          const std::string_view Delimiter = Multiline ? "\"\"\"" : "\"";
          Position += (RawMode ? 1 : 0) + Delimiter.size();
          std::string DecodedValue;
          bool Valid = true;
          bool Closed = false;
          while (Position < Source.size())
          {
            if (startsWith(Source, Position, Delimiter))
            {
              Position += Delimiter.size();
              Closed = true;
              break;
            }
            if (!Multiline && (Source[Position] == '\r' || Source[Position] == '\n'))
            {
              break;
            }
            if (!RawMode && Source[Position] == '\\')
            {
              if (!scanEscape(DecodedValue))
              {
                Valid = false;
              }
              continue;
            }
            const DecodeResult Decoded = unicode::decode(Source, Position);
            if (!Decoded.Valid)
            {
              consumeInvalidEncoding(Position, Decoded);
              Valid = false;
            }
            else
            {
              DecodedValue.append(Source, Position, Decoded.Length);
              Position += Decoded.Length;
            }
          }
          if (!Closed)
          {
            if (Multiline)
            {
              addDiagnostic<DiagnosticKind::UnterminatedMultilineStringLiteral>({Start, Position});
            }
            else
            {
              addDiagnostic<DiagnosticKind::UnterminatedStringLiteral>({Start, Position});
            }
            return makeToken(TokenKind::InvalidStringLiteral, Start, Position);
          }
          if (!Valid)
          {
            return makeToken(TokenKind::InvalidStringLiteral, Start, Position);
          }
          const StringMode Mode = Multiline ? (RawMode ? StringMode::RawMultiline : StringMode::EscapedMultiline) : (RawMode ? StringMode::RawSingleLine : StringMode::EscapedSingleLine);
          return makeToken(TokenKind::StringLiteral, Start, Position, StringInfo{Mode, std::move(DecodedValue)});
        }

        bool scanEscape(std::string &DecodedValue)
        {
          const std::size_t Start = Position++;
          if (Position == Source.size() || Source[Position] == '\r' || Source[Position] == '\n')
          {
            addDiagnostic<DiagnosticKind::UnknownEscape>({Start, Position});
            return false;
          }
          const DecodeResult Decoded = unicode::decode(Source, Position);
          if (!Decoded.Valid)
          {
            consumeInvalidEncoding(Position, Decoded);
            return false;
          }
          const char Kind = Source[Position];
          Position += Decoded.Length;
          char32_t Value = 0;
          switch (Kind)
          {
          case '"':
            Value = U'"';
            break;
          case '\\':
            Value = U'\\';
            break;
          case '0':
            Value = U'\0';
            break;
          case 'a':
            Value = U'\a';
            break;
          case 'b':
            Value = U'\b';
            break;
          case 'f':
            Value = U'\f';
            break;
          case 'n':
            Value = U'\n';
            break;
          case 'r':
            Value = U'\r';
            break;
          case 't':
            Value = U'\t';
            break;
          case 'v':
            Value = U'\v';
            break;
          case 'x':
          {
            unsigned Count = 0;
            while (Position < Source.size() && Count < 2 && isDigitInBase(Source[Position], 16))
            {
              Value = static_cast<char32_t>((Value << 4U) | digitValue(Source[Position]));
              ++Count;
              ++Position;
            }
            if (Count != 2)
            {
              addDiagnostic<DiagnosticKind::InvalidHexEscape>({Start, Position});
              return false;
            }
            break;
          }
          case 'u':
            return scanUnicodeEscape(Start, DecodedValue);
          default:
            addDiagnostic<DiagnosticKind::UnknownEscape>({Start, Position});
            return false;
          }
          // Preserve the existing string value model: \xNN denotes U+00NN.
          unicode::appendUtf8(DecodedValue, Value);
          return true;
        }

        bool scanUnicodeEscape(std::size_t Start, std::string &DecodedValue)
        {
          if (Position == Source.size() || Source[Position] != '{')
          {
            addDiagnostic<DiagnosticKind::InvalidUnicodeEscape>({Start, Position});
            return false;
          }
          ++Position;
          std::size_t DigitCount = 0;
          char32_t Value = 0;
          bool ValidDigits = true;
          while (Position < Source.size() && Source[Position] != '}' && Source[Position] != '"' && Source[Position] != '\\' && Source[Position] != '\r' && Source[Position] != '\n')
          {
            const DecodeResult Decoded = unicode::decode(Source, Position);
            if (!Decoded.Valid)
            {
              consumeInvalidEncoding(Position, Decoded);
              ValidDigits = false;
            }
            else
            {
              const int Digit = digitValue(Source[Position]);
              if (Digit < 0)
              {
                ValidDigits = false;
              }
              else if (DigitCount < 6)
              {
                Value = static_cast<char32_t>((Value << 4U) | Digit);
              }
              Position += Decoded.Length;
            }
            ++DigitCount;
          }
          if (Position == Source.size() || Source[Position] != '}')
          {
            addDiagnostic<DiagnosticKind::InvalidUnicodeEscape>({Start, Position});
            return false;
          }
          ++Position;
          if (!ValidDigits || DigitCount == 0 || DigitCount > 6)
          {
            addDiagnostic<DiagnosticKind::InvalidUnicodeEscape>({Start, Position});
            return false;
          }
          if (!isScalarValue(Value))
          {
            addDiagnostic<DiagnosticKind::InvalidUnicodeScalar>({Start, Position});
            return false;
          }
          unicode::appendUtf8(DecodedValue, Value);
          return true;
        }

        void consumeInvalidEncoding(std::size_t &Cursor, DecodeResult Decoded)
        {
          const std::size_t Start = Cursor;
          Cursor += std::min(std::max<std::size_t>(Decoded.Length, 1), Source.size() - Cursor);
          addDiagnostic<DiagnosticKind::InvalidUtf8>({Start, Cursor});
        }

        template <DiagnosticKind Kind>
        void addDiagnostic(core::SourceRange Span)
        {
          Diagnostics.push_back(core::makeDiagnostic<Kind>(Span));
        }

        Token makeToken(TokenKind Kind, std::size_t Start, std::size_t End, TokenPayload Payload = {}) const
        {
          return {Kind, {Start, End}, std::move(Payload)};
        }

        const std::string &Source;
        std::vector<Token> &Tokens;
        std::vector<Diagnostic> &Diagnostics;
        TokenizerOptions Options;
        std::size_t Position = 0;
    };
  } // namespace

  const std::string &TokenizedBuffer::source() const noexcept
  {
    static const std::string Empty;
    return Source == nullptr ? Empty : Source->text();
  }

  const std::string &TokenizedBuffer::sourceName() const noexcept
  {
    static const std::string Empty;
    return Source == nullptr ? Empty : Source->name();
  }

  core::SourceId TokenizedBuffer::sourceId() const noexcept
  {
    return Source == nullptr ? core::SourceId{} : Source->id();
  }

  const std::vector<std::size_t> &TokenizedBuffer::lineStarts() const noexcept
  {
    static const std::vector<std::size_t> Empty;
    return Source == nullptr ? Empty : Source->lineStarts();
  }

  std::string_view TokenizedBuffer::raw(const Token &Token) const noexcept
  {
    if (Token.Span.Start > Token.Span.End || Token.Span.End > source().size())
    {
      return {};
    }
    return std::string_view(source().data() + Token.Span.Start, Token.Span.size());
  }

  std::size_t TokenizedBuffer::lineNumber(std::size_t ByteOffset) const noexcept
  {
    return Source == nullptr ? 0 : Source->lineNumber(ByteOffset);
  }

  bool TokenizedBuffer::isRegisteredWith(const core::SourceManager &Sources) const noexcept
  {
    const std::shared_ptr<const core::SourceBuffer> Registered = Sources.findSource(sourceId());
    return Registered != nullptr && Registered == Source;
  }

  bool TokenizedBuffer::succeeded() const noexcept
  {
    return Succeeded && Source != nullptr;
  }

  Tokenizer::Tokenizer(core::FrontendContext &Context, TokenizerOptions Options)
      : Context(Context),
        Options(Options)
  {
  }

  TokenizedBuffer Tokenizer::tokenize(std::string Source) const
  {
    const core::SourceId Id = Context.sourceManager().addSource("<memory>", std::move(Source));
    return tokenizeSource(Id);
  }

  TokenizedBuffer Tokenizer::tokenizeSource(core::SourceId Source) const
  {
    TokenizedBuffer Result;
    Result.Source = Context.sourceManager().findSource(Source);
    if (Result.Source == nullptr)
    {
      Diagnostic DiagnosticEntry = core::makeDiagnostic<DiagnosticKind::TokenizerSourceNotFound>({});
      DiagnosticEntry.Source = Source;
      Context.diagnosticEngine().report(DiagnosticEntry);
      return Result;
    }
    std::vector<Diagnostic> Diagnostics;
    Scanner ScannerValue(Result.Source->text(), Result.Tokens, Diagnostics, Options);
    ScannerValue.run();
    Result.Succeeded = Diagnostics.empty() && std::none_of(Result.Tokens.begin(), Result.Tokens.end(), [](const Token &TokenValue)
                                                           {
                                                             return TokenValue.isError();
                                                           });
    for (Diagnostic &DiagnosticEntry : Diagnostics)
    {
      DiagnosticEntry.Source = Result.sourceId();
      Context.diagnosticEngine().report(DiagnosticEntry);
    }
    return Result;
  }

  TokenizedBuffer tokenize(core::FrontendContext &Context, std::string Source, TokenizerOptions Options)
  {
    return Tokenizer(Context, Options).tokenize(std::move(Source));
  }

  TokenizedBuffer tokenizeSource(core::FrontendContext &Context, core::SourceId Source, TokenizerOptions Options)
  {
    return Tokenizer(Context, Options).tokenizeSource(Source);
  }
} // namespace ink::tokenizer
