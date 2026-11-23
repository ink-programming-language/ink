#include "ink/backend/vm/serialization/machine_module_text.h"

#include "machine_module_text_builders.h"
#include "machine_module_text_format.h"
#include "machine_module_text_lexer.h"
#include "machine_module_text_literals.h"

#include "ink/backend/vm/instruction/machine_operand.h"
#include "ink/backend/vm/instruction/opcode_info.h"
#include "ink/backend/vm/instruction/register.h"
#include "ink/backend/vm/module/machine_module.h"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <utility>

// Propagates a failed LLParser-style sub-parse (returns true; diagnostic already recorded).
#define RETURN_IF_FAILED(Expression) \
    do                               \
    {                                \
        if (Expression)              \
        {                            \
            return true;             \
        }                            \
    } while (false)

// Records a parser diagnostic at Span and returns failure (true) when Condition holds.
#define ERROR_IF(Condition, Span, Description) \
    do                                         \
    {                                          \
        if (Condition)                         \
        {                                      \
            return error(Span, Description);   \
        }                                      \
    } while (false)

namespace ink::vm
{
    namespace
    {
        using detail::AsmLexer;
        using detail::FunctionBuilder;
        using detail::FunctionHeader;
        using detail::GlobalHeader;
        using detail::GlobalVariableBuilder;
        using detail::ParsedInstructionOperands;
        using detail::ParseResult;
        using detail::ParseStatus;
        using detail::Spanned;
        using detail::TextSpan;
        using detail::Token;
        using detail::TokenKind;

        const OpcodeInfo* findMatchingOverload(std::span<const OpcodeInfo> Overloads, std::span<const OperandKind> Kinds) noexcept
        {
            for (const OpcodeInfo& Candidate : Overloads)
            {
                if (Candidate.matchesOperandKinds(Kinds))
                {
                    return &Candidate;
                }
            }
            return nullptr;
        }

        // Grammar-driven recursive-descent parser modeled after LLVM's LLParser:
        // internal parse* routines return true on error and short-circuit with '||',
        // while diagnostics flow through AsmLexer's prioritized error slot (Lexer > Parser).
        class ModuleTextParser final
        {
            public:
                explicit ModuleTextParser(std::string_view Text) noexcept
                    : Lexer(Text)
                {
                }

                std::expected<std::unique_ptr<MachineModule>, core::Diagnostic> parse()
                {
                    std::unique_ptr<MachineModule> Module;
                    if (parseModule(Module))
                    {
                        return std::unexpected(Lexer.takeDiagnostic());
                    }
                    return Module;
                }

            private:
                struct OptionalSign
                {
                        bool Present = false;
                        bool Negative = false;
                        TextSpan Span{};
                };

                bool error(TextSpan Span, std::string_view Description)
                {
                    return Lexer.parseError(Span, Description);
                }

                bool adopt(ParseStatus Status) noexcept
                {
                    return !Status && Lexer.recordParserDiagnostic(std::move(Status.error()));
                }

                template <typename ValueType>
                bool adopt(ParseResult<ValueType>&& Result, ValueType& Out) noexcept
                {
                    if (!Result)
                    {
                        return Lexer.recordParserDiagnostic(std::move(Result.error()));
                    }
                    Out = std::move(*Result);
                    return false;
                }

                // --- Module, Declarations, and Sections ---

                bool parseModule(std::unique_ptr<MachineModule>& OutModule)
                {
                    Lexer.skipBlankLines();
                    ERROR_IF(Lexer.kind() == TokenKind::EndOfFile, Lexer.endSpan(), "missing .module directive");
                    return parseModuleHeader(OutModule) || parseSymbolDeclarations(*OutModule) || parseSections(*OutModule);
                }

                bool parseModuleHeader(std::unique_ptr<MachineModule>& OutModule)
                {
                    Token First;
                    Token NameToken;
                    core::Name ModuleName;
                    RETURN_IF_FAILED(Lexer.expect(TokenKind::Identifier, "expected .module followed by a quoted name", First));
                    ERROR_IF(First.text() != detail::ModuleDirective, First.Span, "expected .module followed by a quoted name");
                    RETURN_IF_FAILED(Lexer.expect(TokenKind::String, "expected a quoted module name", NameToken) || Lexer.expectEndOfLine("unexpected token after .module header") || adopt(detail::internName(NameToken.text(), NameToken.Span), ModuleName));
                    OutModule = std::make_unique<MachineModule>(ModuleName);
                    return false;
                }

