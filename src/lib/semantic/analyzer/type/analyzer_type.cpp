#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/type_syntax.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  const ir::Type *Analyzer::analyzeType(AnalyzerState &State, const frontend::parser::TypeSyntax &Node)
  {
    CHECK_ICE(State.at(Node), Node.expression(), core::DiagnosticKind::SemanticInvalidAST, "TypeSyntax requires an expression");
    const auto &Expression = *Node.expression();
    const auto ExpressionSite = State.at(Expression);
    const ir::Value *Value = analyzeExpr(State, Expression);
    RETURN_IF(!Value, nullptr);
    RETURN_REPORT_IF(ExpressionSite, Value->isStorage(), core::DiagnosticKind::SemanticTypeMismatch, nullptr, "a type", "variable storage");
    return requireType(ExpressionSite, Value);
  }

  const ir::Type *Analyzer::analyzeTypeWithLayout(AnalyzerState &State, const frontend::parser::TypeSyntax &Node, std::string_view Description)
  {
    const ir::Type *Type = analyzeType(State, Node);
    RETURN_IF_NULL(Type, nullptr);
    CHECK_HAS_LAYOUT(State.at(Node), Type, Description, nullptr);
    return Type;
  }
} // namespace ink::semantic
