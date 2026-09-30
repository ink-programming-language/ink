#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeInterfaceDecl(AnalysisState &State, const parser::InterfaceDecl &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