                bool parseSymbolDeclarations(MachineModule& Module)
                {
                    Lexer.skipBlankLines();
                    while (Lexer.atWord(detail::DeclareDirective))
                    {
                        RETURN_IF_FAILED(parseSymbolDeclaration(Module));
                        Lexer.skipBlankLines();
                    }
                    return false;
                }

                bool parseSymbolDeclaration(MachineModule& Module)
                {
                    Lexer.consume();
                    Token KindToken;
                    RETURN_IF_FAILED(Lexer.expect(TokenKind::Identifier, "expected 'function' or 'global' after .declare", KindToken));
                    const auto Kind = detail::parseSymbolKind(KindToken.text());
                    ERROR_IF(!Kind, KindToken.Span, "unknown symbol declaration kind");
                    core::Name Symbol;
                    RETURN_IF_FAILED(parseSymbol(Symbol));
                    bool IsConstant = false;
                    if (Token Attribute; Lexer.eatToken(TokenKind::Identifier, Attribute))
                    {
                        ERROR_IF(Attribute.text() != detail::ReadOnlyKeyword, Attribute.Span, "unknown symbol declaration attribute; expected 'readonly'");
                        ERROR_IF(*Kind != MachineSymbolKind::GlobalVariable, Attribute.Span, "only global declarations accept the readonly attribute");
                        IsConstant = true;
                    }
                    RETURN_IF_FAILED(Lexer.expectEndOfLine("unexpected token after the symbol declaration"));
                    Module.declareSymbol(Symbol, *Kind, IsConstant);
                    return false;
                }

                bool parseSections(MachineModule& Module)
                {
                    Lexer.skipBlankLines();
                    while (Lexer.atWord(detail::SectionDirective))
                    {
                        RETURN_IF_FAILED(parseSection(Module));
                        Lexer.skipBlankLines();
                    }
                    return Lexer.kind() != TokenKind::EndOfFile && diagnoseUnexpectedTopLevelDirective();
                }

                bool parseSection(MachineModule& Module)
                {
                    Lexer.consume();
                    Token SectionToken;
                    RETURN_IF_FAILED(Lexer.expect(TokenKind::Identifier, "section directive requires .data or .text", SectionToken));
                    const auto Kind = detail::parseSectionKind(SectionToken.text());
                    ERROR_IF(!Kind, SectionToken.Span, "unknown section");
                    RETURN_IF_FAILED(Lexer.expectEndOfLine("unexpected token after .section directive"));
                    Lexer.skipBlankLines();
                    if (*Kind == detail::SectionKind::Data)
                    {
                        while (Lexer.atWord(detail::GlobalDirective))
                        {
                            RETURN_IF_FAILED(parseGlobalDefinition(Module));
                            Lexer.skipBlankLines();
                        }
                        return false;
                    }
                    while (Lexer.atWord(detail::FunctionDirective))
                    {
                        RETURN_IF_FAILED(parseFunctionDefinition(Module));
                        Lexer.skipBlankLines();
                    }
                    return false;
                }

                bool diagnoseUnexpectedTopLevelDirective()
                {
                    Token First;
                    RETURN_IF_FAILED(Lexer.expect(TokenKind::Identifier, "expected a top-level directive", First));
                    const std::string_view Directive = First.text();
                    ERROR_IF(Directive == detail::DeclareDirective, First.Span, ".declare directives must precede all .section directives");
                    ERROR_IF(Directive == detail::GlobalDirective, First.Span, "global definition must appear in the .data section");
                    ERROR_IF(Directive == detail::FunctionDirective, First.Span, "function definition must appear in the .text section");
                    return error(First.Span, "unexpected or misplaced top-level directive");
                }

                // --- Global Definitions (.section .data) ---

