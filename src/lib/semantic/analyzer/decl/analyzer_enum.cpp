#include "ink/semantic/analyzer/analyzer.h"

#include "ink/semantic/context.h"
#include "ink/frontend/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeEnumDecl(SemanticContext &Context, const frontend::parser::EnumDecl &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(Context, Node);
  }
} // namespace ink::semantic
