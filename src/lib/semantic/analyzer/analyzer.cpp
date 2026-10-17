#include "ink/semantic/analyzer/analyzer.h"

#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/frontend/parser/parser.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/scope_guard.h"

namespace ink::semantic
{
  bool Analyzer::analyze(SemanticContext &Context, const frontend::parser::ParseResult &Input, ir::Module &Output)
  {
    if (&Output.irContext() != &Context.irContext() || !Input.succeeded())
    {
      return false;
    }
    const auto &Lexed = Input.Unit->input().lexedFile();
    if (!Lexed.isRegisteredWith(Context.sourceManager()))
    {
      return false;
    }
    Context.setSource(Lexed.sourceId());
    if (!Input.Unit->root())
    {
      Context.report<core::DiagnosticKind::SemanticInvalidAST>({}, "successful parse result requires a module AST root");
      return false;
    }

    // Preserve caller-owned output and pooled IR, with fresh traversal state for this call.
    AnalyzerState State(Context, Input, Output);
    Scope *ModuleScope = Context.scopeStore().memberScope(Output);
    if (!ModuleScope)
    {
      ModuleScope = Context.scopeStore().createScope(State.Resolver.rootScope(), Output);
    }
    if (!ModuleScope)
    {
      Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Input.Unit->root()->getSourceRange());
      return false;
    }
    ScopeGuard ModuleGuard(State.Resolver, *ModuleScope);
    // TODO: Restore declaration collection and forward references before analyzing function bodies.
    bool Succeeded = true;
    for (const frontend::parser::Stmt *Stmt : Input.Unit->root()->statements())
    {
      if (!analyzeStmt(State, *Stmt))
      {
        Succeeded = false;
      }
    }
    return Succeeded;
  }

  bool Analyzer::reportUnsupported(AnalyzerState &State, const frontend::parser::ASTNodeBase &Node)
  {
    State.Context.report<core::DiagnosticKind::SemanticUnsupported>(Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
    return false;
  }

  bool Analyzer::analyzeMissingStmt(AnalyzerState &State, const frontend::parser::MissingStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeErrorStmt(AnalyzerState &State, const frontend::parser::ErrorStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeMissingDecl(AnalyzerState &State, const frontend::parser::MissingDecl &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeErrorDecl(AnalyzerState &State, const frontend::parser::ErrorDecl &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
