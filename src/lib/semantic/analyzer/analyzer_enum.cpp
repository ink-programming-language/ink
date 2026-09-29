#include "analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeEnumDecl(AnalysisState &State, const parser::EnumDecl &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
