#include "analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeClassDecl(AnalysisState &State, const parser::ClassDecl &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
