#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/ast/member_expr.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  const ir::Value *Analyzer::analyzeMemberExpr(AnalyzerState &State, const frontend::parser::MemberExpr &Node, bool IsCallCallee)
  {
    auto &Context = State.semanticContext();
    if (!Node.object())
    {
      Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "MemberExpr requires an object expression");
      return nullptr;
    }
    if (Node.access() != frontend::parser::TokenKind::Dot)
    {
      reportUnsupported(State, Node);
      return nullptr;
    }
    const ir::Value *Object = analyzeExpr(State, *Node.object());
    if (!Object)
    {
      return nullptr;
    }
    if (!ir::Module::classof(Object) && !ir::Type::classof(Object))
    {
      // TODO: Resolve instance members after runtime object access is supported.
      reportUnsupported(State, Node);
      return nullptr;
    }
    const auto Name = Node.member();
    const Binding *Found = State.nameResolver().lookupMember(*Object, core::NamePool::instance().find(Name.Text));
    if (!Found)
    {
      Context.report<core::DiagnosticKind::SemanticUnknownName>(Name.Range, Name.Text);
      return nullptr;
    }
    return analyzeBinding(State, Node, *Found, IsCallCallee);
  }
} // namespace ink::semantic
