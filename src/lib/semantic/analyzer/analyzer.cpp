#include "analyzer_internal.h"

#include "ink/parser/parser.h"
#include "ink/semantic/context.h"
#include "ink/semantic/ir_builder.h"
#include "ink/semantic/name_resolve/name_resolver.h"

namespace ink::semantic
{
  Module *Analyzer::analyze(SemanticContext &Context, const parser::ParseResult &Input, std::string_view ModuleName)
  {
    if (!Input.succeeded() || !Input.Unit->root())
    {
      return nullptr;
    }
    const auto &Lexed = Input.Unit->input().lexedFile();
    if (!Lexed.isRegisteredWith(Context.compilationContext().sourceManager()))
    {
      return nullptr;
    }
    const parser::ModuleAST &Root = *Input.Unit->root();
    IRBuilder Builder(Context);
    Module *Result = Builder.createModule(Context.namePool().intern(ModuleName));
    if (!Result || !Builder.createModuleDecl(*Result, Root))
    {
      auto Diagnostic = core::makeDiagnostic<core::DiagnosticKind::SemanticConstructionFailed>(Root.getSourceRange());
      Diagnostic.Source = Lexed.sourceId();
      Context.compilationContext().diagnosticEngine().report(Diagnostic);
      return nullptr;
    }

    // The resolver creates the root; the module has its own member scope below it.
    NameResolver Resolver(Context);
    if (!Resolver.enterScope(*Result))
    {
      return nullptr;
    }
    AnalysisState State{Context, Resolver, Lexed.sourceId()};

    // Builtin registration and declaration collection will precede body checking
    // when those features are implemented. No unsupported declaration is published.
    bool Succeeded = true;
    for (const parser::Stmt *Stmt : Root.statements())
    {
      if (!analyzeStmt(State, *Stmt))
      {
        Succeeded = false;
      }
    }
    return Succeeded ? Result : nullptr;
  }

  bool Analyzer::reportUnsupported(AnalysisState &State, const parser::ASTNodeBase &Node)
  {
    auto Diagnostic = core::makeDiagnostic<core::DiagnosticKind::SemanticUnsupported>(Node.getSourceRange(), parser::astKindName(Node.getKind()));
    Diagnostic.Source = State.Source;
    State.Context.compilationContext().diagnosticEngine().report(Diagnostic);
    return false;
  }

  // Recovery nodes are unsupported; analyze() rejects unsuccessful parse results.
  bool Analyzer::analyzeMissingStmt(AnalysisState &State, const parser::MissingStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeErrorStmt(AnalysisState &State, const parser::ErrorStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeMissingDecl(AnalysisState &State, const parser::MissingDecl &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeErrorDecl(AnalysisState &State, const parser::ErrorDecl &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
