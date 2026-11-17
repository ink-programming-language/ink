#include "ink/backend/vm/serialization/machine_module_text.h"

#include "machine_module_text_format.h"

#include "ink/backend/vm/instruction/register.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/backend/vm/module/machine_module.h"

#include <array>
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
#include <unordered_set>
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
        };

        struct Token
        {
                TokenKind Kind;
                std::string_view Spelling;
                std::string DecodedString;
                std::size_t Offset;

                std::string_view text() const noexcept
                {
                    return Kind == TokenKind::String ? std::string_view(DecodedString) : Spelling;
                }
        };

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

        ParseResult<std::vector<Token>> tokenizeLine(std::string_view Line, std::size_t LineOffset)
        {
            std::vector<Token> Tokens;
            for (std::size_t Index = 0; Index < Line.size();)
            {
                const char Character = Line[Index];
                if (Character == ' ' || Character == '\t' || Character == '\r')
                {
                    ++Index;
                    continue;
                }
                if (Character == ';')
                {
                    break;
                }
                const std::size_t Start = Index++;
                if (Character == '"')
                {
                    auto Decoded = detail::decodeQuotedText(Line, Start, Index, LineOffset);
                    if (!Decoded)
                    {
                        return std::unexpected(Decoded.error());
                    }
                    Tokens.push_back({TokenKind::String, Line.substr(Start, Index - Start), std::move(*Decoded), LineOffset + Start});
                    continue;
                }
                if (detail::isDecimalDigit(Character) || detail::isIdentifierStart(Character))
                {
                    while (Index < Line.size() && detail::isIdentifierContinue(Line[Index]))
                    {
                        ++Index;
                    }
                    const TokenKind Kind = detail::isDecimalDigit(Character) ? TokenKind::Number : TokenKind::Identifier;
                    Tokens.push_back({Kind, Line.substr(Start, Index - Start), {}, LineOffset + Start});
                    continue;
                }
                const auto Punctuation = matchPunctuation(Character);
                if (!Punctuation)
                {
                    return std::unexpected(detail::makeTextDiagnostic(LineOffset + Start, "unexpected character"));
                }
                Tokens.push_back({*Punctuation, Line.substr(Start, 1), {}, LineOffset + Start});
            }
            return Tokens;
        }

        struct DeclaredLabel
        {
                std::string_view Label;
                std::size_t Offset;
                core::Name OriginalName;
        };

        class LineCursor
        {
            public:
                LineCursor(std::vector<Token> Tokens, std::size_t EndOffset) noexcept
                    : Tokens(std::move(Tokens)),
                      EndOffset(EndOffset)
                {
                }

                const Token* peek() const noexcept
                {
                    return Position < Tokens.size() ? &Tokens[Position] : nullptr;
                }

                std::size_t offset() const noexcept
                {
                    return peek() ? peek()->Offset : EndOffset;
                }

                bool at(TokenKind Kind) const noexcept
                {
                    return peek() && peek()->Kind == Kind;
                }

                bool atKeyword(std::string_view Keyword) const noexcept
                {
                    return at(TokenKind::Identifier) && peek()->text() == Keyword;
                }

                const Token* eat(TokenKind Kind) noexcept
                {
                    if (!at(Kind))
                    {
                        return nullptr;
                    }
                    return &Tokens[Position++];
                }

                const Token* eatKeyword(std::string_view Keyword) noexcept
                {
                    if (!atKeyword(Keyword))
                    {
                        return nullptr;
                    }
                    return &Tokens[Position++];
                }

                ParseResult<const Token*> expect(TokenKind Kind, std::string_view Message)
                {
                    const std::size_t CurrentOffset = offset();
                    if (const Token* Result = eat(Kind))
                    {
                        return Result;
                    }
                    return std::unexpected(detail::makeTextDiagnostic(CurrentOffset, Message));
                }

                ParseResult<void> expectEndOfLine(std::string_view Message = "unexpected trailing token")
                {
                    if (peek())
                    {
                        return std::unexpected(detail::makeTextDiagnostic(offset(), Message));
                    }
                    return {};
                }

                ParseResult<std::uint64_t> parseUnsignedInteger(std::string_view Message = "expected an unsigned integer")
                {
                    const std::size_t CurrentOffset = offset();
                    const Token* Number = eat(TokenKind::Number);
                    if (!Number)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(CurrentOffset, Message));
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
                        return std::unexpected(detail::makeTextDiagnostic(Number->Offset, Message));
                    }
                    return Value;
                }

                ParseResult<std::int64_t> parseSignedDisplacement()
                {
                    const std::size_t SignOffset = offset();
                    const bool Negative = eat(TokenKind::Minus) != nullptr;
                    if (!Negative && !eat(TokenKind::Plus))
                    {
                        return std::unexpected(detail::makeTextDiagnostic(SignOffset, "expected a signed displacement"));
                    }
                    const auto Magnitude = parseUnsignedInteger("expected a displacement magnitude");
                    if (!Magnitude)
                    {
                        return std::unexpected(Magnitude.error());
                    }
                    constexpr std::uint64_t SignBit = std::uint64_t{1} << 63;
                    if (*Magnitude > SignBit || (!Negative && *Magnitude == SignBit))
                    {
                        return std::unexpected(detail::makeTextDiagnostic(SignOffset, "signed displacement overflows 64-bit two's complement range"));
                    }
                    if (!Negative)
                    {
                        return static_cast<std::int64_t>(*Magnitude);
                    }
                    return *Magnitude == SignBit ? std::numeric_limits<std::int64_t>::min() : -static_cast<std::int64_t>(*Magnitude);
                }

                ParseResult<std::int64_t> parseOptionalDisplacement()
                {
                    if (!at(TokenKind::Plus) && !at(TokenKind::Minus))
                    {
                        return 0;
                    }
                    return parseSignedDisplacement();
                }

                ParseResult<std::uint64_t> parseImmediateBits()
                {
                    const std::size_t StartOffset = offset();
                    const bool Negative = eat(TokenKind::Minus) != nullptr;
                    if (!Negative)
                    {
                        eat(TokenKind::Plus);
                    }
                    const auto Magnitude = parseUnsignedInteger("expected an immediate integer");
                    if (!Magnitude)
                    {
                        return std::unexpected(Magnitude.error());
                    }
                    constexpr std::uint64_t SignBit = std::uint64_t{1} << 63;
                    if (Negative && *Magnitude > SignBit)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(StartOffset, "negative immediate overflows 64-bit two's complement range"));
                    }
                    return Negative ? std::uint64_t{0} - *Magnitude : *Magnitude;
                }

                ParseResult<core::Name> parseSymbol()
                {
                    const std::size_t StartOffset = offset();
                    if (!eat(TokenKind::At))
                    {
                        return std::unexpected(detail::makeTextDiagnostic(StartOffset, "expected '@' before symbol name"));
                    }
                    const Token* NameToken = eat(TokenKind::Identifier);
                    if (!NameToken)
                    {
                        NameToken = eat(TokenKind::String);
                    }
                    if (!NameToken || NameToken->text().empty())
                    {
                        return std::unexpected(detail::makeTextDiagnostic(StartOffset, "expected a nonempty symbol name"));
                    }
                    return detail::internTextName(NameToken->text(), NameToken->Offset);
                }

                ParseResult<DeclaredLabel> parseDeclaredLabel()
                {
                    const auto LabelToken = expect(TokenKind::Identifier, "expected a declaration label identifier");
                    if (!LabelToken)
                    {
                        return std::unexpected(LabelToken.error());
                    }
                    const Token* NameSource = *LabelToken;
                    if (const Token* AliasToken = eat(TokenKind::String))
                    {
                        NameSource = AliasToken;
                    }
                    const auto OriginalName = detail::internTextName(NameSource->text(), NameSource->Offset);
                    if (!OriginalName)
                    {
                        return std::unexpected(OriginalName.error());
                    }
                    return DeclaredLabel{(*LabelToken)->text(), (*LabelToken)->Offset, *OriginalName};
                }

                ParseResult<MachineSymbolLinkage> parseOptionalLinkage(core::Name SymbolName)
                {
                    MachineSymbolLinkage Linkage = MachineSymbolLinkage::Local;
                    if (const Token* Keyword = eat(TokenKind::Identifier))
                    {
                        const auto Parsed = detail::parseLinkage(Keyword->text());
                        if (!Parsed)
                        {
                            return std::unexpected(detail::makeTextDiagnostic(Keyword->Offset, "expected local or external linkage"));
                        }
                        Linkage = *Parsed;
                    }
                    if (peek() || (Linkage == MachineSymbolLinkage::External && !SymbolName.valid()))
                    {
                        return std::unexpected(detail::makeTextDiagnostic(offset(), "external definitions require a nonempty name and linkage must end the declaration"));
                    }
                    return Linkage;
                }

                ParseResult<detail::ParsedOperand> parseOperand()
                {
                    const std::size_t StartOffset = offset();
                    if (eat(TokenKind::LeftBracket))
                    {
                        std::optional<RegisterId> BaseRegister;
                        core::Name BaseSymbol;
                        if (at(TokenKind::At))
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
                            BaseRegister = parseRegister((*BaseToken)->text());
                            if (!BaseRegister || !isAddressRegister(*BaseRegister))
                            {
                                return std::unexpected(detail::makeTextDiagnostic((*BaseToken)->Offset, "unknown or invalid address base register"));
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
                    if (at(TokenKind::At))
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
                    if (at(TokenKind::Identifier))
                    {
                        const Token* RegisterToken = eat(TokenKind::Identifier);
                        const auto Register = parseRegister(RegisterToken->text());
                        if (!Register || !isArithmeticRegister(*Register))
                        {
                            return std::unexpected(detail::makeTextDiagnostic(RegisterToken->Offset, "unknown register operand"));
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
                    const auto Bits = parseImmediateBits();
                    if (!Bits)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(StartOffset, "invalid instruction operand"));
                    }
                    detail::ParsedOperand Result;
                    Result.emplace<ImmediateOperand>(*Bits);
                    return Result;
                }

            private:
                std::vector<Token> Tokens;
                std::size_t EndOffset;
                std::size_t Position = 0;
        };

        class LineStream
        {
            public:
                explicit LineStream(std::string_view Text) noexcept
                    : Text(Text)
                {
                }

                ParseResult<std::optional<LineCursor>> nextLine()
                {
                    while (Offset < Text.size())
                    {
                        const std::size_t LineStart = Offset;
                        const std::size_t Newline = Text.find('\n', LineStart);
                        const std::size_t LineEnd = Newline == std::string_view::npos ? Text.size() : Newline;
                        Offset = Newline == std::string_view::npos ? LineEnd : LineEnd + 1;
                        auto Tokens = tokenizeLine(Text.substr(LineStart, LineEnd - LineStart), LineStart);
                        if (!Tokens)
                        {
                            return std::unexpected(Tokens.error());
                        }
                        if (!Tokens->empty())
                        {
                            return LineCursor(std::move(*Tokens), LineEnd);
                        }
                    }
                    return std::nullopt;
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
                std::size_t Offset = 0;
                std::size_t TargetOffset = 0;
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
                    : Stream(Text)
                {
                }

                ParseResult<std::unique_ptr<MachineModule>> parseModule()
                {
                    const auto HeaderLine = Stream.nextLine();
                    if (!HeaderLine)
                    {
                        return std::unexpected(HeaderLine.error());
                    }
                    if (!*HeaderLine)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Stream.endOffset(), "missing .module directive"));
                    }
                    if (const auto HeaderResult = parseModuleHeader(**HeaderLine); !HeaderResult)
                    {
                        return std::unexpected(HeaderResult.error());
                    }
                    for (;;)
                    {
                        auto NextLine = Stream.nextLine();
                        if (!NextLine)
                        {
                            return std::unexpected(NextLine.error());
                        }
                        if (!*NextLine)
                        {
                            break;
                        }
                        if (const auto LineResult = parseModuleTopLevelLine(**NextLine); !LineResult)
                        {
                            return std::unexpected(LineResult.error());
                        }
                    }
                    return std::move(Module);
                }

            private:
                ParseResult<void> parseModuleHeader(LineCursor Line)
                {
                    const auto First = Line.expect(TokenKind::Identifier, "expected .module followed by a quoted name");
                    if (!First)
                    {
                        return std::unexpected(First.error());
                    }
                    const Token* NameToken = Line.eat(TokenKind::String);
                    if ((*First)->text() != detail::ModuleDirective || !NameToken || Line.peek())
                    {
                        return std::unexpected(detail::makeTextDiagnostic((*First)->Offset, "expected .module followed by a quoted name"));
                    }
                    const auto Interned = detail::internTextName(NameToken->text(), NameToken->Offset);
                    if (!Interned)
                    {
                        return std::unexpected(Interned.error());
                    }
                    Module = std::make_unique<MachineModule>(*Interned);
                    return {};
                }

                ParseResult<void> parseModuleTopLevelLine(LineCursor& Line)
                {
                    const auto First = Line.expect(TokenKind::Identifier, "expected a top-level directive");
                    if (!First)
                    {
                        return std::unexpected(First.error());
                    }
                    const std::string_view Directive = (*First)->text();
                    if (Directive == detail::DeclareDirective)
                    {
                        return parseSymbolDeclaration(**First, Line);
                    }
                    if (Directive == detail::SectionDirective)
                    {
                        return parseSectionDirective(**First, Line);
                    }
                    if (Directive == detail::GlobalDirective)
                    {
                        return parseGlobalDefinition(**First, Line);
                    }
                    if (Directive == detail::FunctionDirective)
                    {
                        return parseFunctionDefinition(**First, Line);
                    }
                    return std::unexpected(detail::makeTextDiagnostic((*First)->Offset, "unexpected or misplaced top-level directive"));
                }

                ParseResult<void> parseSymbolDeclaration(const Token& Directive, LineCursor& Line)
                {
                    const Token* KindToken = Line.eat(TokenKind::Identifier);
                    if (!KindToken)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.Offset, "expected a symbol kind and nonempty symbol name"));
                    }
                    const auto Symbol = Line.parseSymbol();
                    if (!Symbol)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.Offset, "expected a symbol kind and nonempty symbol name"));
                    }
                    const auto Kind = detail::parseSymbolKind(KindToken->text());
                    if (!Kind)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(KindToken->Offset, "unknown symbol declaration kind"));
                    }
                    bool IsConstant = false;
                    if (const Token* Attribute = Line.eat(TokenKind::Identifier))
                    {
                        if (*Kind != MachineSymbolKind::GlobalVariable || Attribute->text() != detail::ReadOnlyKeyword)
                        {
                            return std::unexpected(detail::makeTextDiagnostic(Attribute->Offset, "only global declarations accept the readonly attribute"));
                        }
                        IsConstant = true;
                    }
                    if (const auto EndResult = Line.expectEndOfLine("unexpected token after the symbol declaration"); !EndResult)
                    {
                        return EndResult;
                    }
                    Module->declareSymbol(*Symbol, *Kind, IsConstant);
                    return {};
                }

                ParseResult<void> parseSectionDirective(const Token& Directive, LineCursor& Line)
                {
                    const Token* SectionToken = Line.eat(TokenKind::Identifier);
                    if (!SectionToken || Line.peek())
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.Offset, "section directive requires one section name outside a function"));
                    }
                    const auto Kind = detail::parseSectionKind(SectionToken->text());
                    if (!Kind)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(SectionToken->Offset, "unknown section"));
                    }
                    CurrentSection = *Kind;
                    return {};
                }

                ParseResult<void> registerModuleLabel(const DeclaredLabel& Declared)
                {
                    if (!ModuleLabels.insert(Declared.Label).second)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Declared.Offset, "duplicate declaration label in the module"));
                    }
                    return {};
                }

                ParseResult<void> parseGlobalDefinition(const Token& Directive, LineCursor& HeaderLine)
                {
                    if (CurrentSection != detail::SectionKind::Data)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.Offset, "global definition must appear in the .data section"));
                    }
                    const auto Declared = HeaderLine.parseDeclaredLabel();
                    if (!Declared)
                    {
                        return std::unexpected(Declared.error());
                    }
                    if (!HeaderLine.eat(TokenKind::Comma))
                    {
                        return std::unexpected(detail::makeTextDiagnostic(HeaderLine.offset(), "expected a comma before global size"));
                    }
                    const auto ByteSize = HeaderLine.parseUnsignedInteger("expected global byte size");
                    if (!ByteSize || !HeaderLine.eat(TokenKind::Comma))
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.Offset, "expected global size and alignment"));
                    }
                    const auto Alignment = HeaderLine.parseUnsignedInteger("expected global alignment");
                    if (!Alignment)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.Offset, "expected global size and alignment"));
                    }
                    if (*Alignment == 0 || (*Alignment & (*Alignment - 1)) != 0)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.Offset, "data alignment must be a nonzero power of two"));
                    }
                    const bool IsConstant = HeaderLine.eatKeyword(detail::ReadOnlyKeyword) != nullptr;
                    const auto Linkage = HeaderLine.parseOptionalLinkage(Declared->OriginalName);
                    if (!Linkage)
                    {
                        return std::unexpected(Linkage.error());
                    }
                    if (const auto LabelResult = registerModuleLabel(*Declared); !LabelResult)
                    {
                        return LabelResult;
                    }
                    std::vector<std::uint8_t> InitialBytes;
                    for (;;)
                    {
                        auto BodyLine = Stream.nextLine();
                        if (!BodyLine)
                        {
                            return std::unexpected(BodyLine.error());
                        }
                        if (!*BodyLine)
                        {
                            return std::unexpected(detail::makeTextDiagnostic(Stream.endOffset(), "unterminated global variable definition"));
                        }
                        const auto BodyDirective = (*BodyLine)->expect(TokenKind::Identifier, "expected .byte or .endglobal");
                        if (!BodyDirective)
                        {
                            return std::unexpected(BodyDirective.error());
                        }
                        if ((*BodyDirective)->text() == detail::ByteDirective)
                        {
                            if (const auto ByteResult = parseByteDirective(**BodyLine, *ByteSize, InitialBytes); !ByteResult)
                            {
                                return ByteResult;
                            }
                            continue;
                        }
                        if ((*BodyDirective)->text() != detail::EndGlobalDirective || (*BodyLine)->peek())
                        {
                            return std::unexpected(detail::makeTextDiagnostic((*BodyDirective)->Offset, "expected .byte or .endglobal"));
                        }
                        if (!Module->createGlobalVariable(*ByteSize, *Alignment, InitialBytes, Declared->OriginalName, *Linkage, IsConstant))
                        {
                            return std::unexpected(detail::makeTextDiagnostic((*BodyDirective)->Offset, "invalid global data layout"));
                        }
                        return {};
                    }
                }

                ParseResult<void> parseByteDirective(LineCursor& Line, std::uint64_t DeclaredSize, std::vector<std::uint8_t>& InitialBytes)
                {
                    do
                    {
                        const std::size_t ValueOffset = Line.offset();
                        const auto ByteValue = Line.parseUnsignedInteger("expected a byte value in the range 0 through 255");
                        if (!ByteValue || *ByteValue > 255)
                        {
                            return std::unexpected(detail::makeTextDiagnostic(ValueOffset, "expected a byte value in the range 0 through 255"));
                        }
                        if (InitialBytes.size() >= DeclaredSize)
                        {
                            return std::unexpected(detail::makeTextDiagnostic(ValueOffset, "global initializer exceeds its declared byte size"));
                        }
                        InitialBytes.push_back(static_cast<std::uint8_t>(*ByteValue));
                    } while (Line.eat(TokenKind::Comma));
                    return Line.expectEndOfLine("unexpected token after byte value");
                }

                ParseResult<void> parseFunctionDefinition(const Token& Directive, LineCursor& HeaderLine)
                {
                    if (CurrentSection != detail::SectionKind::Text)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.Offset, "function definition must appear in the .text section"));
                    }
                    const auto Declared = HeaderLine.parseDeclaredLabel();
                    if (!Declared)
                    {
                        return std::unexpected(Declared.error());
                    }
                    const auto Linkage = HeaderLine.parseOptionalLinkage(Declared->OriginalName);
                    if (!Linkage)
                    {
                        return std::unexpected(Linkage.error());
                    }
                    if (const auto LabelResult = registerModuleLabel(*Declared); !LabelResult)
                    {
                        return LabelResult;
                    }
                    MachineFunction& Function = Module->createFunction(Declared->OriginalName, *Linkage);
                    FunctionParseState State{Function};
                    for (;;)
                    {
                        auto BodyLine = Stream.nextLine();
                        if (!BodyLine)
                        {
                            return std::unexpected(BodyLine.error());
                        }
                        if (!*BodyLine)
                        {
                            return std::unexpected(detail::makeTextDiagnostic(Stream.endOffset(), "unterminated function definition"));
                        }
                        const auto First = (*BodyLine)->expect(TokenKind::Identifier, "expected a directive, label, or instruction");
                        if (!First)
                        {
                            return std::unexpected(First.error());
                        }
                        if ((*BodyLine)->eat(TokenKind::Colon))
                        {
                            if (const auto BlockResult = parseBasicBlock(State, **First, **BodyLine); !BlockResult)
                            {
                                return BlockResult;
                            }
                            continue;
                        }
                        const std::string_view Keyword = (*First)->text();
                        if (Keyword == detail::StackSlotDirective)
                        {
                            if (const auto SlotResult = parseStackSlot(State, **First, **BodyLine); !SlotResult)
                            {
                                return SlotResult;
                            }
                            continue;
                        }
                        if (Keyword == detail::FrameDirective)
                        {
                            if (const auto FrameResult = parseFrameLayout(State, **First, **BodyLine); !FrameResult)
                            {
                                return FrameResult;
                            }
                            continue;
                        }
                        if (Keyword == detail::EndFunctionDirective)
                        {
                            if ((*BodyLine)->peek())
                            {
                                return std::unexpected(detail::makeTextDiagnostic((*First)->Offset, "unexpected token after .endfunction"));
                            }
                            return emitFunctionInstructions(State);
                        }
                        if (Keyword.starts_with('.'))
                        {
                            return std::unexpected(detail::makeTextDiagnostic((*First)->Offset, "unknown or misplaced directive inside function"));
                        }
                        if (const auto InstructionResult = parseInstruction(State, **First, **BodyLine); !InstructionResult)
                        {
                            return InstructionResult;
                        }
                    }
                }

                ParseResult<void> parseStackSlot(FunctionParseState& State, const Token& Directive, LineCursor& Line)
                {
                    if (State.HasBlock || State.HasFrame)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.Offset, "stack slots and one final frame layout must precede all basic blocks"));
                    }
                    const auto SlotSize = Line.parseUnsignedInteger("expected stack slot byte size");
                    if (!SlotSize || !Line.eat(TokenKind::Comma))
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.Offset, "invalid stack slot size or alignment"));
                    }
                    const auto Alignment = Line.parseUnsignedInteger("expected stack slot alignment");
                    if (!Alignment || Line.peek() || !State.Function.frameInfo().createStackSlot(*SlotSize, *Alignment))
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.Offset, "invalid stack slot size or alignment"));
                    }
                    return {};
                }

                ParseResult<void> parseFrameLayout(FunctionParseState& State, const Token& Directive, LineCursor& Line)
                {
                    if (State.HasBlock || State.HasFrame)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.Offset, "stack slots and one final frame layout must precede all basic blocks"));
                    }
                    const auto BaseOffset = Line.parseUnsignedInteger("expected frame layout base offset");
                    if (!BaseOffset || Line.peek() || !State.Function.frameInfo().layout(*BaseOffset))
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Directive.Offset, "invalid or overflowing frame layout"));
                    }
                    State.HasFrame = true;
                    return {};
                }

                ParseResult<void> parseBasicBlock(FunctionParseState& State, const Token& LabelToken, LineCursor& Line)
                {
                    const Token* NameSource = &LabelToken;
                    if (const Token* AliasToken = Line.eat(TokenKind::String))
                    {
                        NameSource = AliasToken;
                    }
                    if (Line.peek())
                    {
                        return std::unexpected(detail::makeTextDiagnostic(LabelToken.Offset, "a block label accepts at most one quoted original name"));
                    }
                    if (State.BlockLabels.contains(LabelToken.text()))
                    {
                        return std::unexpected(detail::makeTextDiagnostic(LabelToken.Offset, "duplicate block label in the function"));
                    }
                    const auto BlockName = detail::internTextName(NameSource->text(), NameSource->Offset);
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

                ParseResult<void> parseInstruction(FunctionParseState& State, const Token& Mnemonic, LineCursor& Line)
                {
                    if (!State.CurrentBlock)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Mnemonic.Offset, "an instruction requires a basic block"));
                    }
                    const auto Signature = detail::lookupMnemonic(Mnemonic.text());
                    if (!Signature)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Mnemonic.Offset, "unknown instruction mnemonic"));
                    }
                    PendingInstruction Pending{State.CurrentBlock, nullptr, {}, {}, Mnemonic.Offset, 0, 0};
                    std::array<OperandKind, 3> Kinds{};
                    std::size_t OperandCount = 0;
                    if (Signature->HasBlockTarget)
                    {
                        const std::size_t TargetIndex = Signature->OperandCount - 1;
                        for (std::size_t Index = 0; Index < TargetIndex; ++Index)
                        {
                            const std::size_t OperandOffset = Line.offset();
                            auto OperandValue = Line.parseOperand();
                            if (!OperandValue || !Line.eat(TokenKind::Comma))
                            {
                                return std::unexpected(detail::makeTextDiagnostic(OperandOffset, "expected a branch comparison operand followed by a comma"));
                            }
                            Kinds[Index] = *detail::parsedOperandKind(*OperandValue);
                            Pending.Operands[Index] = std::move(*OperandValue);
                        }
                        const auto TargetToken = Line.expect(TokenKind::Identifier, "a branch requires a final basic block label");
                        if (!TargetToken || Line.peek())
                        {
                            return std::unexpected(detail::makeTextDiagnostic(Line.offset(), "a branch requires a final basic block label"));
                        }
                        Pending.TargetLabel = (*TargetToken)->text();
                        Pending.TargetOffset = (*TargetToken)->Offset;
                        Pending.TargetOperandIndex = TargetIndex;
                        Kinds[TargetIndex] = OperandKind::BasicBlock;
                        OperandCount = Signature->OperandCount;
                    }
                    else
                    {
                        if (Line.peek())
                        {
                            do
                            {
                                const std::size_t OperandOffset = Line.offset();
                                if (OperandCount == Pending.Operands.size())
                                {
                                    return std::unexpected(detail::makeTextDiagnostic(OperandOffset, "too many instruction operands"));
                                }
                                auto OperandValue = Line.parseOperand();
                                if (!OperandValue)
                                {
                                    return std::unexpected(OperandValue.error());
                                }
                                Kinds[OperandCount] = *detail::parsedOperandKind(*OperandValue);
                                Pending.Operands[OperandCount++] = std::move(*OperandValue);
                            } while (Line.eat(TokenKind::Comma));
                        }
                        if (const auto EndResult = Line.expectEndOfLine("expected a comma between instruction operands"); !EndResult)
                        {
                            return EndResult;
                        }
                    }
                    const detail::OpcodeDescriptor* Descriptor = detail::findOpcodeDescriptor(Mnemonic.text(), std::span<const OperandKind>(Kinds.data(), OperandCount));
                    if (!Descriptor)
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Mnemonic.Offset, "instruction operands do not match the opcode form"));
                    }
                    if (const auto ValidationError = detail::validateParsedInstruction(*Descriptor, Pending.Operands))
                    {
                        return std::unexpected(detail::makeTextDiagnostic(Mnemonic.Offset, *ValidationError));
                    }
                    Pending.Descriptor = Descriptor;
                    State.PendingInstructions.push_back(std::move(Pending));
                    return {};
                }

                ParseResult<void> emitFunctionInstructions(FunctionParseState& State)
                {
                    for (PendingInstruction& Pending : State.PendingInstructions)
                    {
                        if (!Pending.TargetLabel.empty())
                        {
                            const auto Target = State.BlockLabels.find(Pending.TargetLabel);
                            if (Target == State.BlockLabels.end())
                            {
                                return std::unexpected(detail::makeTextDiagnostic(Pending.TargetOffset, "unknown basic block label in this function"));
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
                            return std::unexpected(detail::makeTextDiagnostic(Pending.Offset, "instruction operands cannot be inserted into their function"));
                        }
                    }
                    return {};
                }

                LineStream Stream;
                std::unique_ptr<MachineModule> Module;
                std::unordered_set<std::string_view> ModuleLabels;
                detail::SectionKind CurrentSection = detail::SectionKind::None;
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
