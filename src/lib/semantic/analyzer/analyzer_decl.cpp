#include "analyzer_internal.h"

#include "ink/parser/ast.h"

#include <cstdlib>

namespace ink::semantic
{
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

  bool Analyzer::analyzeVarDecl(AnalysisState &State, const parser::VarDecl &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeFieldDecl(AnalysisState &State, const parser::FieldDecl &Node)
  {
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
