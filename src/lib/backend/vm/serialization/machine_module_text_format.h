#pragma once

#include "ink/backend/vm/module/machine_symbol.h"
#include "ink/core/diagnostic/diagnostic.h"
#include "ink/core/diagnostic/diagnostic_builder.h"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <string>
#include <string_view>

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
    // Canonical positive IEEE 754 binary64 quiet NaN bit pattern for deterministic .f64 parsing.
    inline constexpr std::uint64_t CanonicalQuietNaNBits = 0x7ff8000000000000ULL;
    // Upper bound on materialized global initializer bytes expanded by .zero to avoid host OOM aborts.
    inline constexpr std::size_t MaxMaterializedInitializerBytes = std::size_t{256} * 1024 * 1024;

    enum class SectionKind : std::uint8_t
    {
        Data,
        Text,
    };

    struct TextSpan
    {
            std::size_t Begin = 0;
            std::size_t End = 0;

            constexpr TextSpan mergedWith(TextSpan Other) const noexcept
            {
                return TextSpan{Begin, Other.End};
            }
    };

    template <typename ValueType>
    struct Spanned
    {
            ValueType Value{};
            TextSpan Span{};
    };

    template <typename ValueType>
    using ParseResult = std::expected<ValueType, core::Diagnostic>;

    using ParseStatus = std::expected<void, core::Diagnostic>;

    inline core::Diagnostic makeTextDiagnostic(TextSpan Span, std::string_view Description)
    {
        return core::makeDiagnostic<core::DiagnosticKind::VMTextInvalid>(core::SourceRange::fromByteOffsets(Span.Begin, Span.End), std::string(Description));
    }

    inline core::Diagnostic makeTextDiagnostic(std::size_t BeginOffset, std::size_t EndOffset, std::string_view Description)
    {
        return makeTextDiagnostic(TextSpan{BeginOffset, EndOffset}, Description);
    }

    inline core::Diagnostic makeTextDiagnostic(std::size_t Offset, std::string_view Description)
    {
        return makeTextDiagnostic(TextSpan{Offset, Offset}, Description);
    }

    inline std::unexpected<core::Diagnostic> failAt(TextSpan Span, std::string_view Description)
    {
        return std::unexpected(makeTextDiagnostic(Span, Description));
    }

    constexpr bool isAsciiLetter(char Character) noexcept
    {
        return (Character >= 'a' && Character <= 'z') || (Character >= 'A' && Character <= 'Z');
    }

    constexpr bool isIdentifierStart(char Character) noexcept
    {
        return isAsciiLetter(Character) || Character == '_' || Character == '.' || Character == '$';
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
        Output.push_back('"');
        for (const unsigned char Byte : Text)
        {
            if (Byte == '"' || Byte == '\\')
            {
                Output.push_back('\\');
                Output.push_back(static_cast<char>(Byte));
            }
            else if (Byte >= 0x20 && Byte <= 0x7E)
            {
                Output.push_back(static_cast<char>(Byte));
            }
            else
            {
                Output.push_back('\\');
                Output.push_back(UpperHexDigits[Byte >> 4]);
                Output.push_back(UpperHexDigits[Byte & 0x0F]);
            }
        }
        Output.push_back('"');
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

    constexpr std::string_view symbolKindName(MachineSymbolKind Kind) noexcept
    {
        switch (Kind)
        {
        case MachineSymbolKind::Function:
            return "function";
        case MachineSymbolKind::GlobalVariable:
            return "global";
        }
        return {};
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
} // namespace ink::vm::detail