                bool parseGlobalDefinition(MachineModule& Module)
                {
                    const Token Directive = Lexer.take();
                    GlobalHeader Header;
                    RETURN_IF_FAILED(parseGlobalHeader(Directive, Header));
                    GlobalVariableBuilder Builder(Header);
                    Lexer.skipBlankLines();
                    while (Lexer.kind() != TokenKind::EndOfFile && !Lexer.atWord(detail::EndGlobalDirective))
                    {
                        RETURN_IF_FAILED(parseGlobalBodyDirective(Builder));
                        Lexer.skipBlankLines();
                    }
                    ERROR_IF(Lexer.kind() == TokenKind::EndOfFile, Lexer.endSpan(), "unterminated global variable definition");
                    const Token EndDirective = Lexer.take();
                    return Lexer.expectEndOfLine("unexpected token after .endglobal") || adopt(Builder.finish(Module, EndDirective.Span));
                }

                bool parseGlobalHeader(const Token& Directive, GlobalHeader& OutHeader)
                {
                    core::Name GlobalName = core::NAME_None;
                    RETURN_IF_FAILED(parseOptionalSymbol(GlobalName));
                    std::optional<std::uint64_t> ByteSize;
                    std::optional<std::uint64_t> Alignment;
                    std::optional<MachineSymbolLinkage> Linkage;
                    bool IsConstant = false;
                    while (!Lexer.atLineEnd())
                    {
                        Token Attr;
                        RETURN_IF_FAILED(Lexer.expect(TokenKind::Identifier, "expected global attribute 'size', 'align', 'readonly', 'local', or 'external'", Attr));
                        const std::string_view Keyword = Attr.text();
                        if (Keyword == detail::SizeKeyword)
                        {
                            RETURN_IF_FAILED(parseSizeAttribute(ByteSize, Attr.Span, "duplicate 'size' attribute on global definition", "expected global byte size after 'size'"));
                        }
                        else if (Keyword == detail::AlignKeyword)
                        {
                            RETURN_IF_FAILED(parseAlignmentAttribute(Alignment, Attr.Span, "duplicate 'align' attribute on global definition", "expected global alignment after 'align'", "data alignment must be a nonzero power of two"));
                        }
                        else if (Keyword == detail::ReadOnlyKeyword)
                        {
                            ERROR_IF(IsConstant, Attr.Span, "duplicate 'readonly' attribute on global definition");
                            IsConstant = true;
                        }
                        else if (const auto ParsedLinkage = detail::parseLinkage(Keyword))
                        {
                            ERROR_IF(Linkage, Attr.Span, "duplicate linkage attribute on global definition");
                            ERROR_IF(*ParsedLinkage == MachineSymbolLinkage::External && !GlobalName.valid(), Attr.Span, "external definitions require a nonempty symbol name");
                            Linkage = *ParsedLinkage;
                        }
                        else
                        {
                            return error(Attr.Span, "unknown global attribute; expected 'size', 'align', 'readonly', 'local', or 'external'");
                        }
                    }
                    RETURN_IF_FAILED(Lexer.expectEndOfLine());
                    ERROR_IF(!ByteSize, Directive.Span, "global definition requires a 'size' attribute");
                    OutHeader = GlobalHeader{GlobalName, *ByteSize, Alignment.value_or(1), Linkage.value_or(MachineSymbolLinkage::Local), IsConstant};
                    return false;
                }

                bool parseGlobalBodyDirective(GlobalVariableBuilder& Builder)
                {
                    Token Directive;
                    RETURN_IF_FAILED(Lexer.expect(TokenKind::Identifier, "expected a global data directive or .endglobal", Directive));
                    const std::string_view Keyword = Directive.text();
                    if (Keyword == detail::ByteDirective)
                    {
                        return parseByteDirective(Builder);
                    }
                    if (Keyword == detail::BytesDirective)
                    {
                        return parseBytesDirective(Builder);
                    }
                    if (Keyword == detail::ZeroDirective)
                    {
                        return parseZeroDirective(Builder);
                    }
                    if (Keyword == detail::I16Directive)
                    {
                        return parseIntDataDirective(16, Builder);
                    }
                    if (Keyword == detail::I32Directive)
                    {
                        return parseIntDataDirective(32, Builder);
                    }
                    if (Keyword == detail::I64Directive)
                    {
                        return parseIntDataDirective(64, Builder);
                    }
                    if (Keyword == detail::F64Directive)
                    {
                        return parseFloatDataDirective(Builder);
                    }
                    if (Keyword == detail::RelocDirective)
                    {
                        return parseRelocDirective(Builder);
                    }
                    return error(Directive.Span, "expected a global data directive or .endglobal");
                }

