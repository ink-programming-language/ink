#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/config/config_manager.h"
#include "ink/frontend/parser/parser.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  bool Analyzer::analyze(SemanticContext &Context, const frontend::parser::ParseResult &Input)
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

    // Preserve the context-owned IR, but refresh per-input state before dispatch.
    Context.Source = Lexed.sourceId();
    Context.BlockDepthLimit = core::ConfigManager::getSize<core::ConfigKind::SemanticBlockDepthLimit>();
    Context.BlockDepth = 0;
    Context.irBuilder().clearInsertPoint();
    bool Succeeded = true;
    for (const frontend::parser::Stmt *Stmt : Input.Unit->root()->statements())
    {
      if (!analyzeStmt(Context, *Stmt))
      {
        Succeeded = false;
      }
    }
    Context.irBuilder().clearInsertPoint();
    return Succeeded;
  }

  bool Analyzer::reportUnsupported(SemanticContext &Context, const frontend::parser::ASTNodeBase &Node)
  {
    Context.report<core::DiagnosticKind::SemanticUnsupported>(Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
    return false;
  }

  bool Analyzer::analyzeMissingStmt(SemanticContext &Context, const frontend::parser::MissingStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(Context, Node);
  }

  bool Analyzer::analyzeErrorStmt(SemanticContext &Context, const frontend::parser::ErrorStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(Context, Node);
  }

  bool Analyzer::analyzeMissingDecl(SemanticContext &Context, const frontend::parser::MissingDecl &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(Context, Node);
  }

  bool Analyzer::analyzeErrorDecl(SemanticContext &Context, const frontend::parser::ErrorDecl &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(Context, Node);
  }
} // namespace ink::semantic
