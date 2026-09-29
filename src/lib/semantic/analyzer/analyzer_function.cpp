#include "analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeFunctionDecl(AnalysisState &State, const parser::FunctionDecl &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
