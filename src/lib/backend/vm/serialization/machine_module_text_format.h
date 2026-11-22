#pragma once

#include "ink/backend/vm/instruction/machine_instruction_validator.h"
#include "ink/backend/vm/instruction/machine_instructions.h"
#include "ink/backend/vm/module/machine_instruction_builder.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic_builder.h"
#include "ink/core/name/name_pool.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

// Formal Ink VM Assembly Text Grammar (see docs/VM-Text-Format.md):
//
//   Module              := ModuleHeader SymbolDeclaration* Section*
//   ModuleHeader        := '.module' QuotedString
//   SymbolDeclaration   := '.declare' ('function' | 'global') Symbol ['readonly']
//   Section             := '.section' ('.data' GlobalDefinition* | '.text' FunctionDefinition*)
//   GlobalDefinition    := '.global' [Symbol] GlobalAttr+
//                          GlobalBodyDirective*
//                          '.endglobal'
//   GlobalAttr          := 'size' Unsigned | 'align' Unsigned | 'readonly' | 'local' | 'external'
//   GlobalBodyDirective := ByteDirective
//                        | BytesDirective
//                        | ZeroDirective
//                        | IntDataDirective
//                        | FloatDataDirective
//                        | RelocDirective
//   ByteDirective       := '.byte' Unsigned (',' Unsigned)*
//   BytesDirective      := '.bytes' QuotedString
//   ZeroDirective       := '.zero' Unsigned
//   IntDataDirective    := ('.i16' | '.i32' | '.i64') SignedOrUnsignedInt (',' SignedOrUnsignedInt)*
//   FloatDataDirective  := '.f64' FloatLiteral (',' FloatLiteral)*
//   RelocDirective      := '.reloc' Unsigned ',' Symbol [Displacement]
//   FunctionDefinition  := '.function' [Symbol] [Linkage]
//                          StackSlotDirective*
//                          [FrameDirective]
//                          BasicBlock*
//                          '.endfunction'
//   StackSlotDirective  := '.stackslot' StackSlotAttr+
//   StackSlotAttr       := 'size' Unsigned | 'align' Unsigned
//   FrameDirective      := '.frame' 'base' Unsigned
//   BasicBlock          := Identifier ':' [QuotedString]
//                          Instruction*
//   Instruction         := Mnemonic [Operand (',' Operand)*]
//   Linkage             := 'local' | 'external'
//   Symbol              := '@' (Identifier | QuotedString)
//   Operand             := '[' (Register | Symbol) [Displacement] ']'
//                        | Symbol [Displacement]
//                        | Register [Displacement]
//                        | SignedOrUnsignedInt
//                        | Identifier
//   Displacement        := ('+' | '-') Unsigned

namespace ink::vm::detail
{
    inline constexpr std::string_view ModuleDirective = ".module";
    inline constexpr std::string_view DeclareDirective = ".declare";
    inline constexpr std::string_view SectionDirective = ".section";
    inline constexpr std::string_view DataSectionName = ".data";
    inline constexpr std::string_view TextSectionName = ".text";
    inline constexpr std::string_view GlobalDirective = ".global";
    inline constexpr std::string_view EndGlobalDirective = ".endglobal";
    inline constexpr std::string_view ByteDirective = ".byte";
    inline constexpr std::string_view BytesDirective = ".bytes";
    inline constexpr std::string_view ZeroDirective = ".zero";
    inline constexpr std::string_view I16Directive = ".i16";
    inline constexpr std::string_view I32Directive = ".i32";
    inline constexpr std::string_view I64Directive = ".i64";
    inline constexpr std::string_view F64Directive = ".f64";
    inline constexpr std::string_view RelocDirective = ".reloc";
    inline constexpr std::string_view FunctionDirective = ".function";
    inline constexpr std::string_view EndFunctionDirective = ".endfunction";
    inline constexpr std::string_view StackSlotDirective = ".stackslot";
    inline constexpr std::string_view FrameDirective = ".frame";
    inline constexpr std::string_view SizeKeyword = "size";
    inline constexpr std::string_view AlignKeyword = "align";
    inline constexpr std::string_view BaseKeyword = "base";
    inline constexpr std::string_view ReadOnlyKeyword = "readonly";

