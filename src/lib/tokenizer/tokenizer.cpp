#include "ink/tokenizer/tokenizer.h"

#include "ink/tokenizer/unicode.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <iterator>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ink::tokenizer
{
  namespace
  {
    using core::Diagnostic;
    using core::DiagnosticArgumentName;
    using core::DiagnosticKind;
    using core::DiagnosticRelatedKind;
    using core::DiagnosticSourceContext;
    using core::makeDiagnosticBuilder;
    using unicode::DecodeResult;

    struct KeywordEntry
    {
        std::string_view Spelling;
        KeywordKind Kind;
    };

    struct BuiltinTypeEntry
    {
        std::string_view Spelling;
        BuiltinTypeKind Kind;
    };

    struct SuffixEntry
    {
        std::string_view Spelling;
        NumericSuffix Suffix;
        bool Floating;
    };

    constexpr KeywordEntry Keywords[] = {
        {"as", KeywordKind::As},
        {"async", KeywordKind::Async},
        {"await", KeywordKind::Await},
        {"break", KeywordKind::Break},
        {"catch", KeywordKind::Catch},
        {"class", KeywordKind::Class},
        {"comptime", KeywordKind::Comptime},
        {"const", KeywordKind::Const},
        {"continue", KeywordKind::Continue},
        {"decorator", KeywordKind::Decorator},
        {"defer", KeywordKind::Defer},
        {"else", KeywordKind::Else},
        {"enum", KeywordKind::Enum},
        {"extern", KeywordKind::Extern},
        {"final", KeywordKind::Final},
        {"for", KeywordKind::For},
        {"from", KeywordKind::From},
        {"func", KeywordKind::Func},
        {"if", KeywordKind::If},
        {"implicit", KeywordKind::Implicit},
        {"import", KeywordKind::Import},
        {"in", KeywordKind::In},
        {"interface", KeywordKind::Interface},
        {"let", KeywordKind::Let},
        {"match", KeywordKind::Match},
        {"override", KeywordKind::Override},
        {"private", KeywordKind::Private},
        {"protected", KeywordKind::Protected},
        {"public", KeywordKind::Public},
        {"return", KeywordKind::Return},
        {"static", KeywordKind::Static},
        {"this", KeywordKind::This},
        {"throw", KeywordKind::Throw},
        {"try", KeywordKind::Try},
        {"var", KeywordKind::Var},
        {"virtual", KeywordKind::Virtual},
        {"while", KeywordKind::While},
    };

    constexpr BuiltinTypeEntry BuiltinTypes[] = {
        {"i8", BuiltinTypeKind::I8},
        {"i16", BuiltinTypeKind::I16},
        {"i32", BuiltinTypeKind::I32},
        {"i64", BuiltinTypeKind::I64},
        {"i128", BuiltinTypeKind::I128},
        {"u8", BuiltinTypeKind::U8},
        {"u16", BuiltinTypeKind::U16},
        {"u32", BuiltinTypeKind::U32},
        {"u64", BuiltinTypeKind::U64},
        {"u128", BuiltinTypeKind::U128},
        {"int", BuiltinTypeKind::Int},
        {"uint", BuiltinTypeKind::UInt},
        {"ptrsize", BuiltinTypeKind::PtrSize},
        {"f16", BuiltinTypeKind::F16},
        {"f32", BuiltinTypeKind::F32},
        {"f64", BuiltinTypeKind::F64},
        {"bool", BuiltinTypeKind::Bool},
        {"byte", BuiltinTypeKind::Byte},
        {"void", BuiltinTypeKind::Void},
        {"never", BuiltinTypeKind::Never},
        {"type", BuiltinTypeKind::Type},
    };

    constexpr SuffixEntry Suffixes[] = {
        {"i8", NumericSuffix::I8, false},
        {"i16", NumericSuffix::I16, false},
        {"i32", NumericSuffix::I32, false},
        {"i64", NumericSuffix::I64, false},
        {"i128", NumericSuffix::I128, false},
        {"u8", NumericSuffix::U8, false},
        {"u16", NumericSuffix::U16, false},
        {"u32", NumericSuffix::U32, false},
        {"u64", NumericSuffix::U64, false},
        {"u128", NumericSuffix::U128, false},
        {"int", NumericSuffix::Int, false},
        {"uint", NumericSuffix::UInt, false},
        {"ptrsize", NumericSuffix::PtrSize, false},
        {"byte", NumericSuffix::Byte, false},
        {"f16", NumericSuffix::F16, true},
        {"f32", NumericSuffix::F32, true},
        {"f64", NumericSuffix::F64, true},
    };

    constexpr std::string_view Symbols = "(){}[],;:.@+-*/%=!&|^~<>";

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

    bool isHexDigit(char Value) noexcept
    {
      return digitValue(Value) >= 0;
    }

    bool isScalarValue(char32_t Value) noexcept
    {
      return Value <= 0x10FFFF && !(Value >= 0xD800 && Value <= 0xDFFF);
    }

    bool isForbiddenControl(char32_t Value) noexcept
    {
      return (Value <= 0x1F && Value != U'\t' && Value != U'\n' && Value != U'\r') || Value == 0x7F;
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
          if (startsWith(Source, 0, "\xEF\xBB\xBF"))
          {
            Tokens.push_back(makeToken(TokenKind::Utf8Bom, 0, 3));
            Position = 3;
          }
          while (Position < Source.size())
          {
            const std::size_t Before = Position;
            Token Token = scanToken();
            if (Position <= Before)
            {
              addDiagnostic<DiagnosticKind::InvalidCharacter>({Before, std::min(Before + 1, Source.size())});
              Position = std::min(Before + 1, Source.size());
              Token = makeToken(TokenKind::InvalidCharacter, Before, Position);
            }
            Tokens.push_back(std::move(Token));
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
            Position += std::max<std::size_t>(Decoded.Length, 1);
            addDiagnostic<DiagnosticKind::InvalidUtf8>({Start, Position});
            return makeToken(TokenKind::InvalidEncoding, Start, Position);
          }

          if (Decoded.Value == U'\r')
          {
            if (startsWith(Source, Position, "\r\n"))
            {
              Position += 2;
              return makeToken(TokenKind::LineBreak, Start, Position);
            }
            Position += Decoded.Length;
            addDiagnostic<DiagnosticKind::LoneCarriageReturn>({Start, Position});
            return makeToken(TokenKind::InvalidCharacter, Start, Position);
          }
          if (Decoded.Value == U'\n')
          {
            ++Position;
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
          if (Decoded.Value == 0xFEFF)
          {
            Position += Decoded.Length;
            addDiagnostic<DiagnosticKind::UnexpectedBom>({Start, Position});
            return makeToken(TokenKind::InvalidCharacter, Start, Position);
          }
          if (isForbiddenControl(Decoded.Value))
          {
            Position += Decoded.Length;
            addDiagnostic<DiagnosticKind::ForbiddenControlCharacter>({Start, Position});
            return makeToken(TokenKind::InvalidCharacter, Start, Position);
          }
          if (unicode::isUnicodeWhitespace(Decoded.Value))
          {
            Position += Decoded.Length;
            addDiagnostic<DiagnosticKind::NonAsciiWhitespace>({Start, Position});
            return makeToken(TokenKind::InvalidCharacter, Start, Position);
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
            return scanMultilineString(true);
          }
          if (startsWith(Source, Position, "r\""))
          {
            return scanSingleLineString(true);
          }
          if (startsWith(Source, Position, "\"\"\""))
          {
            return scanMultilineString(false);
          }
          if (Decoded.Value == U'"')
          {
            return scanSingleLineString(false);
          }
          if (Decoded.Value == U'\'')
          {
            return scanScalarLiteral();
          }
          if (Decoded.Value >= U'0' && Decoded.Value <= U'9')
          {
            return scanNumber();
          }
          if (Decoded.Value == U'_' || unicode::isXidStart(Decoded.Value))
          {
            return scanIdentifier();
          }
          if (Decoded.Value <= 0x7F && Symbols.find(static_cast<char>(Decoded.Value)) != std::string_view::npos)
          {
            Position += Decoded.Length;
            return makeToken(TokenKind::Symbol, Start, Position, static_cast<char>(Decoded.Value));
          }

          Position += Decoded.Length;
          if (unicode::isDefaultIgnorable(Decoded.Value))
          {
            addInvisibleDiagnostic(Start, Decoded, previousVisibleScalar(Start), nextVisibleScalar(Position), false);
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
          while (Position < Source.size() && Source[Position] != '\n' && !(Source[Position] == '\r' && Position + 1 < Source.size() && Source[Position + 1] == '\n'))
          {
            ++Position;
          }
          const TokenKind Validation = validateRawRange(Start, Position, true);
          return makeToken(Validation == TokenKind::Identifier ? TokenKind::LineComment : Validation, Start, Position);
        }

        Token scanBlockComment()
        {
          const std::size_t Start = Position;
          std::size_t Depth = 1;
          std::vector<std::size_t> OpeningPositions = {Start};
          const std::size_t TrackedDepthLimit = std::max<std::size_t>(Options.MaxBlockCommentDepth, 1);
          bool NestingLimitExceeded = Options.MaxBlockCommentDepth < 1;
          if (NestingLimitExceeded)
          {
            addDiagnostic<DiagnosticKind::BlockCommentNestingLimit>({Start, Start + 2});
          }
          Position += 2;
          while (Position < Source.size() && Depth != 0)
          {
            if (startsWith(Source, Position, "/*"))
            {
              ++Depth;
              if (Depth <= TrackedDepthLimit)
              {
                OpeningPositions.push_back(Position);
              }
              if (Depth > Options.MaxBlockCommentDepth && !NestingLimitExceeded)
              {
                NestingLimitExceeded = true;
                addDiagnostic<DiagnosticKind::BlockCommentNestingLimit>({Position, Position + 2});
              }
              Position += 2;
            }
            else if (startsWith(Source, Position, "*/"))
            {
              if (Depth <= TrackedDepthLimit)
              {
                OpeningPositions.pop_back();
              }
              --Depth;
              Position += 2;
            }
            else
            {
              ++Position;
            }
          }
          if (Depth != 0)
          {
            validateRawRange(Start, Position, true);
            auto Builder = makeDiagnosticBuilder<DiagnosticKind::UnterminatedBlockComment>({Start, std::min(Start + 2, Source.size())}, static_cast<std::uint64_t>(Depth));
            if (Depth <= TrackedDepthLimit)
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
          const TokenKind Validation = validateRawRange(Start, Position, true);
          if (NestingLimitExceeded && Validation == TokenKind::Identifier)
          {
            return makeToken(TokenKind::InvalidCharacter, Start, Position);
          }
          return makeToken(Validation == TokenKind::Identifier ? TokenKind::BlockComment : Validation, Start, Position);
        }

        Token scanIdentifier()
        {
          const std::size_t Start = Position;
          bool Invisible = false;
          std::size_t PreviousVisible = std::string::npos;
          while (Position < Source.size())
          {
            const DecodeResult Decoded = unicode::decode(Source, Position);
            if (!Decoded.Valid || (Decoded.Value != U'_' && !unicode::isXidContinue(Decoded.Value)))
            {
              break;
            }
            if (unicode::isDefaultIgnorable(Decoded.Value) || unicode::isUnicodeWhitespace(Decoded.Value))
            {
              Invisible = true;
              const std::size_t RunStart = Position;
              std::size_t RunEnd = Position;
              while (RunEnd < Source.size())
              {
                const DecodeResult Member = unicode::decode(Source, RunEnd);
                if (!Member.Valid || (Member.Value != U'_' && !unicode::isXidContinue(Member.Value)) || (!unicode::isDefaultIgnorable(Member.Value) && !unicode::isUnicodeWhitespace(Member.Value)))
                {
                  break;
                }
                RunEnd += Member.Length;
              }
              const DecodeResult Next = unicode::decode(Source, RunEnd);
              const std::size_t NextVisible = RunEnd < Source.size() && Next.Valid && (Next.Value == U'_' || unicode::isXidContinue(Next.Value)) ? RunEnd : std::string::npos;
              for (std::size_t MemberOffset = RunStart; MemberOffset < RunEnd;)
              {
                const DecodeResult Member = unicode::decode(Source, MemberOffset);
                addInvisibleDiagnostic(MemberOffset, Member, PreviousVisible, NextVisible, true);
                MemberOffset += Member.Length;
              }
              Position = RunEnd;
              continue;
            }
            else
            {
              PreviousVisible = Position;
            }
            Position += Decoded.Length;
          }
          const std::string_view Spelling(Source.data() + Start, Position - Start);
          if (Invisible)
          {
            return makeToken(TokenKind::InvalidIdentifier, Start, Position);
          }
          const unicode::NfcCheckResult NfcResult = unicode::checkNfc(Spelling);
          if (NfcResult == unicode::NfcCheckResult::Failed)
          {
            addDiagnostic<DiagnosticKind::InvalidUtf8>({Start, Position});
            return makeToken(TokenKind::InvalidEncoding, Start, Position);
          }
          if (NfcResult == unicode::NfcCheckResult::NotNormalized)
          {
            addDiagnostic<DiagnosticKind::IdentifierNotNfc>({Start, Position});
            return makeToken(TokenKind::InvalidIdentifier, Start, Position);
          }
          for (const KeywordEntry &Entry : Keywords)
          {
            if (Entry.Spelling == Spelling)
            {
              return makeToken(TokenKind::Keyword, Start, Position, Entry.Kind);
            }
          }
          if (Spelling == "true")
          {
            return makeToken(TokenKind::BoolLiteral, Start, Position, true);
          }
          if (Spelling == "false")
          {
            return makeToken(TokenKind::BoolLiteral, Start, Position, false);
          }
          if (Spelling == "null")
          {
            return makeToken(TokenKind::NullLiteral, Start, Position);
          }
          for (const BuiltinTypeEntry &Entry : BuiltinTypes)
          {
            if (Entry.Spelling == Spelling)
            {
              return makeToken(TokenKind::BuiltinType, Start, Position, Entry.Kind);
            }
          }
          return makeToken(TokenKind::Identifier, Start, Position);
        }

        Token scanNumber()
        {
          const std::size_t Start = Position;
          unsigned Base = 10;
          bool ExplicitBase = false;
          bool HasFraction = false;
          bool HasExponent = false;
          bool Invalid = false;
          NumericSuffix Suffix = NumericSuffix::None;

          if (Position + 1 < Source.size() && Source[Position] == '0' && (Source[Position + 1] == 'b' || Source[Position + 1] == 'o' || Source[Position + 1] == 'x'))
          {
            ExplicitBase = true;
            Base = Source[Position + 1] == 'b' ? 2U : Source[Position + 1] == 'o' ? 8U
                                                                                  : 16U;
            Position += 2;
          }

          bool SawDigit = false;
          bool PreviousDigit = false;
          while (Position < Source.size())
          {
            const char Current = Source[Position];
            const int Value = digitValue(Current);
            if (Value >= 0 && (isAsciiDigit(Current) || Base == 16))
            {
              SawDigit = true;
              if (static_cast<unsigned>(Value) >= Base)
              {
                Invalid = true;
                addDiagnostic<DiagnosticKind::DigitOutOfRange>({Position, Position + 1});
              }
              PreviousDigit = static_cast<unsigned>(Value) < Base;
              ++Position;
              continue;
            }
            if (Current == '_')
            {
              const bool NextIsDigit = Position + 1 < Source.size() && digitValue(Source[Position + 1]) >= 0 && static_cast<unsigned>(digitValue(Source[Position + 1])) < Base;
              if (!PreviousDigit || !NextIsDigit)
              {
                Invalid = true;
                addDiagnostic<DiagnosticKind::MisplacedNumericSeparator>({Position, Position + 1});
              }
              PreviousDigit = false;
              ++Position;
              continue;
            }
            break;
          }

          if (ExplicitBase && !SawDigit)
          {
            Invalid = true;
            addDiagnostic<DiagnosticKind::MissingBaseDigits>({Start, Position});
          }

          if (!ExplicitBase && Position < Source.size() && Source[Position] == '.' && Position + 1 < Source.size() && isAsciiDigit(Source[Position + 1]))
          {
            HasFraction = true;
            ++Position;
            scanDecimalComponent(Invalid);
          }
          else if (!ExplicitBase && Position + 2 < Source.size() && Source[Position] == '.' && Source[Position + 1] == '_' && isAsciiDigit(Source[Position + 2]))
          {
            HasFraction = true;
            ++Position;
            scanDecimalComponent(Invalid);
          }
          else if (ExplicitBase && Position < Source.size() && Source[Position] == '.' && Position + 1 < Source.size() && (isAsciiDigit(Source[Position + 1]) || (Base == 16 && isHexDigit(Source[Position + 1]))))
          {
            Invalid = true;
            addDiagnostic<DiagnosticKind::UnsupportedNonDecimalFloat>({Position, Position + 1});
            ++Position;
            while (Position < Source.size() && (isAsciiDigit(Source[Position]) || (Base == 16 && isHexDigit(Source[Position])) || Source[Position] == '_'))
            {
              ++Position;
            }
          }

          if (!ExplicitBase && Position < Source.size() && (Source[Position] == 'e' || Source[Position] == 'E'))
          {
            HasExponent = true;
            const std::size_t ExponentStart = Position++;
            if (Position < Source.size() && (Source[Position] == '+' || Source[Position] == '-'))
            {
              ++Position;
            }
            const std::size_t DigitsStart = Position;
            const bool ExponentHasDigits = scanDecimalComponent(Invalid);
            if (!ExponentHasDigits)
            {
              Invalid = true;
              addDiagnostic<DiagnosticKind::MissingExponentDigits>({ExponentStart, std::max(Position, DigitsStart)});
            }
          }

          const std::size_t SuffixStart = Position;
          if (Position < Source.size())
          {
            const DecodeResult Decoded = unicode::decode(Source, Position);
            if (Decoded.Valid && (Decoded.Value == U'_' || unicode::isXidStart(Decoded.Value)))
            {
              if (unicode::isDefaultIgnorable(Decoded.Value))
              {
                Invalid = true;
                addDiagnostic<DiagnosticKind::InvisibleCharacter>({Position, Position + Decoded.Length});
              }
              Position += Decoded.Length;
              while (Position < Source.size())
              {
                const DecodeResult Continued = unicode::decode(Source, Position);
                if (!Continued.Valid || (Continued.Value != U'_' && !unicode::isXidContinue(Continued.Value)))
                {
                  break;
                }
                if (unicode::isDefaultIgnorable(Continued.Value))
                {
                  Invalid = true;
                  addDiagnostic<DiagnosticKind::InvisibleCharacter>({Position, Position + Continued.Length});
                }
                Position += Continued.Length;
              }
              const std::string_view Spelling(Source.data() + SuffixStart, Position - SuffixStart);
              const SuffixEntry *Entry = nullptr;
              for (const SuffixEntry &Candidate : Suffixes)
              {
                if (Candidate.Spelling == Spelling)
                {
                  Entry = &Candidate;
                  break;
                }
              }
              if (!Invalid)
              {
                if (Entry == nullptr)
                {
                  Invalid = true;
                  const bool LooksLikeNonDecimalExponent = ExplicitBase && !Spelling.empty() && (Spelling.front() == 'e' || Spelling.front() == 'E' || Spelling.front() == 'p' || Spelling.front() == 'P');
                  if (LooksLikeNonDecimalExponent)
                  {
                    addDiagnostic<DiagnosticKind::UnsupportedNonDecimalFloat>({SuffixStart, Position});
                  }
                  else
                  {
                    addDiagnostic<DiagnosticKind::UnknownNumericSuffix>({SuffixStart, Position});
                  }
                }
                else if (ExplicitBase && Entry->Floating)
                {
                  Invalid = true;
                  addDiagnostic<DiagnosticKind::InvalidNumericSuffix>({SuffixStart, Position});
                }
                else if ((HasFraction || HasExponent) && !Entry->Floating)
                {
                  Invalid = true;
                  addDiagnostic<DiagnosticKind::InvalidNumericSuffix>({SuffixStart, Position});
                }
                else
                {
                  Suffix = Entry->Suffix;
                  if (Entry->Floating)
                  {
                    HasFraction = true;
                  }
                }
              }
            }
          }

          if (Invalid)
          {
            return makeToken(TokenKind::InvalidNumber, Start, Position);
          }
          const TokenKind Kind = HasFraction || HasExponent ? TokenKind::FloatLiteral : TokenKind::IntegerLiteral;
          return makeToken(Kind, Start, Position, NumericInfo{Base, Suffix});
        }

        bool scanDecimalComponent(bool &Invalid)
        {
          bool SawDigit = false;
          bool PreviousDigit = false;
          while (Position < Source.size())
          {
            if (isAsciiDigit(Source[Position]))
            {
              SawDigit = true;
              PreviousDigit = true;
              ++Position;
            }
            else if (Source[Position] == '_')
            {
              const bool NextIsDigit = Position + 1 < Source.size() && isAsciiDigit(Source[Position + 1]);
              if (!PreviousDigit || !NextIsDigit)
              {
                Invalid = true;
                addDiagnostic<DiagnosticKind::MisplacedNumericSeparator>({Position, Position + 1});
              }
              PreviousDigit = false;
              ++Position;
            }
            else
            {
              break;
            }
          }
          return SawDigit;
        }

        Token scanScalarLiteral()
        {
          const std::size_t Start = Position++;
          char32_t FirstValue = 0;
          unsigned ValueCount = 0;
          bool HasBody = false;
          bool Invalid = false;
          bool Closed = false;
          while (Position < Source.size())
          {
            if (Source[Position] == '\'')
            {
              ++Position;
              Closed = true;
              break;
            }
            if (Source[Position] == '\n' || (Source[Position] == '\r' && Position + 1 < Source.size() && Source[Position + 1] == '\n'))
            {
              Invalid = true;
              addDiagnostic<DiagnosticKind::UnterminatedScalarLiteral>({Start, Position});
              break;
            }
            if (Source[Position] == '\\')
            {
              HasBody = true;
              char32_t Value = 0;
              bool Produced = false;
              if (!scanEscape(Position, Source.size(), Value, Produced))
              {
                Invalid = true;
              }
              if (Produced)
              {
                if (ValueCount == 0)
                {
                  FirstValue = Value;
                }
                ValueCount = std::min(ValueCount + 1, 2U);
              }
              continue;
            }
            HasBody = true;
            const DecodeResult Decoded = unicode::decode(Source, Position);
            if (!Decoded.Valid)
            {
              Invalid = true;
              addDiagnostic<DiagnosticKind::InvalidUtf8>({Position, Position + std::max<std::size_t>(Decoded.Length, 1)});
              Position += std::max<std::size_t>(Decoded.Length, 1);
              continue;
            }
            if (isForbiddenControl(Decoded.Value) || Decoded.Value == U'\r')
            {
              Invalid = true;
              if (Decoded.Value == U'\r')
              {
                addDiagnostic<DiagnosticKind::LoneCarriageReturn>({Position, Position + Decoded.Length});
              }
              else
              {
                addDiagnostic<DiagnosticKind::ForbiddenControlCharacter>({Position, Position + Decoded.Length});
              }
            }
            if (unicode::isDefaultIgnorable(Decoded.Value))
            {
              Invalid = true;
              addDiagnostic<DiagnosticKind::InvisibleCharacter>({Position, Position + Decoded.Length});
            }
            if (ValueCount == 0)
            {
              FirstValue = Decoded.Value;
            }
            ValueCount = std::min(ValueCount + 1, 2U);
            Position += Decoded.Length;
          }
          if (!Closed && Position == Source.size())
          {
            Invalid = true;
            addDiagnostic<DiagnosticKind::UnterminatedScalarLiteral>({Start, Position});
          }
          if (!HasBody && Closed)
          {
            Invalid = true;
            addDiagnostic<DiagnosticKind::EmptyScalarLiteral>({Start, Position});
          }
          else if (ValueCount > 1)
          {
            Invalid = true;
            addDiagnostic<DiagnosticKind::MultipleScalarValues>({Start, Position});
          }
          if (Invalid || !Closed || ValueCount != 1)
          {
            return makeToken(TokenKind::InvalidScalarLiteral, Start, Position);
          }
          return makeToken(TokenKind::ScalarLiteral, Start, Position, FirstValue);
        }

        Token scanSingleLineString(bool RawMode)
        {
          const std::size_t Start = Position;
          Position += RawMode ? 2 : 1;
          std::string DecodedValue;
          bool Invalid = false;
          bool Closed = false;
          while (Position < Source.size())
          {
            if (Source[Position] == '"')
            {
              ++Position;
              Closed = true;
              break;
            }
            if (Source[Position] == '\n' || (Source[Position] == '\r' && Position + 1 < Source.size() && Source[Position + 1] == '\n'))
            {
              Invalid = true;
              addDiagnostic<DiagnosticKind::UnterminatedStringLiteral>({Start, Position});
              break;
            }
            if (!RawMode && Source[Position] == '\\')
            {
              char32_t Value = 0;
              bool Produced = false;
              if (!scanEscape(Position, Source.size(), Value, Produced))
              {
                Invalid = true;
              }
              if (Produced)
              {
                unicode::appendUtf8(DecodedValue, Value);
              }
              continue;
            }
            if (!scanLiteralScalar(Position, Source.size(), DecodedValue))
            {
              Invalid = true;
            }
          }
          if (!Closed && Position == Source.size())
          {
            Invalid = true;
            addDiagnostic<DiagnosticKind::UnterminatedStringLiteral>({Start, Position});
          }
          if (Invalid || !Closed)
          {
            return makeToken(TokenKind::InvalidStringLiteral, Start, Position);
          }
          const StringMode Mode = RawMode ? StringMode::RawSingleLine : StringMode::EscapedSingleLine;
          return makeToken(TokenKind::StringLiteral, Start, Position, StringInfo{Mode, std::move(DecodedValue)});
        }

        Token scanMultilineString(bool RawMode)
        {
          const std::size_t Start = Position;
          Position += RawMode ? 4 : 3;
          std::size_t OpeningLineBreakLength = logicalLineBreakLength(Position);
          if (OpeningLineBreakLength == 0)
          {
            addDiagnostic<DiagnosticKind::MultilineOpeningLineBreakRequired>({Start, Position});
            std::size_t Closing = std::string::npos;
            for (std::size_t LineStart = Position; LineStart < Source.size();)
            {
              const std::size_t LineBreak = Source.find('\n', LineStart);
              if (LineBreak == std::string::npos)
              {
                break;
              }
              std::size_t Candidate = LineBreak + 1;
              while (Candidate < Source.size() && (Source[Candidate] == ' ' || Source[Candidate] == '\t'))
              {
                ++Candidate;
              }
              if (startsWith(Source, Candidate, "\"\"\""))
              {
                Closing = Candidate;
                break;
              }
              LineStart = LineBreak + 1;
            }
            if (Closing == std::string::npos)
            {
              Closing = Source.find("\"\"\"", Position);
            }
            if (Closing == std::string::npos)
            {
              Position = Source.size();
              validateRawRange(Start, Position, false);
              addDiagnostic<DiagnosticKind::UnterminatedMultilineStringLiteral>({Start, Position});
              return makeToken(TokenKind::InvalidStringLiteral, Start, Position);
            }
            Position = Closing + 3;
            validateRawRange(Start, Position, false);
            return makeToken(TokenKind::InvalidStringLiteral, Start, Position);
          }
          Position += OpeningLineBreakLength;

          const std::size_t BodyStart = Position;
          std::size_t ClosingStart = std::string::npos;
          std::size_t ClosingLineStart = std::string::npos;
          while (Position < Source.size())
          {
            const std::size_t LineStart = Position;
            std::size_t LineEnd = Position;
            while (LineEnd < Source.size() && Source[LineEnd] != '\n' && !(Source[LineEnd] == '\r' && LineEnd + 1 < Source.size() && Source[LineEnd + 1] == '\n'))
            {
              ++LineEnd;
            }
            std::size_t FirstContent = LineStart;
            while (FirstContent < LineEnd && (Source[FirstContent] == ' ' || Source[FirstContent] == '\t'))
            {
              ++FirstContent;
            }
            if (startsWith(Source, FirstContent, "\"\"\""))
            {
              ClosingStart = FirstContent;
              ClosingLineStart = LineStart;
              Position = FirstContent + 3;
              break;
            }
            if (LineEnd == Source.size())
            {
              Position = LineEnd;
              break;
            }
            Position = LineEnd + logicalLineBreakLength(LineEnd);
          }

          if (ClosingStart == std::string::npos)
          {
            Position = Source.size();
            validateRawRange(Start, Position, false);
            addDiagnostic<DiagnosticKind::UnterminatedMultilineStringLiteral>({Start, Position});
            return makeToken(TokenKind::InvalidStringLiteral, Start, Position);
          }

          const std::string_view Indentation(Source.data() + ClosingLineStart, ClosingStart - ClosingLineStart);
          std::string DecodedValue;
          bool Invalid = false;
          for (std::size_t LineStart = BodyStart; LineStart < ClosingLineStart;)
          {
            std::size_t LineEnd = LineStart;
            while (LineEnd < ClosingLineStart && Source[LineEnd] != '\n' && !(Source[LineEnd] == '\r' && LineEnd + 1 < ClosingLineStart && Source[LineEnd + 1] == '\n'))
            {
              ++LineEnd;
            }
            std::size_t ContentStart = LineStart;
            const std::string_view RawLine(Source.data() + LineStart, LineEnd - LineStart);
            const bool WhitespaceOnly = std::all_of(RawLine.begin(), RawLine.end(), [](char Value)
                                                    {
                                                      return Value == ' ' || Value == '\t';
                                                    });
            if (RawLine.size() >= Indentation.size() && RawLine.substr(0, Indentation.size()) == Indentation)
            {
              ContentStart += Indentation.size();
            }
            else if (WhitespaceOnly && Indentation.size() >= RawLine.size() && Indentation.substr(0, RawLine.size()) == RawLine)
            {
              ContentStart = LineEnd;
            }
            else
            {
              Invalid = true;
              addDiagnostic<DiagnosticKind::InvalidMultilineIndentation>({LineStart, LineEnd});
            }

            std::size_t Cursor = ContentStart;
            while (Cursor < LineEnd)
            {
              if (!RawMode && Source[Cursor] == '\\')
              {
                char32_t Value = 0;
                bool Produced = false;
                if (!scanEscape(Cursor, LineEnd, Value, Produced))
                {
                  Invalid = true;
                }
                if (Produced)
                {
                  unicode::appendUtf8(DecodedValue, Value);
                }
              }
              else if (!scanLiteralScalar(Cursor, LineEnd, DecodedValue))
              {
                Invalid = true;
              }
            }
            LineStart = LineEnd + logicalLineBreakLength(LineEnd);
            if (LineStart < ClosingLineStart)
            {
              DecodedValue.push_back('\n');
            }
          }

          if (Invalid)
          {
            return makeToken(TokenKind::InvalidStringLiteral, Start, Position);
          }
          const StringMode Mode = RawMode ? StringMode::RawMultiline : StringMode::EscapedMultiline;
          return makeToken(TokenKind::StringLiteral, Start, Position, StringInfo{Mode, std::move(DecodedValue)});
        }

        bool scanEscape(std::size_t &Cursor, std::size_t Limit, char32_t &Value, bool &Produced)
        {
          const std::size_t Start = Cursor;
          Produced = false;
          if (Cursor + 1 >= Limit)
          {
            Cursor = Limit;
            addDiagnostic<DiagnosticKind::UnknownEscape>({Start, Cursor});
            return false;
          }
          const DecodeResult EscapedCharacter = unicode::decode(Source, Cursor + 1);
          if (!EscapedCharacter.Valid)
          {
            const std::size_t InvalidLength = std::max<std::size_t>(EscapedCharacter.Length, 1);
            Cursor = std::min(Cursor + 1 + InvalidLength, Limit);
            addDiagnostic<DiagnosticKind::InvalidUtf8>({Start + 1, Cursor});
            return false;
          }
          if (isForbiddenControl(EscapedCharacter.Value))
          {
            addDiagnostic<DiagnosticKind::ForbiddenControlCharacter>({Cursor + 1, Cursor + 1 + EscapedCharacter.Length});
          }
          if (unicode::isDefaultIgnorable(EscapedCharacter.Value))
          {
            addDiagnostic<DiagnosticKind::InvisibleCharacter>({Cursor + 1, Cursor + 1 + EscapedCharacter.Length});
          }
          const char Kind = Source[Cursor + 1];
          if (Kind == '\n' || Kind == '\r')
          {
            Cursor = Start + 1;
            addDiagnostic<DiagnosticKind::UnknownEscape>({Start, Cursor});
            return false;
          }
          switch (Kind)
          {
          case '\\':
            Value = U'\\';
            Cursor += 2;
            Produced = true;
            return true;
          case '\'':
            Value = U'\'';
            Cursor += 2;
            Produced = true;
            return true;
          case '"':
            Value = U'"';
            Cursor += 2;
            Produced = true;
            return true;
          case '0':
            Value = U'\0';
            Cursor += 2;
            Produced = true;
            return true;
          case 'n':
            Value = U'\n';
            Cursor += 2;
            Produced = true;
            return true;
          case 'r':
            Value = U'\r';
            Cursor += 2;
            Produced = true;
            return true;
          case 't':
            Value = U'\t';
            Cursor += 2;
            Produced = true;
            return true;
          case 'x':
          {
            if (Cursor + 3 >= Limit || !isHexDigit(Source[Cursor + 2]) || !isHexDigit(Source[Cursor + 3]))
            {
              Cursor += 2;
              while (Cursor < Limit && Cursor < Start + 4 && isHexDigit(Source[Cursor]))
              {
                ++Cursor;
              }
              addDiagnostic<DiagnosticKind::InvalidHexEscape>({Start, Cursor});
              return false;
            }
            Value = static_cast<char32_t>(digitValue(Source[Cursor + 2]) * 16 + digitValue(Source[Cursor + 3]));
            Cursor += 4;
            Produced = true;
            return true;
          }
          case 'u':
          {
            if (Cursor + 2 >= Limit || Source[Cursor + 2] != '{')
            {
              Cursor += 2;
              addDiagnostic<DiagnosticKind::InvalidUnicodeEscape>({Start, Cursor});
              return false;
            }
            std::size_t Index = Cursor + 3;
            std::size_t DigitCount = 0;
            char32_t Scalar = 0;
            bool ValidDigits = true;
            while (Index < Limit && Source[Index] != '}' && Source[Index] != '"' && Source[Index] != '\'' && Source[Index] != '\n' && Source[Index] != '\r' && Source[Index] != '\\')
            {
              const DecodeResult EscapedDigit = unicode::decode(Source, Index);
              if (!EscapedDigit.Valid)
              {
                const std::size_t InvalidLength = std::max<std::size_t>(EscapedDigit.Length, 1);
                addDiagnostic<DiagnosticKind::InvalidUtf8>({Index, std::min(Index + InvalidLength, Limit)});
                ValidDigits = false;
                ++DigitCount;
                Index = std::min(Index + InvalidLength, Limit);
                continue;
              }
              if (isForbiddenControl(EscapedDigit.Value) || EscapedDigit.Value == U'\r')
              {
                if (EscapedDigit.Value == U'\r')
                {
                  addDiagnostic<DiagnosticKind::LoneCarriageReturn>({Index, Index + EscapedDigit.Length});
                }
                else
                {
                  addDiagnostic<DiagnosticKind::ForbiddenControlCharacter>({Index, Index + EscapedDigit.Length});
                }
                ValidDigits = false;
              }
              if (unicode::isDefaultIgnorable(EscapedDigit.Value))
              {
                addDiagnostic<DiagnosticKind::InvisibleCharacter>({Index, Index + EscapedDigit.Length});
                ValidDigits = false;
              }
              const int Digit = digitValue(Source[Index]);
              if (Digit < 0)
              {
                ValidDigits = false;
              }
              else if (DigitCount < 7)
              {
                Scalar = static_cast<char32_t>((Scalar << 4U) | Digit);
              }
              ++DigitCount;
              Index += EscapedDigit.Length;
            }
            if (Index >= Limit || Source[Index] != '}')
            {
              Cursor = Index;
              addDiagnostic<DiagnosticKind::InvalidUnicodeEscape>({Start, Cursor});
              return false;
            }
            Cursor = Index + 1;
            if (!ValidDigits || DigitCount == 0 || DigitCount > 6)
            {
              addDiagnostic<DiagnosticKind::InvalidUnicodeEscape>({Start, Cursor});
              return false;
            }
            if (!isScalarValue(Scalar))
            {
              addDiagnostic<DiagnosticKind::InvalidUnicodeScalar>({Start, Cursor});
              return false;
            }
            Value = Scalar;
            Produced = true;
            return true;
          }
          default:
          {
            const DecodeResult Escaped = unicode::decode(Source, Cursor + 1);
            Cursor += 1 + std::max<std::size_t>(Escaped.Length, 1);
            addDiagnostic<DiagnosticKind::UnknownEscape>({Start, Cursor});
            return false;
          }
          }
        }

        bool scanLiteralScalar(std::size_t &Cursor, std::size_t Limit, std::string &DecodedValue)
        {
          const DecodeResult Decoded = unicode::decode(Source, Cursor);
          if (!Decoded.Valid || Cursor + Decoded.Length > Limit)
          {
            const std::size_t Length = std::max<std::size_t>(Decoded.Length, 1);
            addDiagnostic<DiagnosticKind::InvalidUtf8>({Cursor, std::min(Cursor + Length, Limit)});
            Cursor = std::min(Cursor + Length, Limit);
            return false;
          }
          bool Valid = true;
          if (isForbiddenControl(Decoded.Value) || Decoded.Value == U'\r')
          {
            Valid = false;
            if (Decoded.Value == U'\r')
            {
              addDiagnostic<DiagnosticKind::LoneCarriageReturn>({Cursor, Cursor + Decoded.Length});
            }
            else
            {
              addDiagnostic<DiagnosticKind::ForbiddenControlCharacter>({Cursor, Cursor + Decoded.Length});
            }
          }
          if (unicode::isDefaultIgnorable(Decoded.Value))
          {
            Valid = false;
            addDiagnostic<DiagnosticKind::InvisibleCharacter>({Cursor, Cursor + Decoded.Length});
          }
          unicode::appendUtf8(DecodedValue, Decoded.Value);
          Cursor += Decoded.Length;
          return Valid;
        }

        TokenKind validateRawRange(std::size_t Start, std::size_t End, bool AllowInvisible)
        {
          TokenKind Result = TokenKind::Identifier;
          for (std::size_t Cursor = Start; Cursor < End;)
          {
            const DecodeResult Decoded = unicode::decode(Source, Cursor);
            if (!Decoded.Valid)
            {
              const std::size_t Length = std::max<std::size_t>(Decoded.Length, 1);
              addDiagnostic<DiagnosticKind::InvalidUtf8>({Cursor, std::min(Cursor + Length, End)});
              Result = TokenKind::InvalidEncoding;
              Cursor = std::min(Cursor + Length, End);
              continue;
            }
            if (isForbiddenControl(Decoded.Value) || (Decoded.Value == U'\r' && !(Cursor + 1 < End && Source[Cursor + 1] == '\n')))
            {
              if (Decoded.Value == U'\r')
              {
                addDiagnostic<DiagnosticKind::LoneCarriageReturn>({Cursor, Cursor + Decoded.Length});
              }
              else
              {
                addDiagnostic<DiagnosticKind::ForbiddenControlCharacter>({Cursor, Cursor + Decoded.Length});
              }
              if (Result != TokenKind::InvalidEncoding)
              {
                Result = TokenKind::InvalidCharacter;
              }
            }
            if (!AllowInvisible && unicode::isDefaultIgnorable(Decoded.Value))
            {
              addDiagnostic<DiagnosticKind::InvisibleCharacter>({Cursor, Cursor + Decoded.Length});
              if (Result != TokenKind::InvalidEncoding)
              {
                Result = TokenKind::InvalidCharacter;
              }
            }
            Cursor += Decoded.Length;
          }
          return Result;
        }

        std::size_t logicalLineBreakLength(std::size_t Offset) const noexcept
        {
          if (Offset < Source.size() && Source[Offset] == '\n')
          {
            return 1;
          }
          if (Offset + 1 < Source.size() && Source[Offset] == '\r' && Source[Offset + 1] == '\n')
          {
            return 2;
          }
          return 0;
        }

        template <DiagnosticKind Kind>
        void addDiagnostic(core::SourceRange Span)
        {
          Diagnostics.push_back(core::makeDiagnostic<Kind>(Span));
        }

        std::size_t previousVisibleScalar(std::size_t Offset) const
        {
          if (Offset == 0)
          {
            return std::string::npos;
          }
          std::size_t Candidate = Offset - 1;
          while (Candidate != 0 && (static_cast<unsigned char>(Source[Candidate]) & 0xC0U) == 0x80U)
          {
            --Candidate;
          }
          const DecodeResult Decoded = unicode::decode(Source, Candidate);
          return Decoded.Valid && Candidate + Decoded.Length == Offset && !unicode::isDefaultIgnorable(Decoded.Value) && !isForbiddenControl(Decoded.Value) && Decoded.Value != U' ' && Decoded.Value != U'\t' && Decoded.Value != U'\r' && Decoded.Value != U'\n' && !unicode::isUnicodeWhitespace(Decoded.Value) ? Candidate : std::string::npos;
        }

        std::size_t nextVisibleScalar(std::size_t Offset) const
        {
          const DecodeResult Decoded = unicode::decode(Source, Offset);
          return Offset < Source.size() && Decoded.Valid && !unicode::isDefaultIgnorable(Decoded.Value) && !isForbiddenControl(Decoded.Value) && Decoded.Value != U' ' && Decoded.Value != U'\t' && Decoded.Value != U'\r' && Decoded.Value != U'\n' && !unicode::isUnicodeWhitespace(Decoded.Value) ? Offset : std::string::npos;
        }

        void addInvisibleDiagnostic(std::size_t Offset, DecodeResult Decoded, std::size_t PreviousVisible, std::size_t NextVisible, bool IdentifierContext)
        {
          auto Builder = makeDiagnosticBuilder<DiagnosticKind::InvisibleCharacterInContext>({Offset, Offset + Decoded.Length}, Decoded.Value, IdentifierContext ? DiagnosticSourceContext::Identifier : DiagnosticSourceContext::SourceText);
          if (PreviousVisible != std::string::npos)
          {
            const DecodeResult Previous = unicode::decode(Source, PreviousVisible);
            Builder.related(DiagnosticRelatedKind::PreviousVisibleCharacter, {PreviousVisible, PreviousVisible + Previous.Length}, {{DiagnosticArgumentName::Character, Previous.Value}});
          }
          if (NextVisible != std::string::npos)
          {
            const DecodeResult Next = unicode::decode(Source, NextVisible);
            Builder.related(DiagnosticRelatedKind::NextVisibleCharacter, {NextVisible, NextVisible + Next.Length}, {{DiagnosticArgumentName::Character, Next.Value}});
          }
          Diagnostics.push_back(std::move(Builder).build());
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

  std::string_view TokenizedBuffer::raw(const Token &Token) const noexcept
  {
    if (Token.Span.Start > Token.Span.End || Token.Span.End > Source.size())
    {
      return {};
    }
    return std::string_view(Source.data() + Token.Span.Start, Token.Span.size());
  }

  std::size_t TokenizedBuffer::lineNumber(std::size_t ByteOffset) const noexcept
  {
    const std::size_t ClampedOffset = std::min(ByteOffset, Source.size());
    return static_cast<std::size_t>(std::upper_bound(LineStarts.begin(), LineStarts.end(), ClampedOffset) - LineStarts.begin());
  }

  bool TokenizedBuffer::succeeded() const noexcept
  {
    return Succeeded;
  }

  Tokenizer::Tokenizer(core::FrontendContext &Context, TokenizerOptions Options)
      : Context(Context),
        Options(Options)
  {
  }

  TokenizedBuffer Tokenizer::tokenize(std::string Source) const
  {
    TokenizedBuffer Result;
    Result.SourceId = Context.compilationContext().createSourceId();
    Result.Source = std::move(Source);
    Result.LineStarts.push_back(0);
    for (std::size_t Index = 0; Index < Result.Source.size(); ++Index)
    {
      if (Result.Source[Index] == '\n')
      {
        Result.LineStarts.push_back(Index + 1);
      }
    }
    std::vector<Diagnostic> Diagnostics;
    Scanner Scanner(Result.Source, Result.Tokens, Diagnostics, Options);
    Scanner.run();
    Result.Succeeded = Diagnostics.empty() && std::none_of(Result.Tokens.begin(), Result.Tokens.end(), [](const Token &Token)
                                                           {
                                                             return Token.isError();
                                                           });
    for (Diagnostic &DiagnosticEntry : Diagnostics)
    {
      DiagnosticEntry.Source = Result.SourceId;
      Context.diagnosticEngine().report(DiagnosticEntry);
    }
    return Result;
  }

  TokenizedBuffer tokenize(core::FrontendContext &Context, std::string Source, TokenizerOptions Options)
  {
    return Tokenizer(Context, Options).tokenize(std::move(Source));
  }

} // namespace ink::tokenizer
