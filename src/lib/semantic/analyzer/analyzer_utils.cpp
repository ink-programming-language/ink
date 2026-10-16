#include "ink/semantic/analyzer/analyzer_utils.h"

#include "ink/frontend/parser/parser.h"
#include "ink/ir/module/global_variable.h"
#include "ink/semantic/context.h"

#include <charconv>
#include <string>
#include <variant>

namespace ink::semantic
{
  const ir::Type *requireType(SemanticContext &Context, core::SourceRange Range, const ir::Value *Target)
  {
    if (!Target)
    {
      return nullptr;
    }
    if (!ir::Type::classof(Target))
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Range, "a type", ir::GlobalVariable::classof(Target) ? "variable storage" : ir::valueKindName(Target->kind()));
      return nullptr;
    }
    const auto *Type = static_cast<const ir::Type *>(Target);
    if (!Context.irContext().containsType(Type))
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Range, "a type in the current IR context", "a foreign or unregistered type");
      return nullptr;
    }
    return Type;
  }

  const ir::Value *requireRuntimeValue(SemanticContext &Context, core::SourceRange Range, const ir::Value *Target)
  {
    if (!Target)
    {
      return nullptr;
    }
    const ir::Type *Type = Target->type();
    if (!Context.irContext().containsType(Type) || (!Type->hasLayout() && !ir::NullPointerConstant::classof(Target)))
    {
      const auto Actual = ir::Type::classof(Target) ? static_cast<const ir::Type *>(Target)->toString() : std::string(ir::valueKindName(Target->kind()));
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Range, "a runtime value or variable storage", Actual);
      return nullptr;
    }
    return Target;
  }

  std::optional<std::uint64_t> parseIntegerLiteral(const frontend::parser::ParseResult &Input, const frontend::parser::LiteralExpr &Node)
  {
    if (Node.literalKind() != frontend::parser::TokenKind::IntegerLiteral)
    {
      return std::nullopt;
    }
    const auto &Tokens = Input.Unit->input();
    const auto *Numeric = std::get_if<frontend::tokenizer::NumericInfo>(&Tokens.token(Node.token()).Payload);
    if (!Numeric || (Numeric->Base != 2 && Numeric->Base != 8 && Numeric->Base != 10 && Numeric->Base != 16))
    {
      return std::nullopt;
    }
    std::string_view Text = Tokens.spelling(Node.token());
    if (Numeric->Base != 10)
    {
      if (Text.size() < 2)
      {
        return std::nullopt;
      }
      Text.remove_prefix(2);
    }
    std::uint64_t Value = 0;
    const auto Parsed = std::from_chars(Text.data(), Text.data() + Text.size(), Value, static_cast<int>(Numeric->Base));
    if (Parsed.ec != std::errc{} || Parsed.ptr != Text.data() + Text.size())
    {
      return std::nullopt;
    }
    return Value;
  }
} // namespace ink::semantic