    inline constexpr char UpperHexDigits[] = "0123456789ABCDEF";
    inline constexpr char LowerHexDigits[] = "0123456789abcdef";

    // Signed 64-bit immediates within [-DecimalImmediateLimit, DecimalImmediateLimit] format as decimal;
    // wider bit patterns (such as IEEE-754 floats or bitmasks) format as lowercase 0x hexadecimal.
    inline constexpr std::int64_t DecimalImmediateLimit = 1048576;
    // Contiguous zero-byte runs of at least this length compress into a canonical .zero directive.
    inline constexpr std::size_t CanonicalZeroRunMinBytes = 8;

    enum class SectionKind : std::uint8_t
    {
        None,
        Data,
        Text,
    };

    inline core::Diagnostic makeTextDiagnostic(std::size_t BeginOffset, std::size_t EndOffset, std::string_view Description)
    {
        return core::makeDiagnostic<core::DiagnosticKind::VMTextInvalid>(core::SourceRange::fromByteOffsets(BeginOffset, EndOffset), std::string(Description));
    }

    inline core::Diagnostic makeTextDiagnostic(std::size_t Offset, std::string_view Description)
    {
        return makeTextDiagnostic(Offset, Offset, Description);
    }

    inline core::Diagnostic makeWriteDiagnostic(std::string_view Description)
    {
        return core::makeDiagnostic<core::DiagnosticKind::VMTextInvalid>(core::SourceRange{}, std::string(Description));
    }