                bool parseByteDirective(GlobalVariableBuilder& Builder)
                {
                    return parseCommaSeparated([&]() -> bool
                                               {
                                                   Spanned<std::uint64_t> Value;
                                                   RETURN_IF_FAILED(parseUnsignedInteger(Value, "expected a byte value in the range 0 through 255"));
                                                   ERROR_IF(Value.Value > 255, Value.Span, "expected a byte value in the range 0 through 255");
                                                   return adopt(Builder.appendByte(static_cast<std::uint8_t>(Value.Value), Value.Span));
                                               },
                                               "unexpected token after byte value");
                }

                bool parseBytesDirective(GlobalVariableBuilder& Builder)
                {
                    Token StringToken;
                    RETURN_IF_FAILED(Lexer.expect(TokenKind::String, "expected a quoted byte string after .bytes", StringToken) || Lexer.expectEndOfLine("unexpected token after .bytes literal"));
                    return adopt(Builder.appendBytes(StringToken.text(), StringToken.Span));
                }

                bool parseZeroDirective(GlobalVariableBuilder& Builder)
                {
                    Spanned<std::uint64_t> Count;
                    RETURN_IF_FAILED(parseUnsignedInteger(Count, "expected an unsigned zero-byte count after .zero") || Lexer.expectEndOfLine("unexpected token after .zero count"));
                    return adopt(Builder.appendZeros(Count.Value, Count.Span));
                }

                bool parseIntDataDirective(std::uint32_t BitWidth, GlobalVariableBuilder& Builder)
                {
                    const std::size_t ByteWidth = BitWidth / 8;
                    return parseCommaSeparated([&]() -> bool
                                               {
                                                   Spanned<std::uint64_t> Value;
                                                   return parseSignedOrUnsignedInteger(BitWidth, Value, "expected an integer value") || adopt(Builder.appendLittleEndian(Value.Value, ByteWidth, Value.Span));
                                               },
                                               "unexpected token after integer value");
                }

                bool parseFloatDataDirective(GlobalVariableBuilder& Builder)
                {
                    return parseCommaSeparated([&]() -> bool
                                               {
                                                   Spanned<std::uint64_t> Bits;
                                                   return parseFloat64Bits(Bits) || adopt(Builder.appendLittleEndian(Bits.Value, 8, Bits.Span));
                                               },
                                               "unexpected token after floating-point value");
                }

                bool parseRelocDirective(GlobalVariableBuilder& Builder)
                {
                    Spanned<std::uint64_t> Offset;
                    core::Name Symbol;
                    std::int64_t Addend = 0;
                    RETURN_IF_FAILED(parseUnsignedInteger(Offset, "expected a relocation byte offset") || Lexer.expect(TokenKind::Comma, "expected ',' after relocation byte offset") || parseSymbol(Symbol) || parseOptionalDisplacement(Addend) || Lexer.expectEndOfLine("unexpected token after global relocation"));
                    return adopt(Builder.addRelocation(Offset.Value, Symbol, Addend, Offset.Span));
                }

                // --- Function Definitions (.section .text) ---

                bool parseFunctionDefinition(MachineModule& Module)
                {
                    Lexer.consume();
                    FunctionHeader Header;
                    RETURN_IF_FAILED(parseFunctionHeader(Header));
                    MachineFunction& Function = Module.createFunction(Header.Name, Header.Linkage);
                    FunctionBuilder Builder(Function);
                    Lexer.skipBlankLines();
                    while (Lexer.atDirective(detail::StackSlotDirective))
                    {
                        RETURN_IF_FAILED(parseStackSlot(Builder));
                        Lexer.skipBlankLines();
                    }
                    if (Lexer.atDirective(detail::FrameDirective))
                    {
                        RETURN_IF_FAILED(parseFrameLayout(Builder));
                        Lexer.skipBlankLines();
                    }
                    while (Lexer.atLabel())
                    {
                        RETURN_IF_FAILED(parseBasicBlock(Builder));
                        Lexer.skipBlankLines();
                    }
                    return parseEndFunction(Builder);
                }

