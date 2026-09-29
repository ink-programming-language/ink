#include "analyzer_internal.h"

#include "ink/parser/ast.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <cstdlib>

namespace ink::semantic
{
  namespace
  {
    class BlockDepthGuard final
    {
      public:
        explicit BlockDepthGuard(std::size_t &Depth) noexcept
            : Depth(Depth),
              SavedDepth(Depth)
        {
          ++Depth;
        }

        ~BlockDepthGuard() noexcept
        {
          Depth = SavedDepth;
        }

        BlockDepthGuard(const BlockDepthGuard &) = delete;
        BlockDepthGuard &operator=(const BlockDepthGuard &) = delete;
        BlockDepthGuard(BlockDepthGuard &&) = delete;
        BlockDepthGuard &operator=(BlockDepthGuard &&) = delete;

      private:
        std::size_t &Depth;
        std::size_t SavedDepth;
    };
  } // namespace

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

  bool Analyzer::analyzeSimpleStmt(AnalysisState &State, const parser::SimpleStmt &Node)
  {
    return reportUnsupported(State, Node);
  }

  bool Analyzer::analyzeBlockStmt(AnalysisState &State, const parser::BlockStmt &Node)
  {
    if (State.BlockDepth == 256)
    {
      State.report<core::DiagnosticKind::SemanticNestingLimit>(Node.getSourceRange());
      return false;
    }
    BlockDepthGuard DepthGuard(State.BlockDepth);
    NameResolver::ScopeGuard ScopeGuard(State.Resolver);
    bool Succeeded = true;
    for (const parser::Stmt *Stmt : Node.statements())
    {
      if (!analyzeStmt(State, *Stmt))
      {
        Succeeded = false;
      }
    }
    return Succeeded;
  }

  bool Analyzer::analyzeDeclStmt(AnalysisState &State, const parser::DeclStmt &Node)
  {
    return analyzeDecl(State, *Node.declaration());
  }
} // namespace ink::semantic
