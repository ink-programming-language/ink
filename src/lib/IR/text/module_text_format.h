#pragma once

#include "ink/core/diagnostic/diagnostic_builder.h"
#include "ink/core/name/name_pool.h"
#include "ink/ir/enums.h"
#include "ink/ir/function/parameter.h"
#include "ink/ir/instruction/instruction_kind.h"
#include "ink/ir/type/call_abi.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

// Formal Ink IR Text Grammar (LLVM IR style):
//
//   Module             := ModuleHeader TopLevelDeclaration*
//   ModuleHeader       := 'module_path' '=' QuotedString
//   TopLevelDeclaration:= ImportDeclaration | RecordDeclaration | GlobalDeclaration | FunctionDeclaration
//   ImportDeclaration  := 'import' ImportId '=' QuotedString
//   RecordDeclaration  := LocalId '=' 'type' ('struct' | 'class') ['borrowed' | 'from' ImportId] [QuotedString]
//                         ['extends' '(' Type (',' Type)* ')']
//                         '{' FieldDeclaration* MethodDeclaration* '}'
//   FieldDeclaration   := 'field' Visibility MemberKind Type [QuotedString]
//   MethodDeclaration  := 'method' MemberKind FunctionDeclaration
//   GlobalDeclaration  := GlobalId [QuotedString] '=' Visibility [Linkage | 'from' ImportId] 'global' Type
//   FunctionDeclaration:= 'declare' Visibility [Linkage | 'from' ImportId] [ABI] [CallingConvention]
//                         Type GlobalId [QuotedString] '(' BoundParameterList ')'
//                       | 'define' Visibility [Linkage] [ABI] [CallingConvention]
//                         Type GlobalId [QuotedString] '(' BoundParameterList ')'
//                         '{' BasicBlock+ '}'
//   ParameterList      := [Parameter (',' Parameter)* [',' '...'] | '...']
//   Parameter          := [ParameterKind] Type
//   BoundParameterList := [BoundParameter (',' BoundParameter)* [',' '...'] | '...']
//   BoundParameter     := [ParameterKind] Type LocalId [QuotedString]
//   BasicBlock         := LocalId [QuotedString] ':' Instruction*
//   Instruction        := LocalId '=' 'add' Type Value ',' Value
//                       | LocalId '=' 'alloca' Type
//                       | LocalId '=' 'load' Type ',' Type Value
//                       | 'store' Type Value ',' Type Value
//                       | LocalId '=' 'phi' Type PhiIncoming (',' PhiIncoming)*
//                       | [LocalId '='] 'call' Type GlobalOrLocalId '(' [TypedValue (',' TypedValue)*] ')'
//                       | 'ret' ('void' | Type Value)
//                       | 'br' 'label' LocalId
//                       | 'br' Type Value ',' 'label' LocalId ',' 'label' LocalId
//   PhiIncoming        := '[' Value ',' LocalId ']'
//   TypedValue         := Type Value
//   Type               := 'type' | 'bool' | 'void'
//                       | ('i8' | 'i16' | 'i32' | 'i64' | 'u8' | 'u16' | 'u32' | 'u64')
//                       | ('f32' | 'f64')
//                       | 'ptr' '<' Type '>'
//                       | '[' Unsigned 'x' Type ']'
//                       | 'fn' ['<' ABI CallingConvention '>'] '(' ParameterList ')' '->' Type
//                       | LocalId
//   Value              := LocalId | GlobalId | Unsigned | ByteString | Type

namespace ink::ir::detail
{
    inline constexpr std::size_t MaxTypeExpressionDepth = 128;

    inline constexpr std::string_view ModulePathKeyword = "module_path";
    inline constexpr std::string_view ImportKeyword = "import";
    inline constexpr std::string_view FromKeyword = "from";
    inline constexpr std::string_view TypeKeyword = "type";
    inline constexpr std::string_view BorrowedKeyword = "borrowed";
    inline constexpr std::string_view ExtendsKeyword = "extends";
    inline constexpr std::string_view FieldKeyword = "field";
    inline constexpr std::string_view MethodKeyword = "method";
    inline constexpr std::string_view GlobalKeyword = "global";
    inline constexpr std::string_view LabelKeyword = "label";
    inline constexpr std::string_view VoidKeyword = "void";
    inline constexpr std::string_view EllipsisToken = "...";
    inline constexpr std::string_view ArrowToken = "->";

    template <typename Enum>
    struct EnumEntry
    {
            Enum Value;
            std::string_view Spelling;
    };

    template <typename Enum, std::size_t Count>
    constexpr std::string_view enumSpelling(Enum Value, const EnumEntry<Enum> (&Entries)[Count]) noexcept
    {
        for (const EnumEntry<Enum>& Entry : Entries)
        {
            if (Entry.Value == Value)
            {
                return Entry.Spelling;
            }
        }
        return {};
    }

    template <typename Enum, std::size_t Count>
    constexpr std::optional<Enum> parseEnum(std::string_view Spelling, const EnumEntry<Enum> (&Entries)[Count]) noexcept
    {
        for (const EnumEntry<Enum>& Entry : Entries)
        {
            if (Entry.Spelling == Spelling)
            {
                return Entry.Value;
            }
        }
        return std::nullopt;
    }

