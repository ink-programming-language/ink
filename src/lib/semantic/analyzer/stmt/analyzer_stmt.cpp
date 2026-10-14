#include "ink/semantic/analyzer/analyzer.h"

#include "ink/semantic/context.h"
#include "ink/frontend/parser/ast.h"

#include <cstddef>
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

  bool Analyzer::analyzeStmt(SemanticContext &Context, const frontend::parser::Stmt &Stmt)
  {
    // Compile-time evaluation will be restored with the semantic implementation.
    if (Stmt.isComptime())
    {
      return reportUnsupported(Context, Stmt);
    }
    switch (Stmt.getKind())
    {
#define INK_ANALYZE_Root(Name)
#define INK_ANALYZE_Expr(Name)
#define INK_ANALYZE_Stmt(Name) \
  case frontend::parser::ASTKind::Name: \
    return analyze##Name(Context, static_cast<const frontend::parser::Name &>(Stmt));
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

  bool Analyzer::analyzeSimpleStmt(SemanticContext &Context, const frontend::parser::SimpleStmt &Node)
  {
    // TODO: Rebuild semantic analysis for this node.
    return reportUnsupported(Context, Node);
  }

  bool Analyzer::analyzeBlockStmt(SemanticContext &Context, const frontend::parser::BlockStmt &Node)
  {
    if (Context.BlockDepth >= Context.BlockDepthLimit)
    {
      Context.report<core::DiagnosticKind::SemanticNestingLimit>(Node.getSourceRange());
      return false;
    }
    BlockDepthGuard DepthGuard(Context.BlockDepth);
    bool Succeeded = true;
    for (const frontend::parser::Stmt *Stmt : Node.statements())
    {
      if (!analyzeStmt(Context, *Stmt))
      {
        Succeeded = false;
      }
    }
    return Succeeded;
  }

  bool Analyzer::analyzeDeclStmt(SemanticContext &Context, const frontend::parser::DeclStmt &Node)
  {
    return analyzeDecl(Context, *Node.declaration());
  }
} // namespace ink::semantic
