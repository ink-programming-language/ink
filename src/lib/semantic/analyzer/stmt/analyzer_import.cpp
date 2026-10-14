#include "ink/semantic/analyzer/analyzer.h"

#include "ink/semantic/context.h"
#include "ink/frontend/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeDirectImportStmt(SemanticContext &Context, const frontend::parser::DirectImportStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(Context, Node);
  }

  bool Analyzer::analyzeFromImportStmt(SemanticContext &Context, const frontend::parser::FromImportStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(Context, Node);
  }
} // namespace ink::semantic
