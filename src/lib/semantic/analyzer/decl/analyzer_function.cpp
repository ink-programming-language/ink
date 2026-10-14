#include "ink/semantic/analyzer/analyzer.h"

#include "ink/semantic/context.h"
#include "ink/frontend/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeFunctionDecl(SemanticContext &Context, const frontend::parser::FunctionDecl &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(Context, Node);
  }
} // namespace ink::semantic
