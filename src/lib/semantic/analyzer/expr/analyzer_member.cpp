#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/ast/member_expr.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"

namespace ink::semantic
{
  ExprResult Analyzer::analyzeMemberExpr(AnalyzerState &State, const frontend::parser::MemberExpr &Node, bool IsCallCallee)
  {
    auto &Context = State.semanticContext();
    RETURN_REPORT_IF_FALSE(Context, Node.object(), core::DiagnosticKind::SemanticInvalidAST, ExprResult{}, Node.getSourceRange(), "MemberExpr requires an object expression");
    UNSUPPORT_IF(State, Node.access() != frontend::parser::TokenKind::Dot, ExprResult{}, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
    const ExprResult Object = analyzeExpr(State, *Node.object());
    RETURN_IF(!Object, ExprResult{});
    // TODO: Resolve instance members after runtime object access is supported.
    UNSUPPORT_IF(State, Object.Kind != ExprResultKind::Module && Object.Kind != ExprResultKind::Type, ExprResult{}, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
    const auto Name = Node.member();
    const Binding *Found = State.nameResolver().lookupMember(*Object.Value, core::NamePool::instance().find(Name.Text));
    RETURN_REPORT_IF_FALSE(Context, Found, core::DiagnosticKind::SemanticUnknownName, ExprResult{}, Name.Range, Name.Text);
    return analyzeBinding(State, Node, *Found, IsCallCallee);
  }
} // namespace ink::semantic
