#include "ink/backend/vm/serialization/machine_module_text.h"

#include "machine_module_text_format.h"
#include "machine_module_text_lexer.h"

#include "ink/backend/vm/instruction/instruction_encoding.h"
#include "ink/backend/vm/instruction/machine_instruction_validator.h"
#include "ink/backend/vm/instruction/machine_operand.h"
#include "ink/backend/vm/instruction/opcode_info.h"
#include "ink/backend/vm/instruction/register.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/name/name_pool.h"

#include <algorithm>
#include <array>
#include <bit>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ink::vm
{
    namespace
    {
        using detail::AsmLexer;
        using detail::Token;
        using detail::TokenKind;

        struct BlockLabelEntry
        {
                MachineBasicBlock* Block = nullptr;
                bool Defined = false;
        };

        struct ForwardBlockRef
        {
                std::string_view Label;
                std::size_t BeginOffset = 0;
                std::size_t EndOffset = 0;
        };

        struct ParsedRelocationSpan
        {
                std::size_t BeginOffset = 0;
                std::size_t EndOffset = 0;
        };

        struct FunctionParseState
        {
                MachineFunction& Function;
                MachineInstructionBuilder Builder;
                MachineBasicBlock* CurrentBlock = nullptr;
                std::unordered_map<std::string_view, BlockLabelEntry> BlockLabels;
                std::vector<ForwardBlockRef> ForwardRefs;
                bool HasFrame = false;
                bool HasBlock = false;
        };

        // Single-pass recursive-descent parser driven by AsmLexer with sticky diagnostics and direct instruction emission.
        class ModuleTextParser final
        {
            public:
                explicit ModuleTextParser(std::string_view Text) noexcept
                    : Lexer(Text, Error)
                {
                }

                std::expected<std::unique_ptr<MachineModule>, core::Diagnostic> parse()
                {
                    if (!parseModule())
                    {
                        return std::unexpected(std::move(*Error));
                    }
                    return std::move(Module);
                }

            private:
                bool fail(std::size_t BeginOffset, std::size_t EndOffset, std::string_view Description)
                {
                    if (!Error)
                    {
                        Error = detail::makeTextDiagnostic(BeginOffset, EndOffset, Description);
                    }
                    return false;
                }

                bool fail(std::size_t Offset, std::string_view Description)
                {
                    return fail(Offset, Offset, Description);
                }

                bool expect(TokenKind Kind, std::string_view Message, Token& Out)
                {
                    const Token& Next = Lexer.peek();
                    if (Next.Kind == TokenKind::Error)
                    {
                        return false;
                    }
                    if (Next.Kind != Kind)
                    {
                        return fail(Next.BeginOffset, Next.EndOffset, Message);
                    }
                    Out = Lexer.take();
                    return true;
                }

                bool expectEndOfLine(std::string_view Message = "unexpected trailing token")
                {
                    const Token& Next = Lexer.peek();
                    if (Next.Kind == TokenKind::Error)
                    {
                        return false;
                    }
                    if (Next.Kind == TokenKind::EndOfLine)
                    {
                        Lexer.consume();
                        return true;
                    }
                    if (Next.Kind == TokenKind::EndOfFile)
                    {
                        return true;
                    }
                    return fail(Next.BeginOffset, Next.EndOffset, Message);
                }

                bool internName(std::string_view Text, std::size_t BeginOffset, std::size_t EndOffset, core::Name& Out)
                {
                    Out = core::NamePool::instance().intern(Text);
                    if (!Text.empty() && !Out.valid())
                    {
                        return fail(BeginOffset, EndOffset, "name pool capacity exhausted");
                    }
                    return true;
                }

                bool parseSymbol(core::Name& Out)
                {
                    Token SymbolToken;
                    if (!expect(TokenKind::Symbol, "expected '@symbol' name", SymbolToken))
                    {
                        return false;
                    }
                    return internName(SymbolToken.text(), SymbolToken.BeginOffset, SymbolToken.EndOffset, Out);
                }

                bool parseUnsignedInteger(std::uint64_t& Out, std::string_view Message = "expected an unsigned integer")
                {
                    Token Number;
                    if (!expect(TokenKind::Number, Message, Number))
                    {
                        return false;
                    }
                    std::string_view Digits = Number.text();
                    int Base = 10;
                    if (Digits.starts_with("0x") || Digits.starts_with("0X"))
                    {
                        Digits.remove_prefix(2);
                        Base = 16;
                    }
                    if (Digits.empty())
                    {
                        return fail(Number.BeginOffset, Number.EndOffset, Message);
                    }
                    std::uint64_t Value = 0;
                    const auto Result = std::from_chars(Digits.data(), Digits.data() + Digits.size(), Value, Base);
                    if (Result.ec == std::errc::result_out_of_range)
                    {
                        return fail(Number.BeginOffset, Number.EndOffset, "unsigned integer overflows 64-bit range");
                    }
                    if (Result.ec != std::errc{} || Result.ptr != Digits.data() + Digits.size())
                    {
                        return fail(Number.BeginOffset, Number.EndOffset, Message);
                    }
                    Out = Value;
                    return true;
                }

                bool parseSignedDisplacement(std::int64_t& Out)
                {
                    const Token& Sign = Lexer.peek();
                    if (Sign.Kind == TokenKind::Error)
                    {
                        return false;
                    }
                    const std::size_t SignBegin = Sign.BeginOffset;
                    const std::size_t SignEnd = Sign.EndOffset;
                    const bool Negative = Sign.Kind == TokenKind::Minus;
                    if (!Negative && Sign.Kind != TokenKind::Plus)
                    {
                        return fail(SignBegin, SignEnd, "expected a signed displacement");
                    }
                    Lexer.consume();
                    const Token& MagnitudeToken = Lexer.peek();
                    if (MagnitudeToken.Kind == TokenKind::Error)
                    {
                        return false;
                    }
                    if (MagnitudeToken.Kind == TokenKind::EndOfLine || MagnitudeToken.Kind == TokenKind::EndOfFile)
                    {
                        return fail(SignBegin, SignEnd, "expected a displacement magnitude");
                    }
                    const std::size_t MagnitudeEnd = MagnitudeToken.EndOffset;
                    std::uint64_t Magnitude = 0;
                    if (!parseUnsignedInteger(Magnitude, "expected a displacement magnitude"))
                    {
                        return false;
                    }
                    constexpr std::uint64_t SignBit = std::uint64_t{1} << 63;
                    if (Magnitude > SignBit || (!Negative && Magnitude == SignBit))
                    {
                        return fail(SignBegin, MagnitudeEnd, "signed displacement overflows 64-bit two's complement range");
                    }
                    if (!Negative)
                    {
                        Out = static_cast<std::int64_t>(Magnitude);
                    }
                    else
                    {
                        Out = Magnitude == SignBit ? std::numeric_limits<std::int64_t>::min() : -static_cast<std::int64_t>(Magnitude);
                    }
                    return true;
                }

                bool parseOptionalDisplacement(std::int64_t& Out)
                {
                    const Token& Next = Lexer.peek();
                    if (Next.Kind == TokenKind::Error)
                    {
                        return false;
                    }
                    if (Next.Kind != TokenKind::Plus && Next.Kind != TokenKind::Minus)
                    {
                        Out = 0;
                        return true;
                    }
                    return parseSignedDisplacement(Out);
                }

                bool parseSignedOrUnsignedInteger(std::uint32_t BitWidth, std::uint64_t& Out, std::string_view ExpectedMessage)
                {
                    const Token& Start = Lexer.peek();
                    if (Start.Kind == TokenKind::Error)
                    {
                        return false;
                    }
                    const std::size_t StartBegin = Start.BeginOffset;
                    const std::size_t SignEnd = Start.EndOffset;
                    bool Negative = false;
                    bool HasSign = false;
                    if (Start.Kind == TokenKind::Minus)
                    {
                        Negative = true;
                        HasSign = true;
                        Lexer.consume();
                    }
                    else if (Start.Kind == TokenKind::Plus)
                    {
                        HasSign = true;
                        Lexer.consume();
                    }
                    const Token& MagnitudeToken = Lexer.peek();
                    if (MagnitudeToken.Kind == TokenKind::Error)
                    {
                        return false;
                    }
                    if (HasSign && (MagnitudeToken.Kind == TokenKind::EndOfLine || MagnitudeToken.Kind == TokenKind::EndOfFile))
                    {
                        return fail(StartBegin, SignEnd, ExpectedMessage);
                    }
                    const std::size_t EndOffset = MagnitudeToken.EndOffset;
                    std::uint64_t Magnitude = 0;
                    if (!parseUnsignedInteger(Magnitude, ExpectedMessage))
                    {
                        return false;
                    }
                    const std::uint64_t MaxUnsigned = BitWidth == 64 ? std::numeric_limits<std::uint64_t>::max() : ((std::uint64_t{1} << BitWidth) - 1);
                    const std::uint64_t MaxNegativeMagnitude = std::uint64_t{1} << (BitWidth - 1);
                    if (Negative)
                    {
                        if (Magnitude > MaxNegativeMagnitude)
                        {
                            return fail(StartBegin, EndOffset, "negative integer overflows target bit width");
                        }
                        Out = (std::uint64_t{0} - Magnitude) & MaxUnsigned;
                        return true;
                    }
                    if (Magnitude > MaxUnsigned)
                    {
                        return fail(StartBegin, EndOffset, "integer literal overflows target bit width");
                    }
                    Out = Magnitude;
                    return true;
                }

                bool parseFloat64Bits(std::uint64_t& Out)
                {
                    const Token& Start = Lexer.peek();
                    if (Start.Kind == TokenKind::Error)
                    {
                        return false;
                    }
                    const std::size_t StartBegin = Start.BeginOffset;
                    const std::size_t SignEnd = Start.EndOffset;
                    bool Negative = false;
                    bool HasSign = false;
                    if (Start.Kind == TokenKind::Minus)
                    {
                        Negative = true;
                        HasSign = true;
                        Lexer.consume();
                    }
                    else if (Start.Kind == TokenKind::Plus)
                    {
                        HasSign = true;
                        Lexer.consume();
                    }
                    const Token& ValueToken = Lexer.peek();
                    if (ValueToken.Kind == TokenKind::Error)
                    {
                        return false;
                    }
                    if (ValueToken.Kind == TokenKind::EndOfLine || ValueToken.Kind == TokenKind::EndOfFile)
                    {
                        return fail(StartBegin, HasSign ? SignEnd : ValueToken.EndOffset, "expected an IEEE 754 binary64 floating-point value");
                    }
                    if (ValueToken.Kind == TokenKind::Identifier)
                    {
                        const Token Word = Lexer.take();
                        if (Word.text() == "inf")
                        {
                            const double Value = Negative ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
                            Out = std::bit_cast<std::uint64_t>(Value);
                            return true;
                        }
                        if (Word.text() == "nan")
                        {
                            Out = Negative ? (detail::CanonicalQuietNaNBits | (std::uint64_t{1} << 63)) : detail::CanonicalQuietNaNBits;
                            return true;
                        }
                        return fail(StartBegin, Word.EndOffset, "expected an IEEE 754 binary64 floating-point value");
                    }
                    if (ValueToken.Kind != TokenKind::Number)
                    {
                        return fail(StartBegin, ValueToken.EndOffset, "expected an IEEE 754 binary64 floating-point value");
                    }
                    const Token Number = Lexer.take();
                    std::string_view Digits = Number.text();
                    double Value = 0.0;
                    std::from_chars_result Result{};
                    if (Digits.starts_with("0x") || Digits.starts_with("0X"))
                    {
                        if (Digits.find('p') == std::string_view::npos && Digits.find('P') == std::string_view::npos)
                        {
                            return fail(StartBegin, Number.EndOffset, "hexadecimal .f64 literals require a binary exponent 'p'; use .i64 for raw 64-bit bit patterns");
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
                        return fail(StartBegin, Number.EndOffset, "expected an IEEE 754 binary64 floating-point value");
                    }
                    if (Result.ec == std::errc::result_out_of_range)
                    {
                        return fail(StartBegin, Number.EndOffset, "floating-point literal is out of IEEE 754 binary64 range");
                    }
                    if (Result.ec != std::errc{})
                    {
                        return fail(StartBegin, Number.EndOffset, "expected an IEEE 754 binary64 floating-point value");
                    }
                    if (Negative)
                    {
                        Value = -Value;
                    }
                    Out = std::bit_cast<std::uint64_t>(Value);
                    return true;
                }

                bool parseOperand(MachineOperand& Out)
                {
                    const Token& Next = Lexer.peek();
                    if (Next.Kind == TokenKind::Error)
                    {
                        return false;
                    }
                    if (Next.Kind == TokenKind::LeftBracket)
                    {
                        Lexer.consume();
                        std::optional<RegisterId> BaseRegister;
                        core::Name BaseSymbol;
                        if (Lexer.at(TokenKind::Symbol))
                        {
                            if (!parseSymbol(BaseSymbol))
                            {
                                return false;
                            }
                        }
                        else if (Lexer.hasError())
                        {
                            return false;
                        }
                        else
                        {
                            Token BaseToken;
                            if (!expect(TokenKind::Identifier, "expected an address base register or symbol", BaseToken))
                            {
                                return false;
                            }
                            BaseRegister = parseRegister(BaseToken.text());
                            if (!BaseRegister || !isAddressRegister(*BaseRegister))
                            {
                                return fail(BaseToken.BeginOffset, BaseToken.EndOffset, "unknown or invalid address base register");
                            }
                        }
                        std::int64_t Offset = 0;
                        if (!parseOptionalDisplacement(Offset))
                        {
                            return false;
                        }
                        Token Closing;
                        if (!expect(TokenKind::RightBracket, "expected ']' after memory address", Closing))
                        {
                            return false;
                        }
                        Out = BaseRegister ? MachineOperand(AddressOperand(*BaseRegister, Offset)) : MachineOperand(AddressOperand(BaseSymbol, Offset));
                        return true;
                    }
                    if (Next.Kind == TokenKind::Symbol)
                    {
                        core::Name Symbol;
                        std::int64_t Offset = 0;
                        if (!parseSymbol(Symbol) || !parseOptionalDisplacement(Offset))
                        {
                            return false;
                        }
                        Out = MachineOperand(ImmediateOperand(Symbol, Offset));
                        return true;
                    }
                    if (Next.Kind == TokenKind::Identifier)
                    {
                        const Token RegisterToken = Lexer.take();
                        const auto Register = parseRegister(RegisterToken.text());
                        if (!Register || !isArithmeticRegister(*Register))
                        {
                            return fail(RegisterToken.BeginOffset, RegisterToken.EndOffset, "unknown register operand");
                        }
                        std::int64_t Offset = 0;
                        if (!parseOptionalDisplacement(Offset))
                        {
                            return false;
                        }
                        Out = MachineOperand(RegisterOperand(*Register, Offset));
                        return true;
                    }
                    if (Next.Kind == TokenKind::Number || Next.Kind == TokenKind::Plus || Next.Kind == TokenKind::Minus)
                    {
                        std::uint64_t Bits = 0;
                        if (!parseSignedOrUnsignedInteger(64, Bits, "expected an immediate integer"))
                        {
                            return false;
                        }
                        Out = MachineOperand(ImmediateOperand(Bits));
                        return true;
                    }
                    return fail(Next.BeginOffset, Next.EndOffset, "expected an instruction operand");
                }

                bool parseModule()
                {
                    Lexer.skipBlankLines();
                    if (Lexer.hasError())
                    {
                        return false;
                    }
                    if (Lexer.at(TokenKind::EndOfFile))
                    {
                        return fail(Lexer.endOffset(), "missing .module directive");
                    }
                    if (!parseModuleHeader())
                    {
                        return false;
                    }
                    for (;;)
                    {
                        Lexer.skipBlankLines();
                        if (Lexer.hasError())
                        {
                            return false;
                        }
                        if (Lexer.at(TokenKind::EndOfFile))
                        {
                            return true;
                        }
                        if (!parseModuleTopLevelLine())
                        {
                            return false;
                        }
                    }
                }

                bool parseModuleHeader()
                {
                    Token First;
                    if (!expect(TokenKind::Identifier, "expected .module followed by a quoted name", First))
                    {
                        return false;
                    }
                    if (First.text() != detail::ModuleDirective)
                    {
                        return fail(First.BeginOffset, First.EndOffset, "expected .module followed by a quoted name");
                    }
                    Token NameToken;
                    if (!expect(TokenKind::String, "expected a quoted module name", NameToken))
                    {
                        return false;
                    }
                    if (!expectEndOfLine("unexpected token after .module header"))
                    {
                        return false;
                    }
                    core::Name ModuleName;
                    if (!internName(NameToken.text(), NameToken.BeginOffset, NameToken.EndOffset, ModuleName))
                    {
                        return false;
                    }
                    Module = std::make_unique<MachineModule>(ModuleName);
                    return true;
                }

                bool parseModuleTopLevelLine()
                {
                    Token First;
                    if (!expect(TokenKind::Identifier, "expected a top-level directive", First))
                    {
                        return false;
                    }
                    const std::string_view Directive = First.text();
                    if (Directive == detail::DeclareDirective)
                    {
                        return parseSymbolDeclaration(First);
                    }
                    if (Directive == detail::SectionDirective)
                    {
                        return parseSectionDirective();
                    }
                    if (Directive == detail::GlobalDirective)
                    {
                        return parseGlobalDefinition(First);
                    }
                    if (Directive == detail::FunctionDirective)
                    {
                        return parseFunctionDefinition(First);
                    }
                    return fail(First.BeginOffset, First.EndOffset, "unexpected or misplaced top-level directive");
                }

                bool parseSymbolDeclaration(const Token& Directive)
                {
                    if (HasSeenSection)
                    {
                        return fail(Directive.BeginOffset, Directive.EndOffset, ".declare directives must precede all .section directives");
                    }
                    Token KindToken;
                    if (!expect(TokenKind::Identifier, "expected 'function' or 'global' after .declare", KindToken))
                    {
                        return false;
                    }
                    const auto Kind = detail::parseSymbolKind(KindToken.text());
                    if (!Kind)
                    {
                        return fail(KindToken.BeginOffset, KindToken.EndOffset, "unknown symbol declaration kind");
                    }
                    core::Name Symbol;
                    if (!parseSymbol(Symbol))
                    {
                        return false;
                    }
                    bool IsConstant = false;
                    if (const auto Attribute = Lexer.eatToken(TokenKind::Identifier))
                    {
                        if (Attribute->text() != detail::ReadOnlyKeyword)
                        {
                            return fail(Attribute->BeginOffset, Attribute->EndOffset, "unknown symbol declaration attribute; expected 'readonly'");
                        }
                        if (*Kind != MachineSymbolKind::GlobalVariable)
                        {
                            return fail(Attribute->BeginOffset, Attribute->EndOffset, "only global declarations accept the readonly attribute");
                        }
                        IsConstant = true;
                    }
                    if (!expectEndOfLine("unexpected token after the symbol declaration"))
                    {
                        return false;
                    }
                    Module->declareSymbol(Symbol, *Kind, IsConstant);
                    return true;
                }

                bool parseSectionDirective()
                {
                    Token SectionToken;
                    if (!expect(TokenKind::Identifier, "section directive requires .data or .text", SectionToken))
                    {
                        return false;
                    }
                    const auto Kind = detail::parseSectionKind(SectionToken.text());
                    if (!Kind)
                    {
                        return fail(SectionToken.BeginOffset, SectionToken.EndOffset, "unknown section");
                    }
                    if (!expectEndOfLine("unexpected token after .section directive"))
                    {
                        return false;
                    }
                    CurrentSection = *Kind;
                    HasSeenSection = true;
                    return true;
                }

                bool parseGlobalDefinition(const Token& Directive)
                {
                    if (CurrentSection != detail::SectionKind::Data)
                    {
                        return fail(Directive.BeginOffset, Directive.EndOffset, "global definition must appear in the .data section");
                    }
                    core::Name GlobalName = core::NAME_None;
                    if (Lexer.at(TokenKind::Symbol))
                    {
                        if (!parseSymbol(GlobalName))
                        {
                            return false;
                        }
                    }
                    else if (Lexer.hasError())
                    {
                        return false;
                    }
                    std::optional<std::uint64_t> ByteSize;
                    std::optional<std::uint64_t> Alignment;
                    std::optional<MachineSymbolLinkage> Linkage;
                    bool IsConstant = false;
                    while (!Lexer.atLineEnd())
                    {
                        Token Attr;
                        if (!expect(TokenKind::Identifier, "expected global attribute 'size', 'align', 'readonly', 'local', or 'external'", Attr))
                        {
                            return false;
                        }
                        const std::string_view Keyword = Attr.text();
                        if (Keyword == detail::SizeKeyword)
                        {
                            if (ByteSize)
                            {
                                return fail(Attr.BeginOffset, Attr.EndOffset, "duplicate 'size' attribute on global definition");
                            }
                            std::uint64_t ParsedSize = 0;
                            if (!parseUnsignedInteger(ParsedSize, "expected global byte size after 'size'"))
                            {
                                return false;
                            }
                            ByteSize = ParsedSize;
                        }
                        else if (Keyword == detail::AlignKeyword)
                        {
                            if (Alignment)
                            {
                                return fail(Attr.BeginOffset, Attr.EndOffset, "duplicate 'align' attribute on global definition");
                            }
                            const Token& AlignToken = Lexer.peek();
                            const std::size_t AlignBegin = AlignToken.BeginOffset;
                            const std::size_t AlignEnd = AlignToken.EndOffset;
                            std::uint64_t ParsedAlign = 0;
                            if (!parseUnsignedInteger(ParsedAlign, "expected global alignment after 'align'"))
                            {
                                return false;
                            }
                            if (!std::has_single_bit(ParsedAlign))
                            {
                                return fail(AlignBegin, AlignEnd, "data alignment must be a nonzero power of two");
                            }
                            Alignment = ParsedAlign;
                        }
                        else if (Keyword == detail::ReadOnlyKeyword)
                        {
                            if (IsConstant)
                            {
                                return fail(Attr.BeginOffset, Attr.EndOffset, "duplicate 'readonly' attribute on global definition");
                            }
                            IsConstant = true;
                        }
                        else if (const auto ParsedLinkage = detail::parseLinkage(Keyword))
                        {
                            if (Linkage)
                            {
                                return fail(Attr.BeginOffset, Attr.EndOffset, "duplicate linkage attribute on global definition");
                            }
                            if (*ParsedLinkage == MachineSymbolLinkage::External && !GlobalName.valid())
                            {
                                return fail(Attr.BeginOffset, Attr.EndOffset, "external definitions require a nonempty symbol name");
                            }
                            Linkage = *ParsedLinkage;
                        }
                        else
                        {
                            return fail(Attr.BeginOffset, Attr.EndOffset, "unknown global attribute; expected 'size', 'align', 'readonly', 'local', or 'external'");
                        }
                    }
                    if (!expectEndOfLine())
                    {
                        return false;
                    }
                    if (!ByteSize)
                    {
                        return fail(Directive.BeginOffset, Directive.EndOffset, "global definition requires a 'size' attribute");
                    }
                    const std::uint64_t ResolvedAlignment = Alignment.value_or(1);
                    const MachineSymbolLinkage ResolvedLinkage = Linkage.value_or(MachineSymbolLinkage::Local);

                    std::vector<std::uint8_t> InitialBytes;
                    std::vector<MachineGlobalRelocation> Relocations;
                    std::vector<ParsedRelocationSpan> RelocationSpans;
                    std::unordered_set<std::uint64_t> SeenRelocationOffsets;
                    for (;;)
                    {
                        Lexer.skipBlankLines();
                        if (Lexer.hasError())
                        {
                            return false;
                        }
                        if (Lexer.at(TokenKind::EndOfFile))
                        {
                            return fail(Lexer.endOffset(), "unterminated global variable definition");
                        }
                        Token BodyDirective;
                        if (!expect(TokenKind::Identifier, "expected a global data directive or .endglobal", BodyDirective))
                        {
                            return false;
                        }
                        const std::string_view Keyword = BodyDirective.text();
                        if (Keyword == detail::ByteDirective)
                        {
                            if (!parseByteDirective(*ByteSize, InitialBytes))
                            {
                                return false;
                            }
                        }
                        else if (Keyword == detail::BytesDirective)
                        {
                            if (!parseBytesDirective(*ByteSize, InitialBytes))
                            {
                                return false;
                            }
                        }
                        else if (Keyword == detail::ZeroDirective)
                        {
                            if (!parseZeroDirective(*ByteSize, InitialBytes))
                            {
                                return false;
                            }
                        }
                        else if (Keyword == detail::I16Directive)
                        {
                            if (!parseIntDataDirective(16, *ByteSize, InitialBytes))
                            {
                                return false;
                            }
                        }
                        else if (Keyword == detail::I32Directive)
                        {
                            if (!parseIntDataDirective(32, *ByteSize, InitialBytes))
                            {
                                return false;
                            }
                        }
                        else if (Keyword == detail::I64Directive)
                        {
                            if (!parseIntDataDirective(64, *ByteSize, InitialBytes))
                            {
                                return false;
                            }
                        }
                        else if (Keyword == detail::F64Directive)
                        {
                            if (!parseFloatDataDirective(*ByteSize, InitialBytes))
                            {
                                return false;
                            }
                        }
                        else if (Keyword == detail::RelocDirective)
                        {
                            if (!parseRelocDirective(*ByteSize, ResolvedAlignment, Relocations, RelocationSpans, SeenRelocationOffsets))
                            {
                                return false;
                            }
                        }
                        else if (Keyword == detail::EndGlobalDirective)
                        {
                            if (!expectEndOfLine("unexpected token after .endglobal"))
                            {
                                return false;
                            }
                            if (!verifyRelocationsAgainstInitialBytes(InitialBytes, Relocations, RelocationSpans))
                            {
                                return false;
                            }
                            const auto Created = Module->createGlobalVariable(*ByteSize, ResolvedAlignment, InitialBytes, GlobalName, ResolvedLinkage, IsConstant, Relocations);
                            if (!Created)
                            {
                                return fail(BodyDirective.BeginOffset, BodyDirective.EndOffset, "invalid global data layout");
                            }
                            return true;
                        }
                        else
                        {
                            return fail(BodyDirective.BeginOffset, BodyDirective.EndOffset, "expected a global data directive or .endglobal");
                        }
                    }
                }

                bool verifyRelocationsAgainstInitialBytes(std::span<const std::uint8_t> InitialBytes, std::span<const MachineGlobalRelocation> Relocations, std::span<const ParsedRelocationSpan> Spans)
                {
                    for (std::size_t Index = 0; Index < Relocations.size(); ++Index)
                    {
                        const std::uint64_t ByteOffset = Relocations[Index].ByteOffset;
                        if (!std::cmp_less(ByteOffset, InitialBytes.size()))
                        {
                            continue;
                        }
                        const auto Start = static_cast<std::size_t>(ByteOffset);
                        const std::size_t OverlapCount = std::min<std::size_t>(InitialBytes.size() - Start, static_cast<std::size_t>(MachineGlobalRelocation::ByteSize));
                        for (std::size_t Offset = 0; Offset < OverlapCount; ++Offset)
                        {
                            if (InitialBytes[Start + Offset] != 0)
                            {
                                return fail(Spans[Index].BeginOffset, Spans[Index].EndOffset, "global relocation overlaps nonzero initializer bytes");
                            }
                        }
                    }
                    return true;
                }

                bool parseByteDirective(std::uint64_t DeclaredSize, std::vector<std::uint8_t>& InitialBytes)
                {
                    do
                    {
                        const Token& ValueToken = Lexer.peek();
                        const std::size_t BeginOffset = ValueToken.BeginOffset;
                        const std::size_t EndOffset = ValueToken.EndOffset;
                        std::uint64_t ByteValue = 0;
                        if (!parseUnsignedInteger(ByteValue, "expected a byte value in the range 0 through 255"))
                        {
                            return false;
                        }
                        if (ByteValue > 255)
                        {
                            return fail(BeginOffset, EndOffset, "expected a byte value in the range 0 through 255");
                        }
                        if (InitialBytes.size() >= DeclaredSize)
                        {
                            return fail(BeginOffset, EndOffset, "global initializer exceeds its declared byte size");
                        }
                        InitialBytes.push_back(static_cast<std::uint8_t>(ByteValue));
                    } while (Lexer.eat(TokenKind::Comma));
                    return expectEndOfLine("unexpected token after byte value");
                }

                bool parseBytesDirective(std::uint64_t DeclaredSize, std::vector<std::uint8_t>& InitialBytes)
                {
                    Token StringToken;
                    if (!expect(TokenKind::String, "expected a quoted byte string after .bytes", StringToken))
                    {
                        return false;
                    }
                    if (!expectEndOfLine("unexpected token after .bytes literal"))
                    {
                        return false;
                    }
                    const std::string_view Bytes = StringToken.text();
                    if (Bytes.size() > DeclaredSize - InitialBytes.size())
                    {
                        return fail(StringToken.BeginOffset, StringToken.EndOffset, "global initializer exceeds its declared byte size");
                    }
                    InitialBytes.reserve(InitialBytes.size() + Bytes.size());
                    for (const char Byte : Bytes)
                    {
                        InitialBytes.push_back(static_cast<std::uint8_t>(Byte));
                    }
                    return true;
                }

                bool parseZeroDirective(std::uint64_t DeclaredSize, std::vector<std::uint8_t>& InitialBytes)
                {
                    const Token& CountToken = Lexer.peek();
                    const std::size_t BeginOffset = CountToken.BeginOffset;
                    const std::size_t EndOffset = CountToken.EndOffset;
                    std::uint64_t Count = 0;
                    if (!parseUnsignedInteger(Count, "expected an unsigned zero-byte count after .zero"))
                    {
                        return false;
                    }
                    if (!expectEndOfLine("unexpected token after .zero count"))
                    {
                        return false;
                    }
                    if (Count > DeclaredSize - InitialBytes.size())
                    {
                        return fail(BeginOffset, EndOffset, "global initializer exceeds its declared byte size");
                    }
                    if (Count > detail::MaxMaterializedInitializerBytes - std::min(InitialBytes.size(), detail::MaxMaterializedInitializerBytes) || Count > InitialBytes.max_size() - InitialBytes.size())
                    {
                        return fail(BeginOffset, EndOffset, "global initializer exceeds supported materialized storage limits");
                    }
                    InitialBytes.insert(InitialBytes.end(), static_cast<std::size_t>(Count), std::uint8_t{0});
                    return true;
                }

                bool parseIntDataDirective(std::uint32_t BitWidth, std::uint64_t DeclaredSize, std::vector<std::uint8_t>& InitialBytes)
                {
                    const std::size_t ByteWidth = BitWidth / 8;
                    do
                    {
                        const std::size_t BeginOffset = Lexer.peek().BeginOffset;
                        std::uint64_t Bits = 0;
                        if (!parseSignedOrUnsignedInteger(BitWidth, Bits, "expected an integer value"))
                        {
                            return false;
                        }
                        if (ByteWidth > DeclaredSize - InitialBytes.size())
                        {
                            return fail(BeginOffset, Lexer.lastTokenEnd(), "global initializer exceeds its declared byte size");
                        }
                        for (std::size_t ByteIndex = 0; ByteIndex < ByteWidth; ++ByteIndex)
                        {
                            InitialBytes.push_back(static_cast<std::uint8_t>((Bits >> (ByteIndex * 8)) & 0xFFU));
                        }
                    } while (Lexer.eat(TokenKind::Comma));
                    return expectEndOfLine("unexpected token after integer value");
                }

                bool parseFloatDataDirective(std::uint64_t DeclaredSize, std::vector<std::uint8_t>& InitialBytes)
                {
                    do
                    {
                        const std::size_t BeginOffset = Lexer.peek().BeginOffset;
                        std::uint64_t Bits = 0;
                        if (!parseFloat64Bits(Bits))
                        {
                            return false;
                        }
                        if (8 > DeclaredSize - InitialBytes.size())
                        {
                            return fail(BeginOffset, Lexer.lastTokenEnd(), "global initializer exceeds its declared byte size");
                        }
                        for (std::size_t ByteIndex = 0; ByteIndex < 8; ++ByteIndex)
                        {
                            InitialBytes.push_back(static_cast<std::uint8_t>((Bits >> (ByteIndex * 8)) & 0xFFU));
                        }
                    } while (Lexer.eat(TokenKind::Comma));
                    return expectEndOfLine("unexpected token after floating-point value");
                }

                bool parseRelocDirective(std::uint64_t DeclaredSize, std::uint64_t Alignment, std::vector<MachineGlobalRelocation>& Relocations, std::vector<ParsedRelocationSpan>& RelocationSpans, std::unordered_set<std::uint64_t>& SeenRelocationOffsets)
                {
                    const Token& OffsetToken = Lexer.peek();
                    const std::size_t OffsetBegin = OffsetToken.BeginOffset;
                    const std::size_t OffsetEnd = OffsetToken.EndOffset;
                    std::uint64_t ByteOffset = 0;
                    if (!parseUnsignedInteger(ByteOffset, "expected a relocation byte offset"))
                    {
                        return false;
                    }
                    Token Comma;
                    if (!expect(TokenKind::Comma, "expected ',' after relocation byte offset", Comma))
                    {
                        return false;
                    }
                    core::Name Symbol;
                    std::int64_t Addend = 0;
                    if (!parseSymbol(Symbol) || !parseOptionalDisplacement(Addend))
                    {
                        return false;
                    }
                    if (!expectEndOfLine("unexpected token after global relocation"))
                    {
                        return false;
                    }
                    if (Alignment < MachineGlobalRelocation::ByteSize || ByteOffset % MachineGlobalRelocation::ByteSize != 0)
                    {
                        return fail(OffsetBegin, OffsetEnd, "global relocations require alignment >= 8 and an eight-byte aligned byte offset");
                    }
                    if (ByteOffset > DeclaredSize || MachineGlobalRelocation::ByteSize > DeclaredSize - ByteOffset)
                    {
                        return fail(OffsetBegin, OffsetEnd, "global relocation exceeds its declared byte size");
                    }
                    if (!SeenRelocationOffsets.insert(ByteOffset).second)
                    {
                        return fail(OffsetBegin, OffsetEnd, "global relocation overlaps an earlier relocation");
                    }
                    Relocations.push_back({ByteOffset, Symbol, Addend});
                    RelocationSpans.push_back({OffsetBegin, OffsetEnd});
                    return true;
                }

                bool parseFunctionDefinition(const Token& Directive)
                {
                    if (CurrentSection != detail::SectionKind::Text)
                    {
                        return fail(Directive.BeginOffset, Directive.EndOffset, "function definition must appear in the .text section");
                    }
                    core::Name FunctionName = core::NAME_None;
                    if (Lexer.at(TokenKind::Symbol))
                    {
                        if (!parseSymbol(FunctionName))
                        {
                            return false;
                        }
                    }
                    else if (Lexer.hasError())
                    {
                        return false;
                    }
                    MachineSymbolLinkage Linkage = MachineSymbolLinkage::Local;
                    if (const auto LinkageToken = Lexer.eatToken(TokenKind::Identifier))
                    {
                        const auto Parsed = detail::parseLinkage(LinkageToken->text());
                        if (!Parsed)
                        {
                            return fail(LinkageToken->BeginOffset, LinkageToken->EndOffset, "expected 'local' or 'external' linkage");
                        }
                        if (*Parsed == MachineSymbolLinkage::External && !FunctionName.valid())
                        {
                            return fail(LinkageToken->BeginOffset, LinkageToken->EndOffset, "external definitions require a nonempty symbol name");
                        }
                        Linkage = *Parsed;
                    }
                    if (!expectEndOfLine("unexpected token after .function header"))
                    {
                        return false;
                    }

                    MachineFunction& Function = Module->createFunction(FunctionName, Linkage);
                    FunctionParseState State{Function};
                    for (;;)
                    {
                        Lexer.skipBlankLines();
                        if (Lexer.hasError())
                        {
                            return false;
                        }
                        if (Lexer.at(TokenKind::EndOfFile))
                        {
                            return fail(Lexer.endOffset(), "unterminated function definition");
                        }
                        Token First;
                        if (!expect(TokenKind::Identifier, "expected a directive, label, or instruction", First))
                        {
                            return false;
                        }
                        if (Lexer.eat(TokenKind::Colon))
                        {
                            if (!parseBasicBlock(State, First))
                            {
                                return false;
                            }
                            continue;
                        }
                        if (Lexer.hasError())
                        {
                            return false;
                        }
                        const std::string_view Keyword = First.text();
                        if (Keyword == detail::StackSlotDirective)
                        {
                            if (!parseStackSlot(State, First))
                            {
                                return false;
                            }
                        }
                        else if (Keyword == detail::FrameDirective)
                        {
                            if (!parseFrameLayout(State, First))
                            {
                                return false;
                            }
                        }
                        else if (Keyword == detail::EndFunctionDirective)
                        {
                            if (!expectEndOfLine("unexpected token after .endfunction"))
                            {
                                return false;
                            }
                            return verifyForwardBlockReferences(State);
                        }
                        else if (Keyword.starts_with('.'))
                        {
                            return fail(First.BeginOffset, First.EndOffset, "unknown or misplaced directive inside function");
                        }
                        else if (!parseInstruction(State, First))
                        {
                            return false;
                        }
                    }
                }

                bool parseStackSlot(FunctionParseState& State, const Token& Directive)
                {
                    if (State.HasBlock || State.HasFrame)
                    {
                        return fail(Directive.BeginOffset, Directive.EndOffset, "stack slots and one final frame layout must precede all basic blocks");
                    }
                    std::optional<std::uint64_t> SlotSize;
                    std::optional<std::uint64_t> Alignment;
                    while (!Lexer.atLineEnd())
                    {
                        Token Attr;
                        if (!expect(TokenKind::Identifier, "expected stack slot attribute 'size' or 'align'", Attr))
                        {
                            return false;
                        }
                        if (Attr.text() == detail::SizeKeyword)
                        {
                            if (SlotSize)
                            {
                                return fail(Attr.BeginOffset, Attr.EndOffset, "duplicate 'size' attribute on .stackslot");
                            }
                            std::uint64_t ParsedSize = 0;
                            if (!parseUnsignedInteger(ParsedSize, "expected stack slot byte size after 'size'"))
                            {
                                return false;
                            }
                            SlotSize = ParsedSize;
                        }
                        else if (Attr.text() == detail::AlignKeyword)
                        {
                            if (Alignment)
                            {
                                return fail(Attr.BeginOffset, Attr.EndOffset, "duplicate 'align' attribute on .stackslot");
                            }
                            const Token& AlignToken = Lexer.peek();
                            const std::size_t AlignBegin = AlignToken.BeginOffset;
                            const std::size_t AlignEnd = AlignToken.EndOffset;
                            std::uint64_t ParsedAlign = 0;
                            if (!parseUnsignedInteger(ParsedAlign, "expected stack slot alignment after 'align'"))
                            {
                                return false;
                            }
                            if (!std::has_single_bit(ParsedAlign))
                            {
                                return fail(AlignBegin, AlignEnd, "stack slot alignment must be a nonzero power of two");
                            }
                            Alignment = ParsedAlign;
                        }
                        else
                        {
                            return fail(Attr.BeginOffset, Attr.EndOffset, "unknown .stackslot attribute; expected 'size' or 'align'");
                        }
                    }
                    if (!expectEndOfLine())
                    {
                        return false;
                    }
                    if (!SlotSize)
                    {
                        return fail(Directive.BeginOffset, Directive.EndOffset, ".stackslot requires a 'size' attribute");
                    }
                    if (!State.Function.frameInfo().createStackSlot(*SlotSize, Alignment.value_or(1)))
                    {
                        return fail(Directive.BeginOffset, Directive.EndOffset, "invalid stack slot size or alignment");
                    }
                    return true;
                }

                bool parseFrameLayout(FunctionParseState& State, const Token& Directive)
                {
                    if (State.HasBlock || State.HasFrame)
                    {
                        return fail(Directive.BeginOffset, Directive.EndOffset, "stack slots and one final frame layout must precede all basic blocks");
                    }
                    Token BaseKeywordToken;
                    if (!expect(TokenKind::Identifier, "expected 'base' after .frame", BaseKeywordToken))
                    {
                        return false;
                    }
                    if (BaseKeywordToken.text() != detail::BaseKeyword)
                    {
                        return fail(BaseKeywordToken.BeginOffset, BaseKeywordToken.EndOffset, "expected 'base' after .frame");
                    }
                    const Token& BaseToken = Lexer.peek();
                    const std::size_t BaseBegin = BaseToken.BeginOffset;
                    const std::size_t BaseEnd = BaseToken.EndOffset;
                    std::uint64_t BaseOffset = 0;
                    if (!parseUnsignedInteger(BaseOffset, "expected frame layout base offset after 'base'"))
                    {
                        return false;
                    }
                    if (!expectEndOfLine("unexpected token after .frame directive"))
                    {
                        return false;
                    }
                    if (!State.Function.frameInfo().layout(BaseOffset))
                    {
                        return fail(BaseBegin, BaseEnd, "frame layout overflows unsigned 64-bit byte range");
                    }
                    State.HasFrame = true;
                    return true;
                }

                bool parseBasicBlock(FunctionParseState& State, const Token& LabelToken)
                {
                    const auto AliasToken = Lexer.eatToken(TokenKind::String);
                    if (!expectEndOfLine("a block label accepts at most one quoted original name"))
                    {
                        return false;
                    }
                    const Token& NameSource = AliasToken ? *AliasToken : LabelToken;
                    core::Name BlockName;
                    if (!internName(NameSource.text(), NameSource.BeginOffset, NameSource.EndOffset, BlockName))
                    {
                        return false;
                    }
                    const auto Found = State.BlockLabels.find(LabelToken.text());
                    if (Found != State.BlockLabels.end())
                    {
                        if (Found->second.Defined)
                        {
                            return fail(LabelToken.BeginOffset, LabelToken.EndOffset, "duplicate block label in the function");
                        }
                        Found->second.Block->setName(BlockName);
                        static_cast<void>(State.Function.moveBasicBlockToEnd(*Found->second.Block));
                        Found->second.Defined = true;
                        State.CurrentBlock = Found->second.Block;
                    }
                    else
                    {
                        MachineBasicBlock& Block = State.Function.createBasicBlock(BlockName);
                        State.BlockLabels.emplace(LabelToken.text(), BlockLabelEntry{&Block, true});
                        State.CurrentBlock = &Block;
                    }
                    State.HasBlock = true;
                    State.Builder.setInsertPoint(*State.CurrentBlock);
                    return true;
                }

                MachineBasicBlock* resolveBranchTarget(FunctionParseState& State, const Token& TargetToken)
                {
                    const auto Found = State.BlockLabels.find(TargetToken.text());
                    if (Found != State.BlockLabels.end())
                    {
                        return Found->second.Block;
                    }
                    MachineBasicBlock& Placeholder = State.Function.createBasicBlock(core::NAME_None);
                    State.BlockLabels.emplace(TargetToken.text(), BlockLabelEntry{&Placeholder, false});
                    State.ForwardRefs.push_back({TargetToken.text(), TargetToken.BeginOffset, TargetToken.EndOffset});
                    return &Placeholder;
                }

                bool parseInstruction(FunctionParseState& State, const Token& Mnemonic)
                {
                    if (!State.CurrentBlock)
                    {
                        return fail(Mnemonic.BeginOffset, Mnemonic.EndOffset, "an instruction requires a basic block");
                    }
                    const auto Overloads = findOpcodeOverloads(Mnemonic.text());
                    if (Overloads.empty())
                    {
                        return fail(Mnemonic.BeginOffset, Mnemonic.EndOffset, "unknown instruction mnemonic");
                    }
                    std::array<MachineOperand, InstructionEncoding::MaxOperands> Operands{};
                    std::array<OperandKind, InstructionEncoding::MaxOperands> Kinds{};
                    std::size_t OperandCount = 0;
                    Token TargetToken;
                    const bool HasBlockTarget = Overloads.front().HasBlockTarget;
                    if (HasBlockTarget)
                    {
                        const std::size_t TargetIndex = static_cast<std::size_t>(Overloads.front().OperandCount - 1);
                        for (std::size_t Index = 0; Index < TargetIndex; ++Index)
                        {
                            if (!parseOperand(Operands[Index]))
                            {
                                return false;
                            }
                            Token Comma;
                            if (!expect(TokenKind::Comma, "expected ',' after branch comparison operand", Comma))
                            {
                                return false;
                            }
                            Kinds[Index] = Operands[Index].getKind();
                        }
                        if (!expect(TokenKind::Identifier, "a branch requires a final basic block label", TargetToken))
                        {
                            return false;
                        }
                        if (!expectEndOfLine("unexpected token after basic block target"))
                        {
                            return false;
                        }
                        Operands[TargetIndex] = MachineOperand(BasicBlockOperand(nullptr));
                        Kinds[TargetIndex] = OperandKind::BasicBlock;
                        OperandCount = TargetIndex + 1;
                    }
                    else
                    {
                        if (!Lexer.atLineEnd())
                        {
                            do
                            {
                                const Token& Start = Lexer.peek();
                                if (Start.Kind == TokenKind::Error)
                                {
                                    return false;
                                }
                                if (OperandCount == Operands.size())
                                {
                                    return fail(Start.BeginOffset, Start.EndOffset, "too many instruction operands");
                                }
                                if (!parseOperand(Operands[OperandCount]))
                                {
                                    return false;
                                }
                                Kinds[OperandCount] = Operands[OperandCount].getKind();
                                ++OperandCount;
                            } while (Lexer.eat(TokenKind::Comma));
                        }
                        if (!expectEndOfLine("expected ',' between instruction operands"))
                        {
                            return false;
                        }
                    }

                    const OpcodeInfo* Matched = nullptr;
                    const std::span<const OperandKind> KindSpan(Kinds.data(), OperandCount);
                    for (const OpcodeInfo& Candidate : Overloads)
                    {
                        if (Candidate.matchesOperandKinds(KindSpan))
                        {
                            Matched = &Candidate;
                            break;
                        }
                    }
                    if (!Matched)
                    {
                        return fail(Mnemonic.BeginOffset, Mnemonic.EndOffset, "operand types or count do not match instruction mnemonic");
                    }
                    if (const auto ValidationError = validateInstructionConstraints(*Matched, {Operands.data(), OperandCount}))
                    {
                        return fail(Mnemonic.BeginOffset, Mnemonic.EndOffset, *ValidationError);
                    }
                    if (HasBlockTarget)
                    {
                        Operands[OperandCount - 1] = MachineOperand(BasicBlockOperand(resolveBranchTarget(State, TargetToken)));
                    }
                    if (!State.Builder.tryCreate(Matched->Operation, {Operands.data(), OperandCount}))
                    {
                        return fail(Mnemonic.BeginOffset, Mnemonic.EndOffset, "instruction operands cannot be inserted into their function");
                    }
                    return true;
                }

                bool verifyForwardBlockReferences(const FunctionParseState& State)
                {
                    for (const ForwardBlockRef& Reference : State.ForwardRefs)
                    {
                        const auto Found = State.BlockLabels.find(Reference.Label);
                        if (Found == State.BlockLabels.end() || !Found->second.Defined)
                        {
                            return fail(Reference.BeginOffset, Reference.EndOffset, "unknown basic block label in this function");
                        }
                    }
                    return true;
                }

                std::optional<core::Diagnostic> Error;
                AsmLexer Lexer;
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
        return ModuleTextParser(Text).parse();
    }
} // namespace ink::vm
