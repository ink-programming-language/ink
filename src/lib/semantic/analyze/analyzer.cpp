#include "ink/semantic/analyze/analyzer.h"

#include "ink/parser/parser.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <cstdlib>

namespace ink::semantic
{
  struct Analyzer::AnalysisState
  {
      SemanticContext &Context;
      NameResolver &Resolver;
      core::SourceId Source;
      std::size_t BlockDepth = 0;
  };

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
    Module *Result = Context.createModule(Context.namePool().intern(ModuleName));
    if (!Result)
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

  bool Analyzer::analyzeStmt(AnalysisState &State, const parser::Stmt &Stmt)
  {
    switch (Stmt.getKind())
    {
#define INK_ANALYZE_Root(Name)
#define INK_ANALYZE_Expr(Name)
#define INK_ANALYZE_Stmt(Name) \
  case parser::ASTKind::Name:  \
    return analyze##Name(State, static_cast<const parser::Name &>(Stmt));
#define INK_ANALYZE_Decl(Name)
#define INK_ANALYZE_SimpleItem(Name)
#define INK_ANALYZE_BindingPattern(Name)
#define INK_ANALYZE_MatchPattern(Name)
#define AST_NODE(Name, Base, Category, Id) INK_ANALYZE_##Category(Name)
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
#undef INK_ANALYZE_Root
#undef INK_ANALYZE_Expr
#undef INK_ANALYZE_Stmt
#undef INK_ANALYZE_Decl
#undef INK_ANALYZE_SimpleItem
#undef INK_ANALYZE_BindingPattern
#undef INK_ANALYZE_MatchPattern
    default:
      std::abort();
    }
  }

  bool Analyzer::analyzeDecl(AnalysisState &State, const parser::Decl &Declaration)
  {
    switch (Declaration.getKind())
    {
#define INK_ANALYZE_Root(Name)
#define INK_ANALYZE_Expr(Name)
#define INK_ANALYZE_Stmt(Name)
#define INK_ANALYZE_Decl(Name) \
  case parser::ASTKind::Name:  \
    return analyze##Name(State, static_cast<const parser::Name &>(Declaration));
#define INK_ANALYZE_SimpleItem(Name)
#define INK_ANALYZE_BindingPattern(Name)
#define INK_ANALYZE_MatchPattern(Name)
#define AST_NODE(Name, Base, Category, Id) INK_ANALYZE_##Category(Name)
#include "ink/parser/ASTNodes.def"
#undef AST_NODE
#undef INK_ANALYZE_Root
#undef INK_ANALYZE_Expr
#undef INK_ANALYZE_Stmt
#undef INK_ANALYZE_Decl
#undef INK_ANALYZE_SimpleItem
#undef INK_ANALYZE_BindingPattern
#undef INK_ANALYZE_MatchPattern
    default:
      std::abort();
    }
  }

  bool Analyzer::reportUnsupported(AnalysisState &State, const parser::ASTNodeBase &Node)
  {
    auto Diagnostic = core::makeDiagnostic<core::DiagnosticKind::SemanticUnsupported>(Node.getSourceRange(), parser::astKindName(Node.getKind()));
    Diagnostic.Source = State.Source;
    State.Context.compilationContext().diagnosticEngine().report(Diagnostic);
    return false;
  }

  bool Analyzer::analyzeMissingStmt(AnalysisState &State, const parser::MissingStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeErrorStmt(AnalysisState &State, const parser::ErrorStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeSimpleStmt(AnalysisState &State, const parser::SimpleStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeBlockStmt(AnalysisState &State, const parser::BlockStmt &Node)
  {
    if (State.BlockDepth == 256)
    {
      auto Diagnostic = core::makeDiagnostic<core::DiagnosticKind::SemanticNestingLimit>(Node.getSourceRange());
      Diagnostic.Source = State.Source;
      State.Context.compilationContext().diagnosticEngine().report(Diagnostic);
      return false;
    }
    ++State.BlockDepth;
    State.Resolver.enterScope();
    bool Succeeded = true;
    for (const parser::Stmt *Stmt : Node.statements())
    {
      if (!analyzeStmt(State, *Stmt))
      {
        Succeeded = false;
      }
    }
    State.Resolver.exitScope();
    --State.BlockDepth;
    return Succeeded;
  }

  bool Analyzer::analyzeDeclStmt(AnalysisState &State, const parser::DeclStmt &Node)
  {
    return analyzeDecl(State, *Node.declaration());
  }

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

  bool Analyzer::analyzeComptimeStmt(AnalysisState &State, const parser::ComptimeStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeDirectImportStmt(AnalysisState &State, const parser::DirectImportStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeFromImportStmt(AnalysisState &State, const parser::FromImportStmt &Node)
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

  bool Analyzer::analyzeVarDecl(AnalysisState &State, const parser::VarDecl &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeFieldDecl(AnalysisState &State, const parser::FieldDecl &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeFunctionDecl(AnalysisState &State, const parser::FunctionDecl &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeClassDecl(AnalysisState &State, const parser::ClassDecl &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeEnumDecl(AnalysisState &State, const parser::EnumDecl &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeInterfaceDecl(AnalysisState &State, const parser::InterfaceDecl &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
