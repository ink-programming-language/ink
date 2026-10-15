#include "ink/semantic/analyzer/analyzer.h"

#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/block_depth_guard.h"
#include "ink/semantic/context.h"
#include "ink/frontend/parser/ast.h"

#include <cstdlib>

namespace ink::semantic
{
  bool Analyzer::analyzeStmt(AnalyzerState &State, const frontend::parser::Stmt &Stmt)
  {
    // Compile-time evaluation will be restored with the semantic implementation.
    if (Stmt.isComptime())
    {
      return reportUnsupported(State, Stmt);
    }
    switch (Stmt.getKind())
    {
#define INK_ANALYZE_Root(Name)
#define INK_ANALYZE_Expr(Name)
#define INK_ANALYZE_Stmt(Name) \
  case frontend::parser::ASTKind::Name: \
    return analyze##Name(State, static_cast<const frontend::parser::Name &>(Stmt));
#define INK_ANALYZE_Decl(Name)
#define INK_ANALYZE_SimpleItem(Name)
#define INK_ANALYZE_BindingPattern(Name)
#define INK_ANALYZE_MatchPattern(Name)
#define AST_NODE(Name, Base, Category, Id) INK_ANALYZE_##Category(Name)
#include "ink/frontend/parser/ASTNodes.def"
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

  bool Analyzer::analyzeSimpleStmt(AnalyzerState &State, const frontend::parser::SimpleStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeBlockStmt(AnalyzerState &State, const frontend::parser::BlockStmt &Node)
  {
    if (State.RecursionDepth >= State.RecursionDepthLimit)
    {
      State.Context.report<core::DiagnosticKind::SemanticNestingLimit>(Node.getSourceRange());
      return false;
    }
    BlockDepthGuard DepthGuard(State.RecursionDepth);
    bool Succeeded = true;
    for (const frontend::parser::Stmt *Stmt : Node.statements())
    {
      if (!analyzeStmt(State, *Stmt))
      {
        Succeeded = false;
      }
    }
    return Succeeded;
  }

  bool Analyzer::analyzeDeclStmt(AnalyzerState &State, const frontend::parser::DeclStmt &Node)
  {
    return analyzeDecl(State, *Node.declaration());
  }
} // namespace ink::semantic
