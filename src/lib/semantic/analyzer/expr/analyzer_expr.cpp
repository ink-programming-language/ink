#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  Analyzer::ExpressionResult Analyzer::analyzeExpr(AnalysisState &State, const parser::Expr &Node, std::size_t Depth)
  {
    if (Depth >= State.ExpressionDepthLimit)
    {
      State.report<core::DiagnosticKind::SemanticNestingLimit>(Node.getSourceRange());
      return {};
    }
    if (parser::ParenExpr::classof(&Node))
    {
      return analyzeParenExpr(State, static_cast<const parser::ParenExpr &>(Node), Depth);
    }
    if (parser::LiteralExpr::classof(&Node))
    {
      return analyzeLiteralExpr(State, static_cast<const parser::LiteralExpr &>(Node));
    }
    if (parser::NameExpr::classof(&Node))
    {
      return analyzeNameExpr(State, static_cast<const parser::NameExpr &>(Node));
    }
    if (parser::UnaryExpr::classof(&Node))
    {
      return analyzeUnaryExpr(State, static_cast<const parser::UnaryExpr &>(Node), Depth);
    }
    if (parser::CallExpr::classof(&Node))
    {
      return analyzeCallExpr(State, static_cast<const parser::CallExpr &>(Node), Depth);
    }
    reportUnsupported(State, Node);
    return {};
  }
} // namespace ink::semantic