                bool parseEndFunction(FunctionBuilder& Builder)
                {
                    ERROR_IF(Lexer.kind() == TokenKind::EndOfFile, Lexer.endSpan(), "unterminated function definition");
                    Token First;
                    RETURN_IF_FAILED(Lexer.expect(TokenKind::Identifier, "expected a directive, label, or instruction", First));
                    const std::string_view Keyword = First.text();
                    if (Keyword == detail::EndFunctionDirective)
                    {
                        return Lexer.expectEndOfLine("unexpected token after .endfunction") || adopt(Builder.finish());
                    }
                    ERROR_IF(Keyword == detail::StackSlotDirective || Keyword == detail::FrameDirective, First.Span, "stack slots and one final frame layout must precede all basic blocks");
                    ERROR_IF(Keyword.starts_with('.'), First.Span, "unknown or misplaced directive inside function");
                    return error(First.Span, "an instruction requires a basic block");
                }

                bool parseFunctionHeader(FunctionHeader& OutHeader)
                {
                    core::Name FunctionName = core::NAME_None;
                    RETURN_IF_FAILED(parseOptionalSymbol(FunctionName));
                    MachineSymbolLinkage Linkage = MachineSymbolLinkage::Local;
                    if (Token LinkageToken; Lexer.eatToken(TokenKind::Identifier, LinkageToken))
                    {
                        const auto Parsed = detail::parseLinkage(LinkageToken.text());
                        ERROR_IF(!Parsed, LinkageToken.Span, "expected 'local' or 'external' linkage");
                        ERROR_IF(*Parsed == MachineSymbolLinkage::External && !FunctionName.valid(), LinkageToken.Span, "external definitions require a nonempty symbol name");
                        Linkage = *Parsed;
                    }
                    RETURN_IF_FAILED(Lexer.expectEndOfLine("unexpected token after .function header"));
                    OutHeader = FunctionHeader{FunctionName, Linkage};
                    return false;
                }

                bool parseStackSlot(FunctionBuilder& Builder)
                {
                    const Token Directive = Lexer.take();
                    std::optional<std::uint64_t> SlotSize;
                    std::optional<std::uint64_t> Alignment;
                    while (!Lexer.atLineEnd())
                    {
                        Token Attr;
                        RETURN_IF_FAILED(Lexer.expect(TokenKind::Identifier, "expected stack slot attribute 'size' or 'align'", Attr));
                        if (Attr.text() == detail::SizeKeyword)
                        {
                            RETURN_IF_FAILED(parseSizeAttribute(SlotSize, Attr.Span, "duplicate 'size' attribute on .stackslot", "expected stack slot byte size after 'size'"));
                        }
                        else if (Attr.text() == detail::AlignKeyword)
                        {
                            RETURN_IF_FAILED(parseAlignmentAttribute(Alignment, Attr.Span, "duplicate 'align' attribute on .stackslot", "expected stack slot alignment after 'align'", "stack slot alignment must be a nonzero power of two"));
                        }
                        else
                        {
                            return error(Attr.Span, "unknown .stackslot attribute; expected 'size' or 'align'");
                        }
                    }
                    RETURN_IF_FAILED(Lexer.expectEndOfLine());
                    ERROR_IF(!SlotSize, Directive.Span, ".stackslot requires a 'size' attribute");
                    return adopt(Builder.addStackSlot(*SlotSize, Alignment.value_or(1), Directive.Span));
                }

                bool parseFrameLayout(FunctionBuilder& Builder)
                {
                    Lexer.consume();
                    Token BaseKeywordToken;
                    Spanned<std::uint64_t> BaseOffset;
                    RETURN_IF_FAILED(Lexer.expect(TokenKind::Identifier, "expected 'base' after .frame", BaseKeywordToken));
                    ERROR_IF(BaseKeywordToken.text() != detail::BaseKeyword, BaseKeywordToken.Span, "expected 'base' after .frame");
                    RETURN_IF_FAILED(parseUnsignedInteger(BaseOffset, "expected frame layout base offset after 'base'") || Lexer.expectEndOfLine("unexpected token after .frame directive"));
                    return adopt(Builder.layoutFrame(BaseOffset.Value, BaseOffset.Span));
                }