    inline constexpr EnumEntry<SymbolVisibility> VisibilityEntries[] = {
        {SymbolVisibility::Private, "private"},
        {SymbolVisibility::Public, "public"},
    };

    inline constexpr EnumEntry<NativeLinkage> LinkageEntries[] = {
        {NativeLinkage::None, "none"},
        {NativeLinkage::Import, "import"},
        {NativeLinkage::Export, "export"},
    };

    inline constexpr EnumEntry<MemberKind> MemberKindEntries[] = {
        {MemberKind::Instance, "instance"},
        {MemberKind::Static, "static"},
    };

    inline constexpr EnumEntry<Parameter::Kind> ParameterKindEntries[] = {
        {Parameter::Kind::Positional, "positional"},
        {Parameter::Kind::Named, "named"},
    };

    inline constexpr EnumEntry<ABIKind> ABIEntries[] = {
        {ABIKind::Ink, "ink"},
        {ABIKind::C, "c"},
    };

    inline constexpr EnumEntry<CallingConvention> ConventionEntries[] = {
        {CallingConvention::Default, "default"},
        {CallingConvention::CDecl, "cdecl"},
        {CallingConvention::StdCall, "stdcall"},
        {CallingConvention::FastCall, "fastcall"},
        {CallingConvention::VectorCall, "vectorcall"},
    };

    inline constexpr EnumEntry<bool> RecordKindEntries[] = {
        {false, "struct"},
        {true, "class"},
    };

    inline constexpr EnumEntry<bool> FunctionKindEntries[] = {
        {false, "declare"},
        {true, "define"},
    };

    enum class TypeSyntaxKind : std::uint8_t
    {
        Reference,
        Meta,
        Bool,
        Void,
        Integer,
        Float,
        Pointer,
        Array,
        Function,
    };

    inline constexpr EnumEntry<TypeSyntaxKind> KeywordTypeEntries[] = {
        {TypeSyntaxKind::Meta, "type"},
        {TypeSyntaxKind::Bool, "bool"},
        {TypeSyntaxKind::Void, "void"},
        {TypeSyntaxKind::Pointer, "ptr"},
        {TypeSyntaxKind::Function, "fn"},
    };

    struct InstructionDescriptor
    {
            InstructionKind Kind;
            std::string_view Mnemonic;
            bool RequiresResultId;
            bool AllowsResultId;
    };

    inline constexpr InstructionDescriptor InstructionDescriptors[] = {
        {InstructionKind::Add, "add", true, true},
        {InstructionKind::Return, "ret", false, false},
        {InstructionKind::Alloca, "alloca", true, true},
        {InstructionKind::Load, "load", true, true},
        {InstructionKind::Store, "store", false, false},
        {InstructionKind::Branch, "br", false, false},
        {InstructionKind::CondBranch, "br", false, false},
        {InstructionKind::Phi, "phi", true, true},
        {InstructionKind::Call, "call", false, true},
    };

#define INSTRUCTION(EntryKind, Number, Class, Spelling) \
    static_assert(static_cast<std::size_t>(InstructionKind::EntryKind) == Number && InstructionDescriptors[Number].Kind == InstructionKind::EntryKind);
#include "ink/ir/instruction/instruction.def"
#undef INSTRUCTION

    constexpr const InstructionDescriptor& instructionDescriptor(InstructionKind Kind) noexcept
    {
        return InstructionDescriptors[static_cast<std::size_t>(Kind)];
    }

    constexpr const InstructionDescriptor* findInstructionDescriptor(std::string_view Mnemonic) noexcept
    {
        for (const InstructionDescriptor& Entry : InstructionDescriptors)
        {
            if (Entry.Mnemonic == Mnemonic)
            {
                return &Entry;
            }
        }
        return nullptr;
    }

    inline core::Diagnostic makeTextDiagnostic(std::size_t Offset, std::string_view Description)
    {
        return core::makeDiagnostic<core::DiagnosticKind::IRTextInvalid>(core::SourceRange::fromByteOffsets(Offset, Offset), std::string(Description));
    }

    inline std::string_view nameText(core::Name Value) noexcept
    {
        return core::NamePool::instance().text(Value);
    }

    inline void appendQuotedBytes(std::string& Output, std::string_view Prefix, std::string_view Bytes)
    {
        constexpr char Hex[] = "0123456789ABCDEF";
        Output.append(Prefix);
        Output.push_back('"');
        for (const unsigned char Byte : Bytes)
        {
            if (Byte < 0x20 || Byte >= 0x7f || Byte == '"' || Byte == '\\')
            {
                Output.push_back('\\');
                Output.push_back(Hex[Byte >> 4]);
                Output.push_back(Hex[Byte & 0x0f]);
            }
            else
            {
                Output.push_back(static_cast<char>(Byte));
            }
        }
        Output.push_back('"');
    }
} // namespace ink::ir::detail
