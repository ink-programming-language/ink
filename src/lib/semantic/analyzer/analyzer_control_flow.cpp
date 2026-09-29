#include "analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeIfStmt(AnalysisState &State, const parser::IfStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeWhileStmt(AnalysisState &State, const parser::WhileStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeClassicForStmt(AnalysisState &State, const parser::ClassicForStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeForInStmt(AnalysisState &State, const parser::ForInStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeSwitchStmt(AnalysisState &State, const parser::SwitchStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeReturnStmt(AnalysisState &State, const parser::ReturnStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeBreakStmt(AnalysisState &State, const parser::BreakStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeContinueStmt(AnalysisState &State, const parser::ContinueStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeYieldStmt(AnalysisState &State, const parser::YieldStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeDeferStmt(AnalysisState &State, const parser::DeferStmt &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