                bool parseBasicBlock(FunctionBuilder& Builder)
                {
                    const Token LabelToken = Lexer.take();
                    Lexer.consume();
                    Token AliasToken;
                    const bool HasAlias = Lexer.eatToken(TokenKind::String, AliasToken);
                    RETURN_IF_FAILED(Lexer.expectEndOfLine("a block label accepts at most one quoted original name"));
                    const Token& NameSource = HasAlias ? AliasToken : LabelToken;
                    core::Name BlockName;
                    MachineBasicBlock* Block = nullptr;
                    RETURN_IF_FAILED(adopt(detail::internName(NameSource.text(), NameSource.Span), BlockName) || adopt(Builder.createBasicBlock(LabelToken.rawView(), BlockName, LabelToken.Span), Block));
                    Lexer.skipBlankLines();
                    while (Lexer.kind() == TokenKind::Identifier && !Lexer.atLabel() && !Lexer.current().text().starts_with('.'))
                    {
                        RETURN_IF_FAILED(parseInstruction(*Block, Builder));
                        Lexer.skipBlankLines();
                    }
                    return false;
                }

                // --- Instructions and Operands ---

                bool parseInstruction(MachineBasicBlock& Block, FunctionBuilder& Builder)
                {
                    const Token Mnemonic = Lexer.take();
                    const auto Overloads = findOpcodeOverloads(Mnemonic.text());
                    ERROR_IF(Overloads.empty(), Mnemonic.Span, "unknown instruction mnemonic");
                    // Compile-time verified opcode invariants guarantee all overloads of a mnemonic agree on HasBlockTarget and branch arity.
                    ParsedInstructionOperands Parsed;
                    RETURN_IF_FAILED(Overloads.front().HasBlockTarget ? parseBranchOperands(static_cast<std::size_t>(Overloads.front().OperandCount - 1), Parsed) : parseValueOperands(Parsed));
                    const OpcodeInfo* Matched = findMatchingOverload(Overloads, {Parsed.Kinds.data(), Parsed.Count});
                    ERROR_IF(!Matched, Mnemonic.Span, "operand types or count do not match instruction mnemonic");
                    return adopt(Builder.addInstruction(Block, *Matched, std::move(Parsed), Mnemonic.Span));
                }

                bool parseBranchOperands(std::size_t PrefixOperandCount, ParsedInstructionOperands& Out)
                {
                    for (std::size_t Index = 0; Index < PrefixOperandCount; ++Index)
                    {
                        RETURN_IF_FAILED(parseOperand(Out.Operands[Index]) || Lexer.expect(TokenKind::Comma, "expected ',' after branch comparison operand"));
                        Out.Kinds[Index] = Out.Operands[Index].getKind();
                    }
                    Token TargetToken;
                    RETURN_IF_FAILED(Lexer.expect(TokenKind::Identifier, "a branch requires a final basic block label", TargetToken) || Lexer.expectEndOfLine("unexpected token after basic block target"));
                    Out.Operands[PrefixOperandCount] = MachineOperand(BasicBlockOperand(nullptr));
                    Out.Kinds[PrefixOperandCount] = OperandKind::BasicBlock;
                    Out.Count = PrefixOperandCount + 1;
                    Out.BranchTarget = Spanned<std::string_view>{TargetToken.rawView(), TargetToken.Span};
                    return false;
                }

                bool parseValueOperands(ParsedInstructionOperands& Out)
                {
                    if (Lexer.atLineEnd())
                    {
                        return Lexer.expectEndOfLine();
                    }
                    return parseCommaSeparated([&]() -> bool
                                               {
                                                   ERROR_IF(Out.Count == Out.Operands.size(), Lexer.current().Span, "too many instruction operands");
                                                   RETURN_IF_FAILED(parseOperand(Out.Operands[Out.Count]));
                                                   Out.Kinds[Out.Count] = Out.Operands[Out.Count].getKind();
                                                   ++Out.Count;
                                                   return false;
                                               },
                                               "expected ',' between instruction operands");
                }

