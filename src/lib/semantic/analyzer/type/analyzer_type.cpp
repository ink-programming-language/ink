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
    RETURN_REPORT_IF_FALSE(State.semanticContext(), Node.expression(), core::DiagnosticKind::SemanticInvalidAST, nullptr, Node.getSourceRange(), "TypeSyntax requires an expression");
    const auto &Expression = *Node.expression();
    const ir::Value *Value = analyzeExpr(State, Expression);
    RETURN_IF(!Value, nullptr);
    RETURN_REPORT_IF(State.semanticContext(), Value->isStorage(), core::DiagnosticKind::SemanticTypeMismatch, nullptr, Expression.getSourceRange(), "a type", "variable storage");
    return requireType(State.semanticContext(), Expression.getSourceRange(), Value);
  }
} // namespace ink::semantic
