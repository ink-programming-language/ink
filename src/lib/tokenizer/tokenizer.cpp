#include "ink/tokenizer/tokenizer.h"
#include "ink/tokenizer/unicode.h"

#include <algorithm>
#include <utility>

namespace ink::tokenizer
{
  namespace
  {
    using core::DiagnosticKind;

    bool isDecimal(char Character) noexcept
    {
      return Character >= '0' && Character <= '9';
    }

    int digitValue(char Character) noexcept
    {
      if (isDecimal(Character))
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

    bool isScalar(char32_t Value) noexcept
    {
      return Value <= 0x10FFFF && (Value < 0xD800 || Value > 0xDFFF);
    }

    class Scanner
    {
      public:
        Scanner(const std::string &Source, std::vector<Token> &Tokens, std::vector<core::Diagnostic> &Diagnostics)
            : Source(Source),
              Tokens(Tokens),
              Diagnostics(Diagnostics)
        {
        }

        bool run()
        {
          // Validate the entire buffer first, including bytes hidden in comments
          // and raw literals. Scanning then only visits valid scalar boundaries.
          if (!validateSource())
          {
            return false;
          }
          while (Position < Source.size())
          {
            const char Character = Source[Position];
            if (Character == ' ' || Character == '\t' || isNewline())
            {
              ++Position;
              continue;
            }
            if (startsWith("//"))
            {
              Position += 2;
              while (Position < Source.size() && !isNewline())
              {
                ++Position;
              }
              continue;
            }
            if (startsWith("/*"))
            {
              const std::size_t Start = Position;
              const std::size_t End = Source.find("*/", Position + 2);
              if (End == std::string::npos)
              {
                return fail<DiagnosticKind::UnterminatedBlockComment>(Start, Source.size());
              }
              Position = End + 2;
              continue;
            }
            const bool Raw = (Character == 'r' || Character == 'R') && Position + 1 < Source.size() && (Source[Position + 1] == '"' || Source[Position + 1] == '\'');
            if (Raw || Character == '"' || Character == '\'')
            {
              if (!scanLiteral(Raw))
              {
                return false;
              }
              continue;
            }
            if (isDecimal(Character))
            {
              if (!scanNumber())
              {
                return false;
              }
              continue;
            }
            const unicode::DecodeResult Decoded = unicode::decode(Source, Position);
            if (Decoded.Value == '_' || unicode::isXidStart(Decoded.Value))
            {
              if (!scanIdentifier())
              {
                return false;
              }
              continue;
            }
            if (const auto Kind = matchSymbolPrefix(std::string_view(Source).substr(Position)))
            {
              const std::size_t Start = Position;
              Position += tokenSpelling(*Kind).size();
              emit(*Kind, Start);
              continue;
            }
            return fail<DiagnosticKind::InvalidCharacter>(Position, Position + Decoded.Length);
          }
          emit(TokenKind::EndOfFile, Position);
          return true;
        }

      private:
        template <DiagnosticKind Kind>
        bool fail(std::size_t Start, std::size_t End)
        {
          Diagnostics.push_back(core::makeDiagnostic<Kind>(core::SourceRange::fromByteOffsets(Start, End)));
          return false;
        }

        bool validateSource()
        {
          if (Source.size() > core::SourceLocation::MaxByteOffset)
          {
            Diagnostics.push_back(core::makeDiagnostic<DiagnosticKind::SourceTooLarge>({}));
            return false;
          }
          for (std::size_t Cursor = 0; Cursor < Source.size();)
          {
            const unicode::DecodeResult Decoded = unicode::decode(Source, Cursor);
            if (!Decoded.Valid)
            {
              return fail<DiagnosticKind::InvalidUtf8>(Cursor, Cursor + Decoded.Length);
            }
            if (Decoded.Value == 0)
            {
              return fail<DiagnosticKind::NullInSource>(Cursor, Cursor + 1);
            }
            if (Cursor == 0 && Decoded.Value == 0xFEFF)
            {
              return fail<DiagnosticKind::UnexpectedBom>(0, Decoded.Length);
            }
            Cursor += Decoded.Length;
          }
          return true;
        }

        bool startsWith(std::string_view Text) const noexcept
        {
          return std::string_view(Source).substr(Position, Text.size()) == Text;
        }

        bool isNewline() const noexcept
        {
          return Position < Source.size() && (Source[Position] == '\r' || Source[Position] == '\n');
        }

        void consumeNewline() noexcept
        {
          const char First = Source[Position++];
          if (First == '\r' && Position < Source.size() && Source[Position] == '\n')
          {
            ++Position;
          }
        }

        void emit(TokenKind Kind, std::size_t Start, TokenPayload Payload = {})
        {
          Tokens.push_back({Kind, core::SourceRange::fromByteOffsets(Start, Position), std::move(Payload)});
        }

        bool scanIdentifier()
        {
          const std::size_t Start = Position;
          Position += unicode::decode(Source, Position).Length;
          while (Position < Source.size())
          {
            const unicode::DecodeResult Decoded = unicode::decode(Source, Position);
            if (!unicode::isXidContinue(Decoded.Value))
            {
              break;
            }
            Position += Decoded.Length;
          }
          std::string Name;
          if (!unicode::normalizeNfc(std::string_view(Source).substr(Start, Position - Start), Name))
          {
            return fail<DiagnosticKind::NormalizationFailed>(Start, Position);
          }
          if (Name == "_")
          {
            emit(TokenKind::Underscore, Start);
          }
          else if (const auto Keyword = lookupKeyword(Name))
          {
            emit(*Keyword, Start);
          }
          else
          {
            emit(TokenKind::Identifier, Start, IdentifierInfo{std::move(Name)});
          }
          return true;
        }

        std::size_t numericTailEnd() const noexcept
        {
          std::size_t End = Position;
          while (End < Source.size())
          {
            const unicode::DecodeResult Decoded = unicode::decode(Source, End);
            if (!unicode::isXidContinue(Decoded.Value))
            {
              break;
            }
            End += Decoded.Length;
          }
          return End;
        }

        bool scanNumber()
        {
          const std::size_t Start = Position;
          unsigned Base = 10;
          bool Floating = false;
          if (Source[Position] == '0' && Position + 1 < Source.size())
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
          }
          if (Base != 10)
          {
            Position += 2;
            const std::size_t DigitsStart = Position;
            while (Position < Source.size())
            {
              const int Digit = digitValue(Source[Position]);
              if (Digit < 0 || static_cast<unsigned>(Digit) >= Base)
              {
                break;
              }
              ++Position;
            }
            if (Position < Source.size() && isDecimal(Source[Position]))
            {
              return fail<DiagnosticKind::DigitOutOfRange>(Start, numericTailEnd());
            }
            if (Position == DigitsStart)
            {
              return fail<DiagnosticKind::MissingBaseDigits>(Start, numericTailEnd());
            }
            if (Base == 16 && Position < Source.size())
            {
              const char Next = Source[Position];
              const bool HexFraction = Next == '.' && Position + 1 < Source.size() && (digitValue(Source[Position + 1]) >= 0 || Source[Position + 1] == 'p' || Source[Position + 1] == 'P');
              if (Next == 'p' || Next == 'P' || HexFraction)
              {
                return fail<DiagnosticKind::HexadecimalFloat>(Start, Position + 1);
              }
            }
          }
          else
          {
            while (Position < Source.size() && isDecimal(Source[Position]))
            {
              ++Position;
            }
            if (Position + 1 < Source.size() && Source[Position] == '.' && isDecimal(Source[Position + 1]))
            {
              Floating = true;
              ++Position;
              while (Position < Source.size() && isDecimal(Source[Position]))
              {
                ++Position;
              }
            }
            if (Position < Source.size() && (Source[Position] == 'e' || Source[Position] == 'E'))
            {
              Floating = true;
              ++Position;
              if (Position < Source.size() && (Source[Position] == '+' || Source[Position] == '-'))
              {
                ++Position;
              }
              const std::size_t ExponentStart = Position;
              while (Position < Source.size() && isDecimal(Source[Position]))
              {
                ++Position;
              }
              if (Position == ExponentStart)
              {
                return fail<DiagnosticKind::MissingExponentDigits>(Start, numericTailEnd());
              }
            }
          }
          const std::size_t TailEnd = numericTailEnd();
          if (TailEnd != Position)
          {
            return fail<DiagnosticKind::InvalidNumericSuffix>(Start, TailEnd);
          }
          emit(Floating ? TokenKind::FloatLiteral : TokenKind::IntegerLiteral, Start, NumericInfo{Base});
          return true;
        }

        bool scanEscape(char32_t &Value, bool &ProducesScalar)
        {
          const std::size_t Start = Position++;
          if (Position == Source.size())
          {
            return fail<DiagnosticKind::InvalidEscape>(Start, Position);
          }
          if (isNewline())
          {
            consumeNewline();
            ProducesScalar = false;
            return true;
          }
          const char Character = Source[Position++];
          switch (Character)
          {
          case '\\':
            Value = '\\';
            return true;
          case '\'':
            Value = '\'';
            return true;
          case '"':
            Value = '"';
            return true;
          case 'a':
            Value = '\a';
            return true;
          case 'b':
            Value = '\b';
            return true;
          case 'f':
            Value = '\f';
            return true;
          case 'n':
            Value = '\n';
            return true;
          case 'r':
            Value = '\r';
            return true;
          case 't':
            Value = '\t';
            return true;
          case 'v':
            Value = '\v';
            return true;
          default:
            break;
          }
          if (Character >= '0' && Character <= '7')
          {
            Value = Character - '0';
            for (unsigned Count = 1; Count < 3 && Position < Source.size() && Source[Position] >= '0' && Source[Position] <= '7'; ++Count)
            {
              Value = Value * 8 + Source[Position++] - '0';
            }
            return Value <= 0xFF || fail<DiagnosticKind::OctalEscapeOutOfRange>(Start, Position);
          }
          if (Character == 'x' || Character == 'u' || Character == 'U')
          {
            const unsigned Digits = Character == 'x' ? 2 : Character == 'u' ? 4
                                                                            : 8;
            Value = 0;
            for (unsigned Count = 0; Count < Digits; ++Count)
            {
              if (Position == Source.size() || digitValue(Source[Position]) < 0)
              {
                return fail<DiagnosticKind::InvalidEscape>(Start, Position);
              }
              Value = (Value << 4U) | static_cast<char32_t>(digitValue(Source[Position++]));
            }
            return isScalar(Value) || fail<DiagnosticKind::InvalidUnicodeScalar>(Start, Position);
          }
          if (Character == 'N')
          {
            if (Position == Source.size() || Source[Position] != '{')
            {
              return fail<DiagnosticKind::InvalidNamedEscape>(Start, Position);
            }
            const std::size_t NameStart = ++Position;
            while (Position < Source.size() && Source[Position] != '}' && Source[Position] != '"' && Source[Position] != '\'' && !isNewline())
            {
              Position += unicode::decode(Source, Position).Length;
            }
            if (Position == Source.size() || Source[Position] != '}' || Position == NameStart)
            {
              return fail<DiagnosticKind::InvalidNamedEscape>(Start, Position);
            }
            const std::string_view Name = std::string_view(Source).substr(NameStart, Position - NameStart);
            ++Position;
            const std::optional<char32_t> NamedValue = unicode::lookupName(Name);
            if (!NamedValue)
            {
              return fail<DiagnosticKind::UnknownUnicodeName>(Start, Position);
            }
            Value = *NamedValue;
            return isScalar(Value) || fail<DiagnosticKind::InvalidUnicodeScalar>(Start, Position);
          }
          // Include the whole offending scalar in the diagnostic range.
          Position += unicode::decode(Source, Position - 1).Length - 1;
          return fail<DiagnosticKind::InvalidEscape>(Start, Position);
        }

        bool scanLiteral(bool Raw)
        {
          const std::size_t Start = Position;
          Position += Raw ? 1 : 0;
          const char Quote = Source[Position];
          const bool Character = Quote == '\'';
          const bool Multiline = !Character && startsWith("\"\"\"");
          const std::size_t DelimiterLength = Multiline ? 3 : 1;
          Position += DelimiterLength;
          std::string Decoded;
          std::size_t ScalarCount = 0;
          char32_t LastScalar = 0;
          while (Position < Source.size())
          {
            if (Source[Position] == Quote && (!Multiline || startsWith("\"\"\"")))
            {
              Position += DelimiterLength;
              if (Character)
              {
                if (ScalarCount != 1)
                {
                  return fail<DiagnosticKind::InvalidCharacterLength>(Start, Position);
                }
                emit(TokenKind::CharLiteral, Start, CharInfo{LastScalar, Raw});
              }
              else
              {
                const StringMode Mode = Multiline ? (Raw ? StringMode::RawMultiline : StringMode::EscapedMultiline) : (Raw ? StringMode::RawSingleLine : StringMode::EscapedSingleLine);
                emit(TokenKind::StringLiteral, Start, StringInfo{Mode, std::move(Decoded)});
              }
              return true;
            }
            char32_t Value = 0;
            bool ProducesScalar = true;
            if (isNewline())
            {
              const std::size_t NewlineStart = Position;
              consumeNewline();
              if (!Multiline)
              {
                return fail<DiagnosticKind::NewlineInLiteral>(NewlineStart, Position);
              }
              Value = '\n';
            }
            else if (!Raw && Source[Position] == '\\')
            {
              if (!scanEscape(Value, ProducesScalar))
              {
                return false;
              }
            }
            else
            {
              const unicode::DecodeResult Scalar = unicode::decode(Source, Position);
              Value = Scalar.Value;
              Position += Scalar.Length;
            }
            if (ProducesScalar)
            {
              ++ScalarCount;
              LastScalar = Value;
              if (!Character)
              {
                unicode::appendUtf8(Decoded, Value);
              }
            }
          }
          return Character ? fail<DiagnosticKind::UnterminatedCharacter>(Start, Position) : fail<DiagnosticKind::UnterminatedString>(Start, Position);
        }

        const std::string &Source;
        std::vector<Token> &Tokens;
        std::vector<core::Diagnostic> &Diagnostics;
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
    if (Source == nullptr || Token.Span.isInvalid() || Token.Span.getEnd().getByteOffset() > source().size())
    {
      return {};
    }
    return std::string_view(source()).substr(Token.Span.getBegin().getByteOffset(), Token.Span.size());
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

  std::optional<TokenizedBuffer> TokenizedBuffer::fromSnapshot(core::SourceManager &Sources, std::string Name, std::string Text, std::vector<Token> Tokens, bool Succeeded)
  {
    if (Text.size() > core::SourceLocation::MaxByteOffset || (Succeeded && (Tokens.empty() || Tokens.back().Kind != TokenKind::EndOfFile)))
    {
      return std::nullopt;
    }
    if (Succeeded)
    {
      for (std::size_t Offset = 0; Offset < Text.size();)
      {
        const auto Decoded = unicode::decode(Text, Offset);
        if (!Decoded.Valid || Decoded.Value == 0 || (Offset == 0 && Decoded.Value == 0xFEFF))
        {
          return std::nullopt;
        }
        Offset += Decoded.Length;
      }
    }
    std::size_t PreviousEnd = 0;
    for (std::size_t Index = 0; Index < Tokens.size(); ++Index)
    {
      const auto &Entry = Tokens[Index];
      if (!Entry.Span.isValid() || Entry.Span.getBegin().getByteOffset() < PreviousEnd || Entry.Span.getEnd().getByteOffset() > Text.size())
      {
        return std::nullopt;
      }
      PreviousEnd = Entry.Span.getEnd().getByteOffset();
      if (Entry.Kind == TokenKind::EndOfFile)
      {
        if (!Succeeded || Index + 1 != Tokens.size() || !Entry.Span.empty() || PreviousEnd != Text.size())
        {
          return std::nullopt;
        }
      }
      else if (Entry.Span.empty())
      {
        return std::nullopt;
      }
      switch (Entry.Kind)
      {
      case TokenKind::Identifier:
      {
        const auto *Info = std::get_if<IdentifierInfo>(&Entry.Payload);
        if (!Info || Info->Name != std::string_view(Text).substr(Entry.Span.getBegin().getByteOffset(), Entry.Span.size()))
        {
          return std::nullopt;
        }
        break;
      }
      case TokenKind::IntegerLiteral:
      case TokenKind::FloatLiteral:
      {
        const auto *Info = std::get_if<NumericInfo>(&Entry.Payload);
        if (!Info || (Info->Base != 2 && Info->Base != 8 && Info->Base != 10 && Info->Base != 16))
        {
          return std::nullopt;
        }
        break;
      }
      case TokenKind::StringLiteral:
      {
        const auto *Info = std::get_if<StringInfo>(&Entry.Payload);
        if (!Info)
        {
          return std::nullopt;
        }
        switch (Info->Mode)
        {
        case StringMode::EscapedSingleLine:
        case StringMode::RawSingleLine:
        case StringMode::EscapedMultiline:
        case StringMode::RawMultiline:
          break;
        default:
          return std::nullopt;
        }
        for (std::size_t Offset = 0; Offset < Info->Decoded.size();)
        {
          const auto Decoded = unicode::decode(Info->Decoded, Offset);
          if (!Decoded.Valid)
          {
            return std::nullopt;
          }
          Offset += Decoded.Length;
        }
        break;
      }
      case TokenKind::CharLiteral:
      {
        const auto *Info = std::get_if<CharInfo>(&Entry.Payload);
        if (!Info || !isScalar(Info->Value))
        {
          return std::nullopt;
        }
        break;
      }
      case TokenKind::EndOfFile:
#define INK_KEYWORD(Name, DisplayName, Spelling) case TokenKind::Name:
#define INK_SYMBOL(Name, DisplayName, Spelling) case TokenKind::Name:
#include "ink/tokenizer/token.def"
#undef INK_SYMBOL
#undef INK_KEYWORD
        if (!std::holds_alternative<std::monostate>(Entry.Payload))
        {
          return std::nullopt;
        }
        break;
      default:
        return std::nullopt;
      }
    }
    TokenizedBuffer Result;
    const core::SourceId Id = Sources.addSource(std::move(Name), std::move(Text));
    Result.Source = Sources.findSource(Id);
    Result.Tokens = std::move(Tokens);
    Result.Succeeded = Succeeded;
    return Result;
  }

  Tokenizer::Tokenizer(core::FrontendContext &Context)
      : Context(Context)
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
      core::Diagnostic Entry = core::makeDiagnostic<DiagnosticKind::TokenizerSourceNotFound>({});
      Entry.Source = Source;
      Context.diagnosticEngine().report(Entry);
      return Result;
    }
    std::vector<core::Diagnostic> Diagnostics;
    Result.Succeeded = Scanner(Result.Source->text(), Result.Tokens, Diagnostics).run();
    for (core::Diagnostic &Entry : Diagnostics)
    {
      Entry.Source = Source;
      Context.diagnosticEngine().report(Entry);
    }
    return Result;
  }

  TokenizedBuffer tokenize(core::FrontendContext &Context, std::string Source)
  {
    return Tokenizer(Context).tokenize(std::move(Source));
  }

  TokenizedBuffer tokenizeSource(core::FrontendContext &Context, core::SourceId Source)
  {
    return Tokenizer(Context).tokenizeSource(Source);
  }
} // namespace ink::tokenizer