                bool parseOperand(MachineOperand& Out)
                {
                    switch (Lexer.kind())
                    {
                    case TokenKind::LeftBracket:
                        return parseAddressOperand(Out);
                    case TokenKind::Symbol:
                        return parseSymbolImmediateOperand(Out);
                    case TokenKind::Identifier:
                        return parseRegisterOperand(Out);
                    case TokenKind::Number:
                    case TokenKind::Plus:
                    case TokenKind::Minus:
                        return parseNumericImmediateOperand(Out);
                    default:
                        return error(Lexer.current().Span, "expected an instruction operand");
                    }
                }

                bool parseAddressOperand(MachineOperand& Out)
                {
                    Lexer.consume();
                    core::Name BaseSymbol = core::NAME_None;
                    std::optional<RegisterId> BaseRegister;
                    RETURN_IF_FAILED(parseOptionalSymbol(BaseSymbol));
                    if (!BaseSymbol.valid())
                    {
                        Token BaseToken;
                        RETURN_IF_FAILED(Lexer.expect(TokenKind::Identifier, "expected an address base register or symbol", BaseToken));
                        BaseRegister = parseRegister(BaseToken.text());
                        ERROR_IF(!BaseRegister || !isAddressRegister(*BaseRegister), BaseToken.Span, "unknown or invalid address base register");
                    }
                    std::int64_t Offset = 0;
                    RETURN_IF_FAILED(parseOptionalDisplacement(Offset) || Lexer.expect(TokenKind::RightBracket, "expected ']' after memory address"));
                    Out = BaseRegister ? MachineOperand(AddressOperand(*BaseRegister, Offset)) : MachineOperand(AddressOperand(BaseSymbol, Offset));
                    return false;
                }

                bool parseSymbolImmediateOperand(MachineOperand& Out)
                {
                    core::Name Symbol;
                    std::int64_t Offset = 0;
                    RETURN_IF_FAILED(parseSymbol(Symbol) || parseOptionalDisplacement(Offset));
                    Out = MachineOperand(ImmediateOperand(Symbol, Offset));
                    return false;
                }

                bool parseRegisterOperand(MachineOperand& Out)
                {
                    const Token RegisterToken = Lexer.take();
                    const auto Register = parseRegister(RegisterToken.text());
                    ERROR_IF(!Register || !isArithmeticRegister(*Register), RegisterToken.Span, "unknown register operand");
                    std::int64_t Offset = 0;
                    RETURN_IF_FAILED(parseOptionalDisplacement(Offset));
                    Out = MachineOperand(RegisterOperand(*Register, Offset));
                    return false;
                }

                bool parseNumericImmediateOperand(MachineOperand& Out)
                {
                    Spanned<std::uint64_t> Bits;
                    RETURN_IF_FAILED(parseSignedOrUnsignedInteger(64, Bits, "expected an immediate integer"));
                    Out = MachineOperand(ImmediateOperand(Bits.Value));
                    return false;
                }

                // --- Reusable Token, Attribute, and Literal Primitives ---

                template <typename ParseItem>
                bool parseCommaSeparated(ParseItem&& ParseOne, std::string_view TrailingMessage)
                {
                    do
                    {
                        RETURN_IF_FAILED(ParseOne());
                    } while (Lexer.eat(TokenKind::Comma));
                    return Lexer.expectEndOfLine(TrailingMessage);
                }

                bool parseSizeAttribute(std::optional<std::uint64_t>& OutSize, TextSpan AttrSpan, std::string_view DuplicateMessage, std::string_view ValueMessage)
                {
                    ERROR_IF(OutSize, AttrSpan, DuplicateMessage);
                    Spanned<std::uint64_t> Parsed;
                    RETURN_IF_FAILED(parseUnsignedInteger(Parsed, ValueMessage));
                    OutSize = Parsed.Value;
                    return false;
                }

                bool parseAlignmentAttribute(std::optional<std::uint64_t>& OutAlignment, TextSpan AttrSpan, std::string_view DuplicateMessage, std::string_view ValueMessage, std::string_view PowerOfTwoMessage)
                {
                    ERROR_IF(OutAlignment, AttrSpan, DuplicateMessage);
                    Spanned<std::uint64_t> Parsed;
                    RETURN_IF_FAILED(parseUnsignedInteger(Parsed, ValueMessage));
                    ERROR_IF(!std::has_single_bit(Parsed.Value), Parsed.Span, PowerOfTwoMessage);
                    OutAlignment = Parsed.Value;
                    return false;
                }

