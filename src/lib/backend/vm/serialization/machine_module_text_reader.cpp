#include "ink/backend/vm/serialization/machine_module_text.h"

#include "machine_module_text_format.h"

#include "ink/backend/vm/instruction/register.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/backend/vm/module/machine_module.h"

#include <array>
#include <bit>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ink::vm
{
    namespace
    {
        template <typename ValueType = void>
        using ParseResult = std::expected<ValueType, core::Diagnostic>;

        enum class TokenKind : std::uint8_t
        {
            Identifier,
            Number,
            String,
            Colon,
            Comma,
            LeftBracket,
            RightBracket,
            Plus,
            Minus,
            At,
            EndOfLine,
            EndOfFile,
        };

        struct Token
        {
                TokenKind Kind = TokenKind::EndOfFile;
                std::string_view Spelling;
                std::string DecodedString;
                std::size_t BeginOffset = 0;
                std::size_t EndOffset = 0;

                std::string_view text() const noexcept
                {
                    return Kind == TokenKind::String ? std::string_view(DecodedString) : Spelling;
                }
        };

        constexpr bool isAsciiLetter(char Character) noexcept
        {
            return (Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z');
        }

        constexpr bool isNumberBodyChar(char Character) noexcept
        {
            return detail::isDecimalDigit(Character) || isAsciiLetter(Character) || Character == '.' || Character == '_';
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
            case '@':
                return TokenKind::At;
            default:
                return std::nullopt;
            }
        }

        // Single-pass streaming lexer producing exact token byte ranges without per-line vector allocations.
        class Lexer
        {
            public:
                explicit Lexer(std::string_view Text) noexcept
                    : Text(Text)
                {
                }

                ParseResult<Token> nextToken()
                {
                    for (;;)
                    {
                        while (Offset < Text.size() && (Text[Offset] == ' ' || Text[Offset] == '\t'))
                        {
                            ++Offset;
                        }
                        if (Offset >= Text.size())
                        {
                            return Token{TokenKind::EndOfFile, {}, {}, Offset, Offset};
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
                            return Token{TokenKind::EndOfLine, Text.substr(Start, Offset - Start), {}, Start, Offset};
                        }
                        if (Character == '\n')
                        {
                            const std::size_t Start = Offset++;
                            return Token{TokenKind::EndOfLine, Text.substr(Start, 1), {}, Start, Offset};
                        }
                        const std::size_t Start = Offset++;
                        if (Character == '"')
                        {
                            auto Decoded = detail::decodeQuotedText(Text, Start, Offset);
                            if (!Decoded)
                            {
                                return std::unexpected(Decoded.error());
                            }
                            return Token{TokenKind::String, Text.substr(Start, Offset - Start), std::move(*Decoded), Start, Offset};
                        }
                        if (detail::isDecimalDigit(Character))
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
                            return Token{TokenKind::Number, Text.substr(Start, Offset - Start), {}, Start, Offset};
                        }
                        if (detail::isIdentifierStart(Character))
                        {
                            while (Offset < Text.size() && detail::isIdentifierContinue(Text[Offset]))
                            {
                                ++Offset;
                            }
                            return Token{TokenKind::Identifier, Text.substr(Start, Offset - Start), {}, Start, Offset};
                        }
                        if (const auto Punctuation = matchPunctuation(Character))
                        {
                            return Token{*Punctuation, Text.substr(Start, 1), {}, Start, Offset};
                        }
                        return std::unexpected(detail::makeTextDiagnostic(Start, Offset, "unexpected character"));
                    }
                }

                std::size_t endOffset() const noexcept
                {
                    return Text.size();
                }

            private:
                std::string_view Text;
                std::size_t Offset = 0;
        };

        struct PendingInstruction
        {
                MachineBasicBlock* Block = nullptr;
                const detail::OpcodeDescriptor* Descriptor = nullptr;
                std::array<detail::ParsedOperand, 3> Operands{};
                std::string_view TargetLabel;
                std::size_t BeginOffset = 0;
                std::size_t EndOffset = 0;
                std::size_t TargetBeginOffset = 0;
                std::size_t TargetEndOffset = 0;
                std::size_t TargetOperandIndex = 0;
        };

        struct FunctionParseState
        {
                MachineFunction& Function;
                MachineBasicBlock* CurrentBlock = nullptr;
                std::unordered_map<std::string_view, MachineBasicBlock*> BlockLabels;
                std::vector<PendingInstruction> PendingInstructions;
                bool HasFrame = false;
                bool HasBlock = false;
        };

        class ModuleTextReader
        {
            public:
                explicit ModuleTextReader(std::string_view Text) noexcept
                    : LexerState(Text)
                {
                }

                ParseResult<std::unique_ptr<MachineModule>> parseModule()
                {
                    if (const auto Skipped = skipBlankLines(); !Skipped)
                    {
                        return std::unexpected(Skipped.error());
                    }
                    const auto First = peek();
                    if (!First)
                    {
                        return std::unexpected(First.error());
                    }
                    if ((*First)->Kind == TokenKind::EndOfFile)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(LexerState.endOffset(), "missing .module directive"));
                    }
                    if (const auto HeaderResult = parseModuleHeader(); !HeaderResult)
                    {
                        return std::unexpected(HeaderResult.error());
                    }
                    for (;;)
                    {
                        if (const auto Skipped = skipBlankLines(); !Skipped)
                        {
                            return std::unexpected(Skipped.error());
                        }
                        const auto Next = peek();
                        if (!Next)
                        {
                            return std::unexpected(Next.error());
                        }
                        if ((*Next)->Kind == TokenKind::EndOfFile)
                        {
                            break;
                        }
                        if (const auto LineResult = parseModuleTopLevelLine(); !LineResult)
                        {
                            return std::unexpected(LineResult.error());
                        }
                    }
                    return std::move(Module);
                }

            private:
                ParseResult<const Token*> peek()
                {
                    if (!HasCurrent)
                    {
                        auto Next = LexerState.nextToken();
                        if (!Next)
                        {
                            return std::unexpected(Next.error());
                        }
                        Current = std::move(*Next);
                        HasCurrent = true;
                    }
                    return &Current;
                }

                ParseResult<Token> take()
                {
                    const auto Next = peek();
                    if (!Next)
                    {
                        return std::unexpected(Next.error());
                    }
                    LastTokenEnd = Current.EndOffset;
                    HasCurrent = false;
                    return std::move(Current);
                }

                ParseResult<bool> at(TokenKind Kind)
                {
                    const auto Next = peek();
                    if (!Next)
                    {
                        return std::unexpected(Next.error());
                    }
                    return (*Next)->Kind == Kind;
                }

                ParseResult<bool> atLineEnd()
                {
                    const auto Next = peek();
                    if (!Next)
                    {
                        return std::unexpected(Next.error());
                    }
                    return (*Next)->Kind == TokenKind::EndOfLine || (*Next)->Kind == TokenKind::EndOfFile;
                }

                ParseResult<std::optional<Token>> eat(TokenKind Kind)
                {
                    const auto Next = peek();
                    if (!Next)
                    {
                        return std::unexpected(Next.error());
                    }
                    if ((*Next)->Kind != Kind)
                    {
                        return std::nullopt;
                    }
                    LastTokenEnd = Current.EndOffset;
                    HasCurrent = false;
                    return std::move(Current);
                }

                ParseResult<Token> expect(TokenKind Kind, std::string_view Message)
                {
                    const auto Next = peek();
                    if (!Next)
                    {
                        return std::unexpected(Next.error());
                    }
                    if ((*Next)->Kind != Kind)
                    {
                        return std::unexpected(detail::makeTextDiagnostic((*Next)->BeginOffset, (*Next)->EndOffset, Message));
                    }
                    LastTokenEnd = Current.EndOffset;
                    HasCurrent = false;
                    return std::move(Current);
                }

                ParseResult<void> skipBlankLines()
                {
                    for (;;)
                    {
                        const auto Next = peek();
                        if (!Next)
                        {
                            return std::unexpected(Next.error());
                        }
                        if ((*Next)->Kind != TokenKind::EndOfLine)
                        {
                            return {};
                        }
                        HasCurrent = false;
                    }
                }

                ParseResult<void> expectEndOfLine(std::string_view Message = "unexpected trailing token")
                {
                    const auto Next = peek();
                    if (!Next)
                    {
                        return std::unexpected(Next.error());
                    }
                    if ((*Next)->Kind == TokenKind::EndOfLine)
                    {
                        HasCurrent = false;
                        return {};
                    }
                    if ((*Next)->Kind == TokenKind::EndOfFile)
                    {
                        return {};
                    }
                    return std::unexpected(detail::makeTextDiagnostic((*Next)->BeginOffset, (*Next)->EndOffset, Message));
                }

                ParseResult<std::uint64_t> parseUnsignedInteger(std::string_view Message = "expected an unsigned integer")
                {
                    const auto Number = expect(TokenKind::Number, Message);
                    if (!Number)
                    {
                        return std::unexpected(Number.error());
                    }
                    std::string_view Digits = Number->text();
                    int Base = 10;
                    if (Digits.starts_with("0x") || Digits.starts_with("0X"))
                    {
                        Digits.remove_prefix(2);
                        Base = 16;
                    }
                    std::uint64_t Value = 0;
                    const auto Result = std::from_chars(Digits.data(), Digits.data() + Digits.size(), Value, Base);
                    if (Digits.empty() || Result.ec != std::errc{} || Result.ptr != Digits.data() + Digits.size())
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Number->BeginOffset, Number->EndOffset, Message));
                    }
                    return Value;
                }

                ParseResult<std::int64_t> parseSignedDisplacement()
                {
                    const auto Sign = peek();
                    if (!Sign)
                    {
                        return std::unexpected(Sign.error());
                    }
                    const std::size_t SignBegin = (*Sign)->BeginOffset;
                    const bool Negative = (*Sign)->Kind == TokenKind::Minus;
                    if (!Negative && (*Sign)->Kind != TokenKind::Plus)
                    {
                        return std::unexpected(detail::makeTextDiagnostic((*Sign)->BeginOffset, (*Sign)->EndOffset, "expected a signed displacement"));
                    }
                    HasCurrent = false;
                    const auto MagnitudeToken = peek();
                    if (!MagnitudeToken)
                    {
                        return std::unexpected(MagnitudeToken.error());
                    }
                    const std::size_t MagnitudeEnd = (*MagnitudeToken)->EndOffset;
                    const auto Magnitude = parseUnsignedInteger("expected a displacement magnitude");
                    if (!Magnitude)
                    {
                        return std::unexpected(Magnitude.error());
                    }
                    constexpr std::uint64_t SignBit = std::uint64_t{1} << 63;
                    if (*Magnitude > SignBit || (!Negative && *Magnitude == SignBit))
                    {
                        return std::unexpected(detail::makeTextDiagnostic(SignBegin, MagnitudeEnd, "signed displacement overflows 64-bit two's complement range"));
                    }
                    if (!Negative)
                    {
                        return static_cast<std::int64_t>(*Magnitude);
                    }
                    return *Magnitude == SignBit ? std::numeric_limits<std::int64_t>::min() : -static_cast<std::int64_t>(*Magnitude);
                }

                ParseResult<std::int64_t> parseOptionalDisplacement()
                {
                    const auto Next = peek();
                    if (!Next)
                    {
                        return std::unexpected(Next.error());
                    }
                    if ((*Next)->Kind != TokenKind::Plus && (*Next)->Kind != TokenKind::Minus)
                    {
                        return 0;
                    }
                    return parseSignedDisplacement();
                }

                ParseResult<std::uint64_t> parseSignedOrUnsignedInteger(std::uint32_t BitWidth, std::string_view ExpectedMessage)
                {
                    const auto Start = peek();
                    if (!Start)
                    {
                        return std::unexpected(Start.error());
                    }
                    const std::size_t StartBegin = (*Start)->BeginOffset;
                    bool Negative = false;
                    if ((*Start)->Kind == TokenKind::Minus)
                    {
                        Negative = true;
                        HasCurrent = false;
                    }
                    else if ((*Start)->Kind == TokenKind::Plus)
                    {
                        HasCurrent = false;
                    }
                    const auto MagnitudeToken = peek();
                    if (!MagnitudeToken)
                    {
                        return std::unexpected(MagnitudeToken.error());
                    }
                    const std::size_t EndOffset = (*MagnitudeToken)->EndOffset;
                    const auto Magnitude = parseUnsignedInteger(ExpectedMessage);
                    if (!Magnitude)
                    {
                        return std::unexpected(Magnitude.error());
                    }
                    const std::uint64_t MaxUnsigned = BitWidth == 64 ? std::numeric_limits<std::uint64_t>::max() : ((std::uint64_t{1} << BitWidth) - 1);
                    const std::uint64_t MaxNegativeMagnitude = std::uint64_t{1} << (BitWidth - 1);
                    if (Negative)
                    {
                        if (*Magnitude > MaxNegativeMagnitude)
                        {
                            return std::unexpected(detail::makeTextDiagnostic(StartBegin, EndOffset, "negative integer overflows target bit width"));
                        }
                        return (std::uint64_t{0} - *Magnitude) & MaxUnsigned;
                    }
                    if (*Magnitude > MaxUnsigned)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(StartBegin, EndOffset, "integer literal overflows target bit width"));
                    }
                    return *Magnitude;
                }

                ParseResult<std::uint64_t> parseFloat64Bits()
                {
                    const auto Start = peek();
                    if (!Start)
                    {
                        return std::unexpected(Start.error());
                    }
                    const std::size_t StartBegin = (*Start)->BeginOffset;
                    bool Negative = false;
                    if ((*Start)->Kind == TokenKind::Minus)
                    {
                        Negative = true;
                        HasCurrent = false;
                    }
                    else if ((*Start)->Kind == TokenKind::Plus)
                    {
                        HasCurrent = false;
                    }
                    const auto ValueToken = peek();
                    if (!ValueToken)
                    {
                        return std::unexpected(ValueToken.error());
                    }
                    if ((*ValueToken)->Kind == TokenKind::Identifier)
                    {
                        const Token Word = *take();
                        if (Word.text() == "inf")
                        {
                            const double Value = Negative ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
                            return std::bit_cast<std::uint64_t>(Value);
                        }
                        if (Word.text() == "nan")
                        {
                            std::uint64_t Bits = std::bit_cast<std::uint64_t>(std::numeric_limits<double>::quiet_NaN());
                            if (Negative)
                            {
                                Bits |= std::uint64_t{1} << 63;
                            }
                            return Bits;
                        }
                        return std::unexpected(detail::makeTextDiagnostic(StartBegin, Word.EndOffset, "expected an IEEE 754 binary64 floating-point value"));
                    }
                    if ((*ValueToken)->Kind != TokenKind::Number)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(StartBegin, (*ValueToken)->EndOffset, "expected an IEEE 754 binary64 floating-point value"));
                    }
                    const Token Number = *take();
                    std::string_view Digits = Number.text();
                    double Value = 0.0;
                    std::from_chars_result Result{};
                    if (Digits.starts_with("0x") || Digits.starts_with("0X"))
                    {
                        if (Digits.find('p') == std::string_view::npos && Digits.find('P') == std::string_view::npos)
                        {
                            return std::unexpected(detail::makeTextDiagnostic(StartBegin, Number.EndOffset, "hexadecimal .f64 literals require a binary exponent 'p'; use .i64 for raw 64-bit bit patterns"));
                        }
                        Digits.remove_prefix(2);
                        Result = std::from_chars(Digits.data(), Digits.data() + Digits.size(), Value, std::chars_format::hex);
                    }
                    else
                    {
                        Result = std::from_chars(Digits.data(), Digits.data() + Digits.size(), Value, std::chars_format::general);
                    }
                    if (Digits.empty() || Result.ptr != Digits.data() + Digits.size())
                    {
                        return std::unexpected(detail::makeTextDiagnostic(StartBegin, Number.EndOffset, "expected an IEEE 754 binary64 floating-point value"));
                    }
                    if (Result.ec == std::errc::result_out_of_range)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(StartBegin, Number.EndOffset, "floating-point literal is out of IEEE 754 binary64 range"));
                    }
                    if (Result.ec != std::errc{})
                    {
                        return std::unexpected(detail::makeTextDiagnostic(StartBegin, Number.EndOffset, "expected an IEEE 754 binary64 floating-point value"));
                    }
                    if (Negative)
                    {
                        Value = -Value;
                    }
                    return std::bit_cast<std::uint64_t>(Value);
                }

                ParseResult<core::Name> parseSymbol()
                {
                    const auto AtToken = expect(TokenKind::At, "expected '@' before symbol name");
                    if (!AtToken)
                    {
                        return std::unexpected(AtToken.error());
                    }
                    auto NameToken = eat(TokenKind::Identifier);
                    if (!NameToken)
                    {
                        return std::unexpected(NameToken.error());
                    }
                    if (!*NameToken)
                    {
                        NameToken = eat(TokenKind::String);
                        if (!NameToken)
                        {
                            return std::unexpected(NameToken.error());
                        }
                    }
                    if (!*NameToken)
                    {
                        const auto Next = peek();
                        if (!Next)
                        {
                            return std::unexpected(Next.error());
                        }
                        return std::unexpected(detail::makeTextDiagnostic(AtToken->BeginOffset, (*Next)->EndOffset, "expected a nonempty symbol name after '@'"));
                    }
                    if ((*NameToken)->text().empty())
                    {
                        return std::unexpected(detail::makeTextDiagnostic((*NameToken)->BeginOffset, (*NameToken)->EndOffset, "symbol name must be nonempty"));
                    }
                    return detail::internTextName((*NameToken)->text(), (*NameToken)->BeginOffset, (*NameToken)->EndOffset);
                }

                ParseResult<detail::ParsedOperand> parseOperand()
                {
                    const auto Next = peek();
                    if (!Next)
                    {
                        return std::unexpected(Next.error());
                    }
                    if ((*Next)->Kind == TokenKind::LeftBracket)
                    {
                        HasCurrent = false;
                        std::optional<RegisterId> BaseRegister;
                        core::Name BaseSymbol;
                        const auto BaseIsAt = at(TokenKind::At);
                        if (!BaseIsAt)
                        {
                            return std::unexpected(BaseIsAt.error());
                        }
                        if (*BaseIsAt)
                        {
                            const auto Symbol = parseSymbol();
                            if (!Symbol)
                            {
                                return std::unexpected(Symbol.error());
                            }
                            BaseSymbol = *Symbol;
                        }
                        else
                        {
                            const auto BaseToken = expect(TokenKind::Identifier, "expected an address base register or symbol");
                            if (!BaseToken)
                            {
                                return std::unexpected(BaseToken.error());
                            }
                            BaseRegister = parseRegister(BaseToken->text());
                            if (!BaseRegister || !isAddressRegister(*BaseRegister))
                            {
                                return std::unexpected(detail::makeTextDiagnostic(BaseToken->BeginOffset, BaseToken->EndOffset, "unknown or invalid address base register"));
                            }
                        }
                        const auto Offset = parseOptionalDisplacement();
                        if (!Offset)
                        {
                            return std::unexpected(Offset.error());
                        }
                        if (const auto Closing = expect(TokenKind::RightBracket, "expected ']' after memory address"); !Closing)
                        {
                            return std::unexpected(Closing.error());
                        }
                        detail::ParsedOperand Result;
                        if (BaseRegister)
                        {
                            Result.emplace<AddressOperand>(*BaseRegister, *Offset);
                        }
                        else
                        {
                            Result.emplace<AddressOperand>(BaseSymbol, *Offset);
                        }
                        return Result;
                    }
                    if ((*Next)->Kind == TokenKind::At)
                    {
                        const auto Symbol = parseSymbol();
                        if (!Symbol)
                        {
                            return std::unexpected(Symbol.error());
                        }
                        const auto Offset = parseOptionalDisplacement();
                        if (!Offset)
                        {
                            return std::unexpected(Offset.error());
                        }
                        detail::ParsedOperand Result;
                        Result.emplace<ImmediateOperand>(*Symbol, *Offset);
                        return Result;
                    }
                    if ((*Next)->Kind == TokenKind::Identifier)
                    {
                        const Token RegisterToken = *take();
                        const auto Register = parseRegister(RegisterToken.text());
                        if (!Register || !isArithmeticRegister(*Register))
                        {
                            return std::unexpected(detail::makeTextDiagnostic(RegisterToken.BeginOffset, RegisterToken.EndOffset, "unknown register operand"));
                        }
                        const auto Offset = parseOptionalDisplacement();
                        if (!Offset)
                        {
                            return std::unexpected(Offset.error());
                        }
                        detail::ParsedOperand Result;
                        Result.emplace<RegisterOperand>(*Register, *Offset);
                        return Result;
                    }
                    if ((*Next)->Kind == TokenKind::Number || (*Next)->Kind == TokenKind::Plus || (*Next)->Kind == TokenKind::Minus)
                    {
                        const auto Bits = parseSignedOrUnsignedInteger(64, "expected an immediate integer");
                        if (!Bits)
                        {
                            return std::unexpected(Bits.error());
                        }
                        detail::ParsedOperand Result;
                        Result.emplace<ImmediateOperand>(*Bits);
                        return Result;
                    }
                    return std::unexpected(detail::makeTextDiagnostic((*Next)->BeginOffset, (*Next)->EndOffset, "expected an instruction operand"));
                }

                ParseResult<void> parseModuleHeader()
                {
                    const auto First = expect(TokenKind::Identifier, "expected .module followed by a quoted name");
                    if (!First)
                    {
                        return std::unexpected(First.error());
                    }
                    if (First->text() != detail::ModuleDirective)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(First->BeginOffset, First->EndOffset, "expected .module followed by a quoted name"));
                    }
                    const auto NameToken = expect(TokenKind::String, "expected a quoted module name");
                    if (!NameToken)
                    {
                        return std::unexpected(NameToken.error());
                    }
                    if (const auto EndResult = expectEndOfLine("unexpected token after .module header"); !EndResult)
                    {
                        return EndResult;
                    }
                    const auto Interned = detail::internTextName(NameToken->text(), NameToken->BeginOffset, NameToken->EndOffset);
                    if (!Interned)
                    {
                        return std::unexpected(Interned.error());
                    }
                    Module = std::make_unique<MachineModule>(*Interned);
                    return {};
                }

                ParseResult<void> parseModuleTopLevelLine()
                {
                    const auto First = expect(TokenKind::Identifier, "expected a top-level directive");
                    if (!First)
                    {
                        return std::unexpected(First.error());
                    }
                    const std::string_view Directive = First->text();
                    if (Directive == detail::DeclareDirective)
                    {
                        return parseSymbolDeclaration(*First);
                    }
                    if (Directive == detail::SectionDirective)
                    {
                        return parseSectionDirective(*First);
                    }
                    if (Directive == detail::GlobalDirective)
                    {
                        return parseGlobalDefinition(*First);
                    }
                    if (Directive == detail::FunctionDirective)
                    {
                        return parseFunctionDefinition(*First);
                    }
                    return std::unexpected(detail::makeTextDiagnostic(First->BeginOffset, First->EndOffset, "unexpected or misplaced top-level directive"));
                }

                ParseResult<void> parseSymbolDeclaration(const Token& Directive)
                {
                    if (HasSeenSection)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.BeginOffset, Directive.EndOffset, ".declare directives must precede all .section directives"));
                    }
                    const auto KindToken = expect(TokenKind::Identifier, "expected 'function' or 'global' after .declare");
                    if (!KindToken)
                    {
                        return std::unexpected(KindToken.error());
                    }
                    const auto Kind = detail::parseSymbolKind(KindToken->text());
                    if (!Kind)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(KindToken->BeginOffset, KindToken->EndOffset, "unknown symbol declaration kind"));
                    }
                    const auto Symbol = parseSymbol();
                    if (!Symbol)
                    {
                        return std::unexpected(Symbol.error());
                    }
                    bool IsConstant = false;
                    const auto Attribute = eat(TokenKind::Identifier);
                    if (!Attribute)
                    {
                        return std::unexpected(Attribute.error());
                    }
                    if (*Attribute)
                    {
                        if (*Kind != MachineSymbolKind::GlobalVariable || (*Attribute)->text() != detail::ReadOnlyKeyword)
                        {
                            return std::unexpected(detail::makeTextDiagnostic((*Attribute)->BeginOffset, (*Attribute)->EndOffset, "only global declarations accept the readonly attribute"));
                        }
                        IsConstant = true;
                    }
                    if (const auto EndResult = expectEndOfLine("unexpected token after the symbol declaration"); !EndResult)
                    {
                        return EndResult;
                    }
                    Module->declareSymbol(*Symbol, *Kind, IsConstant);
                    return {};
                }

                ParseResult<void> parseSectionDirective(const Token& Directive)
                {
                    const auto SectionToken = expect(TokenKind::Identifier, "section directive requires .data or .text");
                    if (!SectionToken)
                    {
                        return std::unexpected(SectionToken.error());
                    }
                    const auto Kind = detail::parseSectionKind(SectionToken->text());
                    if (!Kind)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(SectionToken->BeginOffset, SectionToken->EndOffset, "unknown section"));
                    }
                    if (const auto EndResult = expectEndOfLine("unexpected token after .section directive"); !EndResult)
                    {
                        return EndResult;
                    }
                    static_cast<void>(Directive);
                    CurrentSection = *Kind;
                    HasSeenSection = true;
                    return {};
                }

                ParseResult<void> parseGlobalDefinition(const Token& Directive)
                {
                    if (CurrentSection != detail::SectionKind::Data)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.BeginOffset, Directive.EndOffset, "global definition must appear in the .data section"));
                    }
                    core::Name GlobalName = core::NAME_None;
                    const auto HasSymbol = at(TokenKind::At);
                    if (!HasSymbol)
                    {
                        return std::unexpected(HasSymbol.error());
                    }
                    if (*HasSymbol)
                    {
                        const auto ParsedSymbol = parseSymbol();
                        if (!ParsedSymbol)
                        {
                            return std::unexpected(ParsedSymbol.error());
                        }
                        GlobalName = *ParsedSymbol;
                    }
                    std::optional<std::uint64_t> ByteSize;
                    std::optional<std::uint64_t> Alignment;
                    std::optional<MachineSymbolLinkage> Linkage;
                    bool IsConstant = false;
                    for (;;)
                    {
                        const auto LineDone = atLineEnd();
                        if (!LineDone)
                        {
                            return std::unexpected(LineDone.error());
                        }
                        if (*LineDone)
                        {
                            break;
                        }
                        const auto Attr = expect(TokenKind::Identifier, "expected global attribute 'size', 'align', 'readonly', 'local', or 'external'");
                        if (!Attr)
                        {
                            return std::unexpected(Attr.error());
                        }
                        const std::string_view Keyword = Attr->text();
                        if (Keyword == detail::SizeKeyword)
                        {
                            if (ByteSize)
                            {
                                return std::unexpected(detail::makeTextDiagnostic(Attr->BeginOffset, Attr->EndOffset, "duplicate 'size' attribute on global definition"));
                            }
                            const auto ParsedSize = parseUnsignedInteger("expected global byte size after 'size'");
                            if (!ParsedSize)
                            {
                                return std::unexpected(ParsedSize.error());
                            }
                            ByteSize = *ParsedSize;
                        }
                        else if (Keyword == detail::AlignKeyword)
                        {
                            if (Alignment)
                            {
                                return std::unexpected(detail::makeTextDiagnostic(Attr->BeginOffset, Attr->EndOffset, "duplicate 'align' attribute on global definition"));
                            }
                            const auto AlignToken = peek();
                            if (!AlignToken)
                            {
                                return std::unexpected(AlignToken.error());
                            }
                            const std::size_t AlignBegin = (*AlignToken)->BeginOffset;
                            const std::size_t AlignEnd = (*AlignToken)->EndOffset;
                            const auto ParsedAlign = parseUnsignedInteger("expected global alignment after 'align'");
                            if (!ParsedAlign)
                            {
                                return std::unexpected(ParsedAlign.error());
                            }
                            if (*ParsedAlign == 0 || (*ParsedAlign & (*ParsedAlign - 1)) != 0)
                            {
                                return std::unexpected(detail::makeTextDiagnostic(AlignBegin, AlignEnd, "data alignment must be a nonzero power of two"));
                            }
                            Alignment = *ParsedAlign;
                        }
                        else if (Keyword == detail::ReadOnlyKeyword)
                        {
                            if (IsConstant)
                            {
                                return std::unexpected(detail::makeTextDiagnostic(Attr->BeginOffset, Attr->EndOffset, "duplicate 'readonly' attribute on global definition"));
                            }
                            IsConstant = true;
                        }
                        else if (const auto ParsedLinkage = detail::parseLinkage(Keyword))
                        {
                            if (Linkage)
                            {
                                return std::unexpected(detail::makeTextDiagnostic(Attr->BeginOffset, Attr->EndOffset, "duplicate linkage attribute on global definition"));
                            }
                            if (*ParsedLinkage == MachineSymbolLinkage::External && !GlobalName.valid())
                            {
                                return std::unexpected(detail::makeTextDiagnostic(Attr->BeginOffset, Attr->EndOffset, "external definitions require a nonempty symbol name"));
                            }
                            Linkage = *ParsedLinkage;
                        }
                        else
                        {
                            return std::unexpected(detail::makeTextDiagnostic(Attr->BeginOffset, Attr->EndOffset, "unknown global attribute; expected 'size', 'align', 'readonly', 'local', or 'external'"));
                        }
                    }
                    if (const auto EndResult = expectEndOfLine(); !EndResult)
                    {
                        return EndResult;
                    }
                    if (!ByteSize)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.BeginOffset, Directive.EndOffset, "global definition requires a 'size' attribute"));
                    }
                    const std::uint64_t ResolvedAlignment = Alignment.value_or(1);
                    const MachineSymbolLinkage ResolvedLinkage = Linkage.value_or(MachineSymbolLinkage::Local);

                    std::vector<std::uint8_t> InitialBytes;
                    std::vector<MachineGlobalRelocation> Relocations;
                    for (;;)
                    {
                        if (const auto Skipped = skipBlankLines(); !Skipped)
                        {
                            return Skipped;
                        }
                        const auto Next = peek();
                        if (!Next)
                        {
                            return std::unexpected(Next.error());
                        }
                        if ((*Next)->Kind == TokenKind::EndOfFile)
                        {
                            return std::unexpected(detail::makeTextDiagnostic(LexerState.endOffset(), "unterminated global variable definition"));
                        }
                        const auto BodyDirective = expect(TokenKind::Identifier, "expected a global data directive or .endglobal");
                        if (!BodyDirective)
                        {
                            return std::unexpected(BodyDirective.error());
                        }
                        const std::string_view Keyword = BodyDirective->text();
                        if (Keyword == detail::ByteDirective)
                        {
                            if (const auto Result = parseByteDirective(*ByteSize, InitialBytes); !Result)
                            {
                                return Result;
                            }
                            continue;
                        }
                        if (Keyword == detail::BytesDirective)
                        {
                            if (const auto Result = parseBytesDirective(*ByteSize, InitialBytes); !Result)
                            {
                                return Result;
                            }
                            continue;
                        }
                        if (Keyword == detail::ZeroDirective)
                        {
                            if (const auto Result = parseZeroDirective(*ByteSize, InitialBytes); !Result)
                            {
                                return Result;
                            }
                            continue;
                        }
                        if (Keyword == detail::I16Directive)
                        {
                            if (const auto Result = parseIntDataDirective(16, *ByteSize, InitialBytes); !Result)
                            {
                                return Result;
                            }
                            continue;
                        }
                        if (Keyword == detail::I32Directive)
                        {
                            if (const auto Result = parseIntDataDirective(32, *ByteSize, InitialBytes); !Result)
                            {
                                return Result;
                            }
                            continue;
                        }
                        if (Keyword == detail::I64Directive)
                        {
                            if (const auto Result = parseIntDataDirective(64, *ByteSize, InitialBytes); !Result)
                            {
                                return Result;
                            }
                            continue;
                        }
                        if (Keyword == detail::F64Directive)
                        {
                            if (const auto Result = parseFloatDataDirective(*ByteSize, InitialBytes); !Result)
                            {
                                return Result;
                            }
                            continue;
                        }
                        if (Keyword == detail::RelocDirective)
                        {
                            if (const auto Result = parseRelocDirective(*ByteSize, ResolvedAlignment, Relocations); !Result)
                            {
                                return Result;
                            }
                            continue;
                        }
                        if (Keyword != detail::EndGlobalDirective)
                        {
                            return std::unexpected(detail::makeTextDiagnostic(BodyDirective->BeginOffset, BodyDirective->EndOffset, "expected a global data directive or .endglobal"));
                        }
                        if (const auto EndResult = expectEndOfLine("unexpected token after .endglobal"); !EndResult)
                        {
                            return EndResult;
                        }
                        const auto Created = Module->createGlobalVariable(*ByteSize, ResolvedAlignment, InitialBytes, GlobalName, ResolvedLinkage, IsConstant, Relocations);
                        if (!Created)
                        {
                            if (Created.error() == MachineModuleDataError::RelocationNonZeroInitialBytes)
                            {
                                return std::unexpected(detail::makeTextDiagnostic(BodyDirective->BeginOffset, BodyDirective->EndOffset, "global relocation overlaps nonzero initializer bytes"));
                            }
                            return std::unexpected(detail::makeTextDiagnostic(BodyDirective->BeginOffset, BodyDirective->EndOffset, "invalid global data layout"));
                        }
                        return {};
                    }
                }

                ParseResult<void> parseByteDirective(std::uint64_t DeclaredSize, std::vector<std::uint8_t>& InitialBytes)
                {
                    for (;;)
                    {
                        const auto ValueToken = peek();
                        if (!ValueToken)
                        {
                            return std::unexpected(ValueToken.error());
                        }
                        const std::size_t BeginOffset = (*ValueToken)->BeginOffset;
                        const std::size_t EndOffset = (*ValueToken)->EndOffset;
                        const auto ByteValue = parseUnsignedInteger("expected a byte value in the range 0 through 255");
                        if (!ByteValue)
                        {
                            return std::unexpected(ByteValue.error());
                        }
                        if (*ByteValue > 255)
                        {
                            return std::unexpected(detail::makeTextDiagnostic(BeginOffset, EndOffset, "expected a byte value in the range 0 through 255"));
                        }
                        if (InitialBytes.size() >= DeclaredSize)
                        {
                            return std::unexpected(detail::makeTextDiagnostic(BeginOffset, EndOffset, "global initializer exceeds its declared byte size"));
                        }
                        InitialBytes.push_back(static_cast<std::uint8_t>(*ByteValue));
                        const auto Comma = eat(TokenKind::Comma);
                        if (!Comma)
                        {
                            return std::unexpected(Comma.error());
                        }
                        if (!*Comma)
                        {
                            break;
                        }
                    }
                    return expectEndOfLine("unexpected token after byte value");
                }

                ParseResult<void> parseBytesDirective(std::uint64_t DeclaredSize, std::vector<std::uint8_t>& InitialBytes)
                {
                    const auto StringToken = expect(TokenKind::String, "expected a quoted byte string after .bytes");
                    if (!StringToken)
                    {
                        return std::unexpected(StringToken.error());
                    }
                    if (const auto EndResult = expectEndOfLine("unexpected token after .bytes literal"); !EndResult)
                    {
                        return EndResult;
                    }
                    const std::string_view Bytes = StringToken->text();
                    if (Bytes.size() > DeclaredSize - InitialBytes.size())
                    {
                        return std::unexpected(detail::makeTextDiagnostic(StringToken->BeginOffset, StringToken->EndOffset, "global initializer exceeds its declared byte size"));
                    }
                    for (const char Byte : Bytes)
                    {
                        InitialBytes.push_back(static_cast<std::uint8_t>(Byte));
                    }
                    return {};
                }

                ParseResult<void> parseZeroDirective(std::uint64_t DeclaredSize, std::vector<std::uint8_t>& InitialBytes)
                {
                    const auto CountToken = peek();
                    if (!CountToken)
                    {
                        return std::unexpected(CountToken.error());
                    }
                    const std::size_t BeginOffset = (*CountToken)->BeginOffset;
                    const std::size_t EndOffset = (*CountToken)->EndOffset;
                    const auto Count = parseUnsignedInteger("expected an unsigned zero-byte count after .zero");
                    if (!Count)
                    {
                        return std::unexpected(Count.error());
                    }
                    if (const auto EndResult = expectEndOfLine("unexpected token after .zero count"); !EndResult)
                    {
                        return EndResult;
                    }
                    if (*Count > DeclaredSize - InitialBytes.size())
                    {
                        return std::unexpected(detail::makeTextDiagnostic(BeginOffset, EndOffset, "global initializer exceeds its declared byte size"));
                    }
                    if (*Count > InitialBytes.max_size() - InitialBytes.size())
                    {
                        return std::unexpected(detail::makeTextDiagnostic(BeginOffset, EndOffset, "global initializer exceeds host storage limits"));
                    }
                    InitialBytes.insert(InitialBytes.end(), static_cast<std::size_t>(*Count), std::uint8_t{0});
                    return {};
                }

                ParseResult<void> parseIntDataDirective(std::uint32_t BitWidth, std::uint64_t DeclaredSize, std::vector<std::uint8_t>& InitialBytes)
                {
                    const std::size_t ByteWidth = BitWidth / 8;
                    for (;;)
                    {
                        const auto StartToken = peek();
                        if (!StartToken)
                        {
                            return std::unexpected(StartToken.error());
                        }
                        const std::size_t BeginOffset = (*StartToken)->BeginOffset;
                        const auto Bits = parseSignedOrUnsignedInteger(BitWidth, "expected an integer value");
                        if (!Bits)
                        {
                            return std::unexpected(Bits.error());
                        }
                        if (ByteWidth > DeclaredSize - InitialBytes.size())
                        {
                            return std::unexpected(detail::makeTextDiagnostic(BeginOffset, LastTokenEnd, "global initializer exceeds its declared byte size"));
                        }
                        for (std::size_t ByteIndex = 0; ByteIndex < ByteWidth; ++ByteIndex)
                        {
                            InitialBytes.push_back(static_cast<std::uint8_t>((*Bits >> (ByteIndex * 8)) & 0xFFU));
                        }
                        const auto Comma = eat(TokenKind::Comma);
                        if (!Comma)
                        {
                            return std::unexpected(Comma.error());
                        }
                        if (!*Comma)
                        {
                            break;
                        }
                    }
                    return expectEndOfLine("unexpected token after integer value");
                }

                ParseResult<void> parseFloatDataDirective(std::uint64_t DeclaredSize, std::vector<std::uint8_t>& InitialBytes)
                {
                    for (;;)
                    {
                        const auto StartToken = peek();
                        if (!StartToken)
                        {
                            return std::unexpected(StartToken.error());
                        }
                        const std::size_t BeginOffset = (*StartToken)->BeginOffset;
                        const auto Bits = parseFloat64Bits();
                        if (!Bits)
                        {
                            return std::unexpected(Bits.error());
                        }
                        if (8 > DeclaredSize - InitialBytes.size())
                        {
                            return std::unexpected(detail::makeTextDiagnostic(BeginOffset, LastTokenEnd, "global initializer exceeds its declared byte size"));
                        }
                        for (std::size_t ByteIndex = 0; ByteIndex < 8; ++ByteIndex)
                        {
                            InitialBytes.push_back(static_cast<std::uint8_t>((*Bits >> (ByteIndex * 8)) & 0xFFU));
                        }
                        const auto Comma = eat(TokenKind::Comma);
                        if (!Comma)
                        {
                            return std::unexpected(Comma.error());
                        }
                        if (!*Comma)
                        {
                            break;
                        }
                    }
                    return expectEndOfLine("unexpected token after floating-point value");
                }

                ParseResult<void> parseRelocDirective(std::uint64_t DeclaredSize, std::uint64_t Alignment, std::vector<MachineGlobalRelocation>& Relocations)
                {
                    const auto OffsetToken = peek();
                    if (!OffsetToken)
                    {
                        return std::unexpected(OffsetToken.error());
                    }
                    const std::size_t OffsetBegin = (*OffsetToken)->BeginOffset;
                    const std::size_t OffsetEnd = (*OffsetToken)->EndOffset;
                    const auto ByteOffset = parseUnsignedInteger("expected a relocation byte offset");
                    if (!ByteOffset)
                    {
                        return std::unexpected(ByteOffset.error());
                    }
                    if (const auto Comma = expect(TokenKind::Comma, "expected ',' after relocation byte offset"); !Comma)
                    {
                        return std::unexpected(Comma.error());
                    }
                    const auto Symbol = parseSymbol();
                    if (!Symbol)
                    {
                        return std::unexpected(Symbol.error());
                    }
                    const auto Addend = parseOptionalDisplacement();
                    if (!Addend)
                    {
                        return std::unexpected(Addend.error());
                    }
                    if (const auto EndResult = expectEndOfLine("unexpected token after global relocation"); !EndResult)
                    {
                        return EndResult;
                    }
                    if (Alignment < MachineGlobalRelocation::ByteSize || *ByteOffset % MachineGlobalRelocation::ByteSize != 0)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(OffsetBegin, OffsetEnd, "global relocations require alignment >= 8 and an eight-byte aligned byte offset"));
                    }
                    if (*ByteOffset > DeclaredSize || MachineGlobalRelocation::ByteSize > DeclaredSize - *ByteOffset)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(OffsetBegin, OffsetEnd, "global relocation exceeds its declared byte size"));
                    }
                    for (const MachineGlobalRelocation& Existing : Relocations)
                    {
                        if (Existing.ByteOffset == *ByteOffset)
                        {
                            return std::unexpected(detail::makeTextDiagnostic(OffsetBegin, OffsetEnd, "global relocation overlaps an earlier relocation"));
                        }
                    }
                    Relocations.push_back({*ByteOffset, *Symbol, *Addend});
                    return {};
                }

                ParseResult<void> parseFunctionDefinition(const Token& Directive)
                {
                    if (CurrentSection != detail::SectionKind::Text)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.BeginOffset, Directive.EndOffset, "function definition must appear in the .text section"));
                    }
                    core::Name FunctionName = core::NAME_None;
                    const auto HasSymbol = at(TokenKind::At);
                    if (!HasSymbol)
                    {
                        return std::unexpected(HasSymbol.error());
                    }
                    if (*HasSymbol)
                    {
                        const auto ParsedSymbol = parseSymbol();
                        if (!ParsedSymbol)
                        {
                            return std::unexpected(ParsedSymbol.error());
                        }
                        FunctionName = *ParsedSymbol;
                    }
                    MachineSymbolLinkage Linkage = MachineSymbolLinkage::Local;
                    const auto LinkageToken = eat(TokenKind::Identifier);
                    if (!LinkageToken)
                    {
                        return std::unexpected(LinkageToken.error());
                    }
                    if (*LinkageToken)
                    {
                        const auto Parsed = detail::parseLinkage((*LinkageToken)->text());
                        if (!Parsed)
                        {
                            return std::unexpected(detail::makeTextDiagnostic((*LinkageToken)->BeginOffset, (*LinkageToken)->EndOffset, "expected 'local' or 'external' linkage"));
                        }
                        if (*Parsed == MachineSymbolLinkage::External && !FunctionName.valid())
                        {
                            return std::unexpected(detail::makeTextDiagnostic((*LinkageToken)->BeginOffset, (*LinkageToken)->EndOffset, "external definitions require a nonempty symbol name"));
                        }
                        Linkage = *Parsed;
                    }
                    if (const auto EndResult = expectEndOfLine("unexpected token after .function header"); !EndResult)
                    {
                        return EndResult;
                    }

                    MachineFunction& Function = Module->createFunction(FunctionName, Linkage);
                    FunctionParseState State{Function};
                    for (;;)
                    {
                        if (const auto Skipped = skipBlankLines(); !Skipped)
                        {
                            return Skipped;
                        }
                        const auto Next = peek();
                        if (!Next)
                        {
                            return std::unexpected(Next.error());
                        }
                        if ((*Next)->Kind == TokenKind::EndOfFile)
                        {
                            return std::unexpected(detail::makeTextDiagnostic(LexerState.endOffset(), "unterminated function definition"));
                        }
                        const auto First = expect(TokenKind::Identifier, "expected a directive, label, or instruction");
                        if (!First)
                        {
                            return std::unexpected(First.error());
                        }
                        const auto Colon = eat(TokenKind::Colon);
                        if (!Colon)
                        {
                            return std::unexpected(Colon.error());
                        }
                        if (*Colon)
                        {
                            if (const auto BlockResult = parseBasicBlock(State, *First); !BlockResult)
                            {
                                return BlockResult;
                            }
                            continue;
                        }
                        const std::string_view Keyword = First->text();
                        if (Keyword == detail::StackSlotDirective)
                        {
                            if (const auto SlotResult = parseStackSlot(State, *First); !SlotResult)
                            {
                                return SlotResult;
                            }
                            continue;
                        }
                        if (Keyword == detail::FrameDirective)
                        {
                            if (const auto FrameResult = parseFrameLayout(State, *First); !FrameResult)
                            {
                                return FrameResult;
                            }
                            continue;
                        }
                        if (Keyword == detail::EndFunctionDirective)
                        {
                            if (const auto EndResult = expectEndOfLine("unexpected token after .endfunction"); !EndResult)
                            {
                                return EndResult;
                            }
                            return emitFunctionInstructions(State);
                        }
                        if (Keyword.starts_with('.'))
                        {
                            return std::unexpected(detail::makeTextDiagnostic(First->BeginOffset, First->EndOffset, "unknown or misplaced directive inside function"));
                        }
                        if (const auto InstructionResult = parseInstruction(State, *First); !InstructionResult)
                        {
                            return InstructionResult;
                        }
                    }
                }

                ParseResult<void> parseStackSlot(FunctionParseState& State, const Token& Directive)
                {
                    if (State.HasBlock || State.HasFrame)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.BeginOffset, Directive.EndOffset, "stack slots and one final frame layout must precede all basic blocks"));
                    }
                    std::optional<std::uint64_t> SlotSize;
                    std::optional<std::uint64_t> Alignment;
                    for (;;)
                    {
                        const auto LineDone = atLineEnd();
                        if (!LineDone)
                        {
                            return std::unexpected(LineDone.error());
                        }
                        if (*LineDone)
                        {
                            break;
                        }
                        const auto Attr = expect(TokenKind::Identifier, "expected stack slot attribute 'size' or 'align'");
                        if (!Attr)
                        {
                            return std::unexpected(Attr.error());
                        }
                        if (Attr->text() == detail::SizeKeyword)
                        {
                            if (SlotSize)
                            {
                                return std::unexpected(detail::makeTextDiagnostic(Attr->BeginOffset, Attr->EndOffset, "duplicate 'size' attribute on .stackslot"));
                            }
                            const auto ParsedSize = parseUnsignedInteger("expected stack slot byte size after 'size'");
                            if (!ParsedSize)
                            {
                                return std::unexpected(ParsedSize.error());
                            }
                            SlotSize = *ParsedSize;
                        }
                        else if (Attr->text() == detail::AlignKeyword)
                        {
                            if (Alignment)
                            {
                                return std::unexpected(detail::makeTextDiagnostic(Attr->BeginOffset, Attr->EndOffset, "duplicate 'align' attribute on .stackslot"));
                            }
                            const auto AlignToken = peek();
                            if (!AlignToken)
                            {
                                return std::unexpected(AlignToken.error());
                            }
                            const std::size_t AlignBegin = (*AlignToken)->BeginOffset;
                            const std::size_t AlignEnd = (*AlignToken)->EndOffset;
                            const auto ParsedAlign = parseUnsignedInteger("expected stack slot alignment after 'align'");
                            if (!ParsedAlign)
                            {
                                return std::unexpected(ParsedAlign.error());
                            }
                            if (*ParsedAlign == 0 || (*ParsedAlign & (*ParsedAlign - 1)) != 0)
                            {
                                return std::unexpected(detail::makeTextDiagnostic(AlignBegin, AlignEnd, "stack slot alignment must be a nonzero power of two"));
                            }
                            Alignment = *ParsedAlign;
                        }
                        else
                        {
                            return std::unexpected(detail::makeTextDiagnostic(Attr->BeginOffset, Attr->EndOffset, "unknown .stackslot attribute; expected 'size' or 'align'"));
                        }
                    }
                    if (const auto EndResult = expectEndOfLine(); !EndResult)
                    {
                        return EndResult;
                    }
                    if (!SlotSize)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.BeginOffset, Directive.EndOffset, ".stackslot requires a 'size' attribute"));
                    }
                    if (!State.Function.frameInfo().createStackSlot(*SlotSize, Alignment.value_or(1)))
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.BeginOffset, Directive.EndOffset, "invalid stack slot size or alignment"));
                    }
                    return {};
                }

                ParseResult<void> parseFrameLayout(FunctionParseState& State, const Token& Directive)
                {
                    if (State.HasBlock || State.HasFrame)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.BeginOffset, Directive.EndOffset, "stack slots and one final frame layout must precede all basic blocks"));
                    }
                    const auto BaseKeywordToken = expect(TokenKind::Identifier, "expected 'base' after .frame");
                    if (!BaseKeywordToken)
                    {
                        return std::unexpected(BaseKeywordToken.error());
                    }
                    if (BaseKeywordToken->text() != detail::BaseKeyword)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(BaseKeywordToken->BeginOffset, BaseKeywordToken->EndOffset, "expected 'base' after .frame"));
                    }
                    const auto BaseToken = peek();
                    if (!BaseToken)
                    {
                        return std::unexpected(BaseToken.error());
                    }
                    const std::size_t BaseBegin = (*BaseToken)->BeginOffset;
                    const std::size_t BaseEnd = (*BaseToken)->EndOffset;
                    const auto BaseOffset = parseUnsignedInteger("expected frame layout base offset after 'base'");
                    if (!BaseOffset)
                    {
                        return std::unexpected(BaseOffset.error());
                    }
                    if (const auto EndResult = expectEndOfLine("unexpected token after .frame directive"); !EndResult)
                    {
                        return EndResult;
                    }
                    if (!State.Function.frameInfo().layout(*BaseOffset))
                    {
                        return std::unexpected(detail::makeTextDiagnostic(BaseBegin, BaseEnd, "frame layout overflows unsigned 64-bit byte range"));
                    }
                    State.HasFrame = true;
                    return {};
                }

                ParseResult<void> parseBasicBlock(FunctionParseState& State, const Token& LabelToken)
                {
                    const auto AliasToken = eat(TokenKind::String);
                    if (!AliasToken)
                    {
                        return std::unexpected(AliasToken.error());
                    }
                    if (const auto EndResult = expectEndOfLine("a block label accepts at most one quoted original name"); !EndResult)
                    {
                        return EndResult;
                    }
                    if (State.BlockLabels.contains(LabelToken.text()))
                    {
                        return std::unexpected(detail::makeTextDiagnostic(LabelToken.BeginOffset, LabelToken.EndOffset, "duplicate block label in the function"));
                    }
                    const Token& NameSource = *AliasToken ? **AliasToken : LabelToken;
                    const auto BlockName = detail::internTextName(NameSource.text(), NameSource.BeginOffset, NameSource.EndOffset);
                    if (!BlockName)
                    {
                        return std::unexpected(BlockName.error());
                    }
                    State.HasBlock = true;
                    MachineBasicBlock& Block = State.Function.createBasicBlock(*BlockName);
                    State.BlockLabels.emplace(LabelToken.text(), &Block);
                    State.CurrentBlock = &Block;
                    return {};
                }

                ParseResult<void> parseInstruction(FunctionParseState& State, const Token& Mnemonic)
                {
                    if (!State.CurrentBlock)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Mnemonic.BeginOffset, Mnemonic.EndOffset, "an instruction requires a basic block"));
                    }
                    const auto Signature = detail::lookupMnemonic(Mnemonic.text());
                    if (!Signature)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Mnemonic.BeginOffset, Mnemonic.EndOffset, "unknown instruction mnemonic"));
                    }
                    PendingInstruction Pending{State.CurrentBlock, nullptr, {}, {}, Mnemonic.BeginOffset, Mnemonic.EndOffset, 0, 0, 0};
                    std::array<OperandKind, 3> Kinds{};
                    std::size_t OperandCount = 0;
                    if (Signature->HasBlockTarget)
                    {
                        const std::size_t TargetIndex = Signature->BlockTargetIndex;
                        for (std::size_t Index = 0; Index < TargetIndex; ++Index)
                        {
                            auto OperandValue = parseOperand();
                            if (!OperandValue)
                            {
                                return std::unexpected(OperandValue.error());
                            }
                            if (const auto Comma = expect(TokenKind::Comma, "expected ',' after branch comparison operand"); !Comma)
                            {
                                return std::unexpected(Comma.error());
                            }
                            Kinds[Index] = *detail::parsedOperandKind(*OperandValue);
                            Pending.Operands[Index] = std::move(*OperandValue);
                        }
                        const auto TargetToken = expect(TokenKind::Identifier, "a branch requires a final basic block label");
                        if (!TargetToken)
                        {
                            return std::unexpected(TargetToken.error());
                        }
                        if (const auto EndResult = expectEndOfLine("unexpected token after basic block target"); !EndResult)
                        {
                            return EndResult;
                        }
                        Pending.TargetLabel = TargetToken->text();
                        Pending.TargetBeginOffset = TargetToken->BeginOffset;
                        Pending.TargetEndOffset = TargetToken->EndOffset;
                        Pending.TargetOperandIndex = TargetIndex;
                        Kinds[TargetIndex] = OperandKind::BasicBlock;
                        OperandCount = TargetIndex + 1;
                    }
                    else
                    {
                        const auto LineDone = atLineEnd();
                        if (!LineDone)
                        {
                            return std::unexpected(LineDone.error());
                        }
                        if (!*LineDone)
                        {
                            for (;;)
                            {
                                const auto Start = peek();
                                if (!Start)
                                {
                                    return std::unexpected(Start.error());
                                }
                                if (OperandCount == Pending.Operands.size())
                                {
                                    return std::unexpected(detail::makeTextDiagnostic((*Start)->BeginOffset, (*Start)->EndOffset, "too many instruction operands"));
                                }
                                auto OperandValue = parseOperand();
                                if (!OperandValue)
                                {
                                    return std::unexpected(OperandValue.error());
                                }
                                Kinds[OperandCount] = *detail::parsedOperandKind(*OperandValue);
                                Pending.Operands[OperandCount++] = std::move(*OperandValue);
                                const auto Comma = eat(TokenKind::Comma);
                                if (!Comma)
                                {
                                    return std::unexpected(Comma.error());
                                }
                                if (!*Comma)
                                {
                                    break;
                                }
                            }
                        }
                        if (const auto EndResult = expectEndOfLine("expected ',' between instruction operands"); !EndResult)
                        {
                            return EndResult;
                        }
                    }
                    const detail::OpcodeDescriptor* Descriptor = detail::findOpcodeDescriptor(Mnemonic.text(), {Kinds.data(), OperandCount});
                    if (!Descriptor)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Mnemonic.BeginOffset, Mnemonic.EndOffset, "operand types or count do not match instruction mnemonic"));
                    }
                    if (const auto ValidationError = detail::validateParsedInstruction(*Descriptor, Pending.Operands))
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Mnemonic.BeginOffset, Mnemonic.EndOffset, *ValidationError));
                    }
                    Pending.Descriptor = Descriptor;
                    State.PendingInstructions.push_back(std::move(Pending));
                    return {};
                }

                ParseResult<void> emitFunctionInstructions(FunctionParseState& State)
                {
                    for (PendingInstruction& Pending : State.PendingInstructions)
                    {
                        if (Pending.Descriptor->HasBlockTarget)
                        {
                            const auto Target = State.BlockLabels.find(Pending.TargetLabel);
                            if (Target == State.BlockLabels.end())
                            {
                                return std::unexpected(detail::makeTextDiagnostic(Pending.TargetBeginOffset, Pending.TargetEndOffset, "unknown basic block label in this function"));
                            }
                            Pending.Operands[Pending.TargetOperandIndex].emplace<BasicBlockOperand>(Target->second);
                        }
                    }
                    MachineInstructionBuilder Builder;
                    for (const PendingInstruction& Pending : State.PendingInstructions)
                    {
                        Builder.setInsertPoint(*Pending.Block);
                        if (!Pending.Descriptor->Create(Builder, Pending.Operands))
                        {
                            return std::unexpected(detail::makeTextDiagnostic(Pending.BeginOffset, Pending.EndOffset, "instruction operands cannot be inserted into their function"));
                        }
                    }
                    return {};
                }

                Lexer LexerState;
                Token Current;
                bool HasCurrent = false;
                std::size_t LastTokenEnd = 0;
                std::unique_ptr<MachineModule> Module;
                detail::SectionKind CurrentSection = detail::SectionKind::None;
                bool HasSeenSection = false;
        };
    } // namespace

    std::expected<std::unique_ptr<MachineModule>, core::Diagnostic> readMachineModuleText(std::string_view Text)
    {
        if (Text.size() > core::SourceLocation::MaxByteOffset)
        {
            return std::unexpected(detail::makeTextDiagnostic(0, "VM text exceeds the supported source offset range"));
        }
        return ModuleTextReader(Text).parseModule();
    }
} // namespace ink::vm
