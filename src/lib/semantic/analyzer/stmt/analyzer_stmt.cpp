#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

#include <cstdlib>

namespace ink::semantic
{
  bool Analyzer::analyzeStmt(AnalysisState &State, const parser::Stmt &Stmt)
  {
    switch (Stmt.getKind())
    {
#define INK_ANALYZE_Root(Name)
#define INK_ANALYZE_Expr(Name)
#define INK_ANALYZE_Stmt(Name) \
  case parser::ASTKind::Name: \
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

  bool Analyzer::analyzeSimpleStmt(AnalysisState &, const parser::SimpleStmt &)
  {
    // TODO: Rebuild semantic analysis for this node; the placeholder must not report success.
    return false;
  }

  bool Analyzer::analyzeBlockStmt(AnalysisState &, const parser::BlockStmt &)
  {
    // TODO: Rebuild semantic analysis for this node; the placeholder must not report success.
    return false;
  }

  bool Analyzer::analyzeDeclStmt(AnalysisState &, const parser::DeclStmt &)
  {
    // TODO: Rebuild semantic analysis for this node; the placeholder must not report success.
    return false;
  }
} // namespace ink::semantic