                bool parseSymbol(core::Name& Out)
                {
                    Token SymbolToken;
                    return Lexer.expect(TokenKind::Symbol, "expected '@symbol' name", SymbolToken) || adopt(detail::internName(SymbolToken.text(), SymbolToken.Span), Out);
                }

                bool parseOptionalSymbol(core::Name& Out)
                {
                    Token SymbolToken;
                    if (!Lexer.eatToken(TokenKind::Symbol, SymbolToken))
                    {
                        Out = core::NAME_None;
                        return false;
                    }
                    return adopt(detail::internName(SymbolToken.text(), SymbolToken.Span), Out);
                }

                OptionalSign eatOptionalSign()
                {
                    const TokenKind Kind = Lexer.kind();
                    if (Kind != TokenKind::Plus && Kind != TokenKind::Minus)
                    {
                        return OptionalSign{};
                    }
                    const Token SignToken = Lexer.take();
                    return OptionalSign{true, Kind == TokenKind::Minus, SignToken.Span};
                }

                bool parseUnsignedInteger(Spanned<std::uint64_t>& Out, std::string_view Message = "expected an unsigned integer")
                {
                    Token Number;
                    std::uint64_t Value = 0;
                    RETURN_IF_FAILED(Lexer.expect(TokenKind::Number, Message, Number) || adopt(detail::decodeUnsignedInteger(Number.text(), Number.Span, Message), Value));
                    Out = Spanned<std::uint64_t>{Value, Number.Span};
                    return false;
                }

                bool parseOptionalDisplacement(std::int64_t& Out)
                {
                    const OptionalSign Sign = eatOptionalSign();
                    if (!Sign.Present)
                    {
                        Out = 0;
                        return false;
                    }
                    ERROR_IF(Lexer.atLineEnd(), Sign.Span, "expected a displacement magnitude");
                    Spanned<std::uint64_t> Magnitude;
                    return parseUnsignedInteger(Magnitude, "expected a displacement magnitude") || adopt(detail::applyDisplacementSign(Sign.Negative, Magnitude.Value, Sign.Span.mergedWith(Magnitude.Span)), Out);
                }

                bool parseSignedOrUnsignedInteger(std::uint32_t BitWidth, Spanned<std::uint64_t>& Out, std::string_view ExpectedMessage)
                {
                    const OptionalSign Sign = eatOptionalSign();
                    ERROR_IF(Sign.Present && Lexer.atLineEnd(), Sign.Span, ExpectedMessage);
                    Spanned<std::uint64_t> Magnitude;
                    RETURN_IF_FAILED(parseUnsignedInteger(Magnitude, ExpectedMessage));
                    const TextSpan FullSpan = Sign.Present ? Sign.Span.mergedWith(Magnitude.Span) : Magnitude.Span;
                    std::uint64_t Bits = 0;
                    RETURN_IF_FAILED(adopt(detail::applyIntegerWidthAndSign(BitWidth, Sign.Negative, Magnitude.Value, FullSpan), Bits));
                    Out = Spanned<std::uint64_t>{Bits, FullSpan};
                    return false;
                }

                bool parseFloat64Bits(Spanned<std::uint64_t>& Out)
                {
                    const OptionalSign Sign = eatOptionalSign();
                    ERROR_IF(Lexer.atLineEnd(), Sign.Present ? Sign.Span : Lexer.current().Span, "expected an IEEE 754 binary64 floating-point value");
                    const Token ValueToken = Lexer.take();
                    const TextSpan FullSpan = Sign.Present ? Sign.Span.mergedWith(ValueToken.Span) : ValueToken.Span;
                    std::uint64_t Bits = 0;
                    RETURN_IF_FAILED(adopt(detail::decodeFloat64Bits(Sign.Negative, ValueToken, FullSpan), Bits));
                    Out = Spanned<std::uint64_t>{Bits, FullSpan};
                    return false;
                }

                AsmLexer Lexer;
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

#undef ERROR_IF
#undef RETURN_IF_FAILED
