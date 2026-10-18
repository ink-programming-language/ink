#include "ink/semantic/analyzer/analyzer.h"

#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/frontend/parser/ast.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  bool Analyzer::analyzeDecl(AnalyzerState &State, const frontend::parser::Decl &Declaration)
  {
    UNSUPPORT_IF(State, Declaration.isComptime(), false, Declaration.getSourceRange(), frontend::parser::astKindName(Declaration.getKind()));
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
      State.semanticContext().report<core::DiagnosticKind::SemanticInvalidAST>(Declaration.getSourceRange(), "declaration node kind must belong to the declaration AST category");
      return false;
    }
  }

  bool Analyzer::analyzeFieldDecl(AnalyzerState &State, const frontend::parser::FieldDecl &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(State, Node);
  }
} // namespace ink::semantic