    constexpr bool isIdentifierStart(char Character) noexcept
    {
        return (Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z') || Character == '_' || Character == '.' || Character == '$';
    }

    constexpr bool isDecimalDigit(char Character) noexcept
    {
        return Character >= '0' && Character <= '9';
    }

    constexpr bool isIdentifierContinue(char Character) noexcept
    {
        return isIdentifierStart(Character) || isDecimalDigit(Character);
    }

    constexpr bool isLabelIdentifier(std::string_view Text) noexcept
    {
        if (Text.empty() || !isIdentifierStart(Text.front()))
        {
            return false;
        }
        for (const char Character : Text)
        {
            if (!isIdentifierContinue(Character))
            {
                return false;
            }
        }
        return true;
    }

    constexpr int hexDigitValue(char Character) noexcept
    {
        if (isDecimalDigit(Character))
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

    inline void appendQuotedText(std::string& Output, std::string_view Text)
    {
        Output += '"';
        for (const unsigned char Byte : Text)
        {
            if (Byte == '"' || Byte == '\\')
            {
                Output += '\\';
                Output += static_cast<char>(Byte);
            }
            else if (Byte >= 0x20 && Byte <= 0x7E)
            {
                Output += static_cast<char>(Byte);
            }
            else
            {
                Output += '\\';
                Output += UpperHexDigits[Byte >> 4];
                Output += UpperHexDigits[Byte & 0x0F];
            }
        }
        Output += '"';
    }

    inline std::expected<std::string, core::Diagnostic> decodeQuotedText(std::string_view Text, std::size_t QuoteStart, std::size_t& Index)
    {
        std::string Decoded;
        while (Index < Text.size())
        {
            const unsigned char Next = static_cast<unsigned char>(Text[Index++]);
            if (Next == '"')
            {
                return Decoded;
            }
            if (Next == '\n' || Next == '\r')
            {
                return std::unexpected(makeTextDiagnostic(QuoteStart, Index - 1, "unterminated quoted string"));
            }
            if (Next == '\\')
            {
                const std::size_t EscapeStart = Index - 1;
                if (Index == Text.size() || Text[Index] == '\n' || Text[Index] == '\r')
                {
                    return std::unexpected(makeTextDiagnostic(EscapeStart, Index, "incomplete string escape"));
                }
                const char Escaped = Text[Index++];
                if (Escaped == '"' || Escaped == '\\')
                {
                    Decoded.push_back(Escaped);
                }
                else if (Index < Text.size() && hexDigitValue(Escaped) >= 0 && hexDigitValue(Text[Index]) >= 0)
                {
                    const int ByteValue = hexDigitValue(Escaped) * 16 + hexDigitValue(Text[Index++]);
                    Decoded.push_back(static_cast<char>(ByteValue));
                }
                else
                {
                    const std::size_t EscapeEnd = Index < Text.size() && Text[Index] != '"' && Text[Index] != '\n' && Text[Index] != '\r' ? Index + 1 : Index;
                    return std::unexpected(makeTextDiagnostic(EscapeStart, EscapeEnd, "expected a two-digit hexadecimal byte escape"));
                }
            }
            else if (Next < 0x20 || Next > 0x7E)
            {
                return std::unexpected(makeTextDiagnostic(Index - 1, Index, "bytes outside printable ASCII must be escaped"));
            }
            else
            {
                Decoded.push_back(static_cast<char>(Next));
            }
        }
        return std::unexpected(makeTextDiagnostic(QuoteStart, Index, "unterminated quoted string"));
    }

    constexpr std::optional<SectionKind> parseSectionKind(std::string_view Text) noexcept
    {
        if (Text == DataSectionName)
        {
            return SectionKind::Data;
        }
        if (Text == TextSectionName)
        {
            return SectionKind::Text;
        }
        return std::nullopt;
    }

    constexpr std::optional<MachineSymbolKind> parseSymbolKind(std::string_view Text) noexcept
    {
        if (Text == "function")
        {
            return MachineSymbolKind::Function;
        }
        if (Text == "global")
        {
            return MachineSymbolKind::GlobalVariable;
        }
        return std::nullopt;
    }

    constexpr std::optional<std::string_view> symbolKindName(MachineSymbolKind Kind) noexcept
    {
        switch (Kind)
        {
        case MachineSymbolKind::Function:
            return "function";
        case MachineSymbolKind::GlobalVariable:
            return "global";
        }
        return std::nullopt;
    }

    constexpr std::optional<MachineSymbolLinkage> parseLinkage(std::string_view Text) noexcept
    {
        if (Text == "local")
        {
            return MachineSymbolLinkage::Local;
        }
        if (Text == "external")
        {
            return MachineSymbolLinkage::External;
        }
        return std::nullopt;
    }

    constexpr std::optional<std::string_view> linkageName(MachineSymbolLinkage Linkage) noexcept
    {
        switch (Linkage)
        {
        case MachineSymbolLinkage::Local:
            return "local";
        case MachineSymbolLinkage::External:
            return "external";
        }
        return std::nullopt;
    }

    inline std::expected<core::Name, core::Diagnostic> internTextName(std::string_view Text, std::size_t BeginOffset, std::size_t EndOffset)
    {
        const core::Name Name = core::NamePool::instance().intern(Text);
        if (!Text.empty() && !Name.valid())
        {
            return std::unexpected(makeTextDiagnostic(BeginOffset, EndOffset, "name pool capacity exhausted"));
        }
        return Name;
    }

    using ParsedOperand = std::variant<std::monostate, RegisterOperand, AddressOperand, ImmediateOperand, BasicBlockOperand>;

    constexpr std::optional<OperandKind> parsedOperandKind(const ParsedOperand& Value) noexcept
    {
        if (std::holds_alternative<RegisterOperand>(Value))
        {
            return OperandKind::Register;
        }
        if (std::holds_alternative<AddressOperand>(Value))
        {
            return OperandKind::Address;
        }
        if (std::holds_alternative<ImmediateOperand>(Value))
        {
            return OperandKind::Immediate;
        }
        if (std::holds_alternative<BasicBlockOperand>(Value))
        {
            return OperandKind::BasicBlock;
        }
        return std::nullopt;
    }

    template <typename OperandType>
    consteval OperandKind operandKindForType() noexcept
    {
        if constexpr (std::is_same_v<OperandType, RegisterOperand>)
        {
            return OperandKind::Register;
        }
        else if constexpr (std::is_same_v<OperandType, AddressOperand>)
        {
            return OperandKind::Address;
        }
        else if constexpr (std::is_same_v<OperandType, ImmediateOperand>)
        {
            return OperandKind::Immediate;
        }
        else
        {
            static_assert(std::is_same_v<OperandType, BasicBlockOperand>);
            return OperandKind::BasicBlock;
        }
    }

    template <Opcode Operation, std::size_t... Indices>
    consteval std::array<OperandKind, sizeof...(Indices)> buildOperandKinds(std::index_sequence<Indices...>) noexcept
    {
        using Tuple = typename OpcodeTraits<Operation>::OperandTuple;
        return {operandKindForType<std::tuple_element_t<Indices, Tuple>>()...};
    }

    template <Opcode Operation>
    struct OpcodeDescriptorAdapter
    {
            static constexpr std::size_t OperandCount = OpcodeTraits<Operation>::OperandCount;
            static constexpr auto OperandKinds = buildOperandKinds<Operation>(std::make_index_sequence<OperandCount>{});

            template <std::size_t... Indices>
            static MachineInstruction* createWithIndices(MachineInstructionBuilder& Builder, const std::array<ParsedOperand, 3>& Operands, std::index_sequence<Indices...>)
            {
                using Tuple = typename OpcodeTraits<Operation>::OperandTuple;
                if (!(std::holds_alternative<std::tuple_element_t<Indices, Tuple>>(Operands[Indices]) && ...))
                {
                    return nullptr;
                }
                return Builder.tryCreate<Operation>(*std::get_if<std::tuple_element_t<Indices, Tuple>>(&Operands[Indices])...);
            }

            static MachineInstruction* create(MachineInstructionBuilder& Builder, const std::array<ParsedOperand, 3>& Operands)
            {
                return createWithIndices(Builder, Operands, std::make_index_sequence<OperandCount>{});
            }
    };

    struct OpcodeDescriptor
    {
            Opcode Operation;
            std::string_view Spelling;
            InstructionCategory Category;
            InstructionDomain Domain;
            std::uint32_t BitWidth;
            bool HasDestination;
            bool HasBlockTarget;
            std::span<const OperandKind> OperandKinds;
            MachineInstruction* (*Create)(MachineInstructionBuilder& Builder, const std::array<ParsedOperand, 3>& Operands);
    };

    inline constexpr OpcodeDescriptor OpcodeDescriptors[] = {
#define OPCODE(Kind, Encoding, Spelling, Form, Width, RegisterCount, AddressCount, ImmediateCount, Category, Domain) \
    {Opcode::Kind, Spelling, InstructionCategory::Category, InstructionDomain::Domain, Width, OpcodeTraits<Opcode::Kind>::HasDestination, OpcodeTraits<Opcode::Kind>::BlockCount == 1, OpcodeDescriptorAdapter<Opcode::Kind>::OperandKinds, &OpcodeDescriptorAdapter<Opcode::Kind>::create},
#include "ink/backend/vm/instruction/opcode.def"
#undef OPCODE
    };

    struct MnemonicSignature
    {
            bool HasBlockTarget = false;
            std::size_t BlockTargetIndex = 0;
    };

    constexpr std::optional<MnemonicSignature> lookupMnemonic(std::string_view Spelling) noexcept
    {
        bool Found = false;
        MnemonicSignature Signature{};
        for (const OpcodeDescriptor& Entry : OpcodeDescriptors)
        {
            if (Entry.Spelling != Spelling)
            {
                continue;
            }
            if (!Found)
            {
                Found = true;
                Signature.HasBlockTarget = Entry.HasBlockTarget;
                Signature.BlockTargetIndex = Entry.HasBlockTarget ? Entry.OperandKinds.size() - 1 : 0;
            }
            else if (Signature.HasBlockTarget != Entry.HasBlockTarget || (Entry.HasBlockTarget && Signature.BlockTargetIndex != Entry.OperandKinds.size() - 1))
            {
                return std::nullopt;
            }
        }
        return Found ? std::optional{Signature} : std::nullopt;
    }

    consteval bool verifyOpcodeDescriptorTable() noexcept
    {
        for (std::size_t First = 0; First < std::size(OpcodeDescriptors); ++First)
        {
            const OpcodeDescriptor& LHS = OpcodeDescriptors[First];
            for (std::size_t Second = First + 1; Second < std::size(OpcodeDescriptors); ++Second)
            {
                const OpcodeDescriptor& RHS = OpcodeDescriptors[Second];
                if (LHS.Spelling != RHS.Spelling)
                {
                    continue;
                }
                if (LHS.HasBlockTarget != RHS.HasBlockTarget)
                {
                    return false;
                }
                if (LHS.HasBlockTarget && LHS.OperandKinds.size() != RHS.OperandKinds.size())
                {
                    return false;
                }
                if (LHS.OperandKinds.size() == RHS.OperandKinds.size())
                {
                    bool SameKinds = true;
                    for (std::size_t Index = 0; Index < LHS.OperandKinds.size(); ++Index)
                    {
                        if (LHS.OperandKinds[Index] != RHS.OperandKinds[Index])
                        {
                            SameKinds = false;
                            break;
                        }
                    }
                    if (SameKinds)
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    static_assert(verifyOpcodeDescriptorTable(), "Overloaded VM mnemonics must have distinct operand signatures and consistent block-target positions");

    constexpr const OpcodeDescriptor* findOpcodeDescriptor(std::string_view Spelling, std::span<const OperandKind> Kinds) noexcept
    {
        for (const OpcodeDescriptor& Entry : OpcodeDescriptors)
        {
            if (Entry.Spelling == Spelling && Entry.OperandKinds.size() == Kinds.size())
            {
                bool Matches = true;
                for (std::size_t Index = 0; Index < Kinds.size(); ++Index)
                {
                    if (Entry.OperandKinds[Index] != Kinds[Index])
                    {
                        Matches = false;
                        break;
                    }
                }
                if (Matches)
                {
                    return &Entry;
                }
            }
        }
        return nullptr;
    }

    inline std::optional<std::string_view> validateParsedInstruction(const OpcodeDescriptor& Descriptor, const std::array<ParsedOperand, 3>& Operands) noexcept
    {
        if (Descriptor.Domain == InstructionDomain::FloatingPoint)
        {
            for (std::size_t Index = 0; Index < Descriptor.OperandKinds.size(); ++Index)
            {
                const auto& Operand = std::get<RegisterOperand>(Operands[Index]);
                if (!isValidFloatingPointOperand(Index, Operand))
                {
                    return "floating-point operands require a GPR destination and GPR or ZERO sources, all with zero displacement";
                }
            }
        }
        if (Descriptor.HasDestination)
        {
            if (const auto* Destination = std::get_if<RegisterOperand>(&Operands[0]))
            {
                if (!isValidRegisterDestination(*Destination))
                {
                    return "a register destination must be BP, SP, or R0-R9 with zero displacement";
                }
            }
        }
        if (Descriptor.Category == InstructionCategory::Call || Descriptor.Operation == Opcode::Jmpi)
        {
            if (const auto* Target = std::get_if<ImmediateOperand>(&Operands[0]))
            {
                if (!isValidDirectFunctionTarget(*Target))
                {
                    return "a direct function symbol cannot have a nonzero displacement";
                }
            }
        }
        if (Descriptor.BitWidth != 64)
        {
            for (std::size_t Index = 0; Index < Descriptor.OperandKinds.size(); ++Index)
            {
                if (const auto* Immediate = std::get_if<ImmediateOperand>(&Operands[Index]); Immediate && Immediate->getSymbol())
                {
                    return "symbol address immediate requires a 64-bit instruction";
                }
            }
        }
        return std::nullopt;
    }
} // namespace ink::vm::detail
