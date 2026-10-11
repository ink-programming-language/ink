#include "../analyzer_internal.h"

#include "ink/frontend/parser/ast.h"

#include <cstdlib>

namespace ink::semantic
{
  bool Analyzer::analyzeDecl(AnalysisState &State, const frontend::parser::Decl &Declaration)
  {
    if (Declaration.isComptime())
    {
      return reportUnsupported(State, Declaration);
    }
    switch (Declaration.getKind())
    {
#define INK_ANALYZE_Root(Name)
#define INK_ANALYZE_Expr(Name)
#define INK_ANALYZE_Stmt(Name)
#define INK_ANALYZE_Decl(Name) \
  case frontend::parser::ASTKind::Name: \
    return analyze##Name(State, static_cast<const frontend::parser::Name &>(Declaration));
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

  bool Analyzer::analyzeFieldDecl(AnalysisState &State, const frontend::parser::FieldDecl &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
