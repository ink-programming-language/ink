#pragma once

#include "ink/core/diagnostic/diagnostic_builder.h"
#include "ink/ir/instruction/instruction_kind.h"
#include "ink/ir/text/module_text.h"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string>
#include <string_view>
#include <vector>

namespace ink::ir::text
{
  enum class ExpressionKind
  {
    Reference,
    MetaType,
    BoolType,
    VoidType,
    IntegerType,
    FloatType,
    PointerType,
    ArrayType,
    FunctionType,
    IntegerConstant,
    FloatConstant,
    StringConstant,
  };

  // Children: pointer/array element; function return then fixed parameter types; numeric constant type.
  // Reference Text includes its '%' or '@' prefix; string constants store decoded bytes in Text.
  struct Expression
  {
      ExpressionKind Kind = ExpressionKind::Reference;
      std::string Text;
      std::uint64_t Bits = 0;
      std::uint64_t Count = 0;
      std::uint32_t Width = 0;
      bool Signed = true;
      CallABI ABI = {};
      bool Variadic = false;
      std::vector<Parameter::Kind> ParameterKinds;
      std::vector<Expression> Children;
      std::size_t Offset = 0;
  };

  struct IncomingSyntax
  {
      Expression Value;
      std::string Block;
      std::size_t Offset = 0;
  };

  struct InstructionSyntax
  {
      std::string Id;
      InstructionKind Kind = InstructionKind::Return;
      Expression ResultType;
      std::vector<Expression> Operands;
      std::vector<std::string> Targets;
      std::vector<IncomingSyntax> Incoming;
      std::size_t Offset = 0;
  };

  struct BlockSyntax
  {
      std::string Id;
      std::string Name;
      std::vector<InstructionSyntax> Instructions;
      std::size_t Offset = 0;
  };

  struct ArgumentSyntax
  {
      std::string Id;
      std::string Name;
      std::size_t Offset = 0;
  };

  struct FunctionSyntax
  {
      std::string Id;
      std::string Name;
      Expression Signature;
      SymbolVisibility Visibility = SymbolVisibility::Private;
      NativeLinkage Linkage = NativeLinkage::None;
      std::vector<ArgumentSyntax> Arguments;
      std::vector<BlockSyntax> Blocks;
      std::size_t Offset = 0;
  };

  struct FieldSyntax
  {
      std::string Name;
      Expression Type;
      MemberKind Kind = MemberKind::Instance;
      SymbolVisibility Visibility = SymbolVisibility::Public;
      std::size_t Offset = 0;
  };

  struct MethodSyntax
  {
      MemberKind Kind = MemberKind::Instance;
      FunctionSyntax Function;
      std::size_t Offset = 0;
  };

  struct RecordSyntax
  {
      std::string Id;
      bool IsObject = false;
      // Borrowed reachable records may be defined without adding them to the module declaration lists.
      bool Registered = true;
      std::vector<Expression> Bases;
      std::vector<FieldSyntax> Fields;
      std::vector<MethodSyntax> Methods;
      std::size_t Offset = 0;
  };

  struct GlobalSyntax
  {
      std::string Id;
      std::string Name;
      Expression Type;
      SymbolVisibility Visibility = SymbolVisibility::Private;
      NativeLinkage Linkage = NativeLinkage::None;
      std::size_t Offset = 0;
  };

  struct ModuleSyntax
  {
      std::string Name;
      std::vector<RecordSyntax> Records;
      std::vector<GlobalSyntax> Globals;
      std::vector<FunctionSyntax> Functions;
  };

  inline core::Diagnostic error(std::size_t Offset, std::string_view Description)
  {
    return core::makeDiagnostic<core::DiagnosticKind::IRTextInvalid>(core::SourceRange::fromByteOffsets(Offset, Offset), std::string(Description));
  }

  [[nodiscard]] std::expected<ModuleSyntax, core::Diagnostic> extractModule(const Module &ModuleValue);
  [[nodiscard]] std::expected<std::unique_ptr<ParsedModule>, core::Diagnostic> buildModule(const ModuleSyntax &Syntax);
  [[nodiscard]] bool validateTextGraph(const Module &ModuleValue, std::string &Error);
} // namespace ink::ir::text
