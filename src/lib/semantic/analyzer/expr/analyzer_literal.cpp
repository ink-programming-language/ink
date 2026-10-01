#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

#include <variant>

namespace ink::semantic
{
  using namespace ink::ir;

  Analyzer::ExpressionResult Analyzer::analyzeLiteralExpr(AnalysisState &State, const parser::LiteralExpr &Node)
  {
    if (Node.literalKind() == tokenizer::TokenKind::IntegerLiteral)
    {
      if (State.Evaluating)
      {
        const auto *Target = State.ExpectedType && IntegerType::classof(State.ExpectedType) ? static_cast<const IntegerType *>(State.ExpectedType) : State.Context.typePool().getType<TypeKind::Integer>(32, true);
        return {convertExpression(State, {nullptr, &Node}, *Target, Node)};
      }
      return {nullptr, &Node};
    }
    if (Node.literalKind() == tokenizer::TokenKind::StringLiteral)
    {
      const auto *String = std::get_if<tokenizer::StringInfo>(&State.Input.token(Node.token()).Payload);
      if (!String)
      {
        State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
        return {};
      }
      const auto *U8 = State.Context.typePool().getType<TypeKind::Integer>(8, false);
      const auto *Slice = State.Context.typePool().getType<TypeKind::Slice>(*U8, AccessKind::ReadOnly);
      return {State.Context.constantPool().getStringConstant(*Slice, String->Decoded)};
    }
    reportUnsupported(State, Node);
    return {};
  }
} // namespace ink::semantic
