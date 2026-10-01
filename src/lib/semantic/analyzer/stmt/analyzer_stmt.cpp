#include "../analyzer_internal.h"

#include "ink/core/config_manager.h"
#include "ink/parser/ast.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <cstdlib>

namespace ink::semantic
{
  using namespace ink::ir;

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
    AnalysisState::TraversalGuard Traversal(State);
    if (!reportExecution(State, Traversal.status(), Stmt))
    {
      return false;
    }
    if (State.Terminated)
    {
      State.report<core::DiagnosticKind::SemanticUnreachableStatement>(Stmt.getSourceRange());
      return false;
    }
    if (Stmt.isComptime() && !State.Evaluating && parser::BlockStmt::classof(&Stmt))
    {
      bool Entered = false;
      bool ReusedResult = false;
      const auto Result = State.Context.comptimeState().Engine.executeOnce(*State.Frame, &Stmt, [&]() -> execution::ExecutionResult
      {
        Entered = true;
        AnalysisState::EvaluationGuard Guard(State);
        return {analyzeBlockStmt(State, static_cast<const parser::BlockStmt &>(Stmt)) ? execution::ExecutionStatus::Success : execution::ExecutionStatus::UnsupportedOperation, nullptr};
      }, &ReusedResult);
      if (!Result && !Entered && !ReusedResult)
      {
        reportExecution(State, Result.Status, Stmt);
      }
      return static_cast<bool>(Result);
    }
    if (Stmt.isComptime() && !State.Evaluating && !parser::DeclStmt::classof(&Stmt) && !parser::IfStmt::classof(&Stmt) && !parser::WhileStmt::classof(&Stmt) && !parser::ClassicForStmt::classof(&Stmt))
    {
      return reportUnsupported(State, Stmt);
    }
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
    bool Succeeded = true;
    for (const parser::SimpleItem *Item : Node.items())
    {
      const ExpressionResult Result = analyzeSimpleItem(State, *Item);
      if (!Result)
      {
        Succeeded = false;
        if (State.Evaluating)
        {
          break;
        }
      }
    }
    return Succeeded;
  }

  bool Analyzer::analyzeBlockStmt(AnalysisState &State, const parser::BlockStmt &Node)
  {
    const auto BlockDepthLimit = core::ConfigManager::getSize<core::ConfigKind::SemanticBlockDepthLimit>();
    if (State.BlockDepth >= BlockDepthLimit)
    {
      State.report<core::DiagnosticKind::SemanticNestingLimit>(Node.getSourceRange());
      return false;
    }
    BlockDepthGuard DepthGuard(State.BlockDepth);
    NameResolver::ScopeGuard ScopeGuard(State.Resolver);
    AnalysisState::FrameGuard Frame(State, execution::ExecutionFrameKind::Block);
    if (!Frame)
    {
      return reportExecution(State, State.Context.comptimeState().Engine.lastStatus(), Node);
    }
    bool Succeeded = true;
    for (const parser::Stmt *Stmt : Node.statements())
    {
      if (!analyzeStmt(State, *Stmt))
      {
        Succeeded = false;
      }
      if (State.Breaking || State.Continuing || (State.Evaluating && (!Succeeded || State.Terminated)))
      {
        break;
      }
    }
    return Succeeded;
  }

  bool Analyzer::analyzeDeclStmt(AnalysisState &State, const parser::DeclStmt &Node)
  {
    return analyzeDecl(State, *Node.declaration());
  }
} // namespace ink::semantic
