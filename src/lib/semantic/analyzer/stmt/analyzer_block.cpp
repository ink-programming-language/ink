#include "ink/semantic/analyzer/analyzer.h"

#include "ink/frontend/parser/ast/block_stmt.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/analyzer/block_depth_guard.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/scope_guard.h"

namespace ink::semantic
{
  bool Analyzer::analyzeBlockStmt(AnalyzerState &State, const frontend::parser::BlockStmt &Node)
  {
    auto &Context = State.semanticContext();
    CHECK_RECURSION_DEPTH(State, Node.getSourceRange(), false);
    BlockDepthGuard DepthGuard(State.recursionDepth());
    ScopeGuard Scope(State.nameResolver());
    bool Succeeded = true;
    for (const frontend::parser::Stmt *Stmt : Node.statements())
    {
      const ir::BasicBlock *Block = State.irBuilder().insertBlock();
      if (Block && Block->hasTerminator())
      {
        // TODO: Extend reachability handling when structured control flow and deferred cleanup are lowered.
        // Invalid unreachable source must not append to the already terminated IR block.
        Context.report<core::DiagnosticKind::SemanticUnreachableStatement>(Stmt->getSourceRange());
        Succeeded = false;
        continue;
      }
      if (!analyzeStmt(State, *Stmt))
      {
        Succeeded = false;
      }
    }
    return Succeeded;
  }
} // namespace ink::semantic
