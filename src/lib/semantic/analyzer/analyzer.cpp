#include "analyzer_internal.h"

#include "ink/parser/parser.h"

namespace ink::semantic
{
  bool Analyzer::analyze(SemanticContext &Context, const parser::ParseResult &Input)
  {
    if (!Input.succeeded() || !Input.Unit->root())
    {
      return false;
    }
    const auto &Lexed = Input.Unit->input().lexedFile();
    if (!Lexed.isRegisteredWith(Context.sourceManager()))
    {
      return false;
    }

    // State belongs to this call; sibling statements are visited even after a recoverable failure.
    AnalysisState State(Context, Input.Unit->input());
    bool Succeeded = true;
    for (const parser::Stmt *Stmt : Input.Unit->root()->statements())
    {
      if (!analyzeStmt(State, *Stmt))
      {
        Succeeded = false;
      }
    }
    return Succeeded;
  }

  bool Analyzer::reportUnsupported(AnalysisState &State, const parser::ASTNodeBase &Node)
  {
    State.report<core::DiagnosticKind::SemanticUnsupported>(Node.getSourceRange(), parser::astKindName(Node.getKind()));
    return false;
  }

  bool Analyzer::analyzeMissingStmt(AnalysisState &State, const parser::MissingStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeErrorStmt(AnalysisState &State, const parser::ErrorStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeMissingDecl(AnalysisState &State, const parser::MissingDecl &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeErrorDecl(AnalysisState &State, const parser::ErrorDecl &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
