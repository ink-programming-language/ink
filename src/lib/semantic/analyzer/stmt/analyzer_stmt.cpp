#include "ink/semantic/analyzer/analyzer.h"

#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/frontend/parser/ast.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  bool Analyzer::analyzeStmt(AnalyzerState &State, const frontend::parser::Stmt &Stmt)
  {
    // TODO: Restore compile-time statement evaluation.
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
      State.semanticContext().report<core::DiagnosticKind::SemanticInvalidAST>(Stmt.getSourceRange(), "statement node kind must belong to the statement AST category");
      return false;
    }
  }

  bool Analyzer::analyzeSimpleStmt(AnalyzerState &State, const frontend::parser::SimpleStmt &Node)
  {
    if (State.AnalysisMode == AnalyzerState::Mode::CollectDecls)
    {
      return true;
    }
    bool Succeeded = true;
    for (const frontend::parser::SimpleItem *Item : Node.items())
    {
      if (!frontend::parser::ExprItem::classof(Item))
      {
        // TODO: Restore assignments and the remaining simple items.
        return reportUnsupported(State, Node);
      }
      const auto &Expression = *static_cast<const frontend::parser::ExprItem *>(Item)->expression();
      if (!requireRuntimeValue(State.Context, Expression.getSourceRange(), analyzeExpr(State, Expression)))
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
