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
    if (!Node.expression())
    {
      State.semanticContext().report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "TypeSyntax requires an expression");
      return nullptr;
    }
    const auto &Expression = *Node.expression();
    return requireType(State.semanticContext(), Expression.getSourceRange(), analyzeExpr(State, Expression));
  }
} // namespace ink::semantic
