#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/type_syntax.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"

namespace ink::semantic
{
  const ir::Type *Analyzer::analyzeType(AnalyzerState &State, const frontend::parser::TypeSyntax &Node)
  {
    const auto &Expression = *Node.expression();
    return requireType(State.semanticContext(), Expression.getSourceRange(), analyzeExpr(State, Expression));
  }
} // namespace ink::semantic
