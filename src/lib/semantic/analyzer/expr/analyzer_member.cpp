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
    const auto Site = State.at(Node);
    CHECK_ICE(Site, Node.object(), core::DiagnosticKind::SemanticInvalidAST, "MemberExpr requires an object expression");
    CHECK_ICE(Site, Node.access() == frontend::parser::TokenKind::Dot, core::DiagnosticKind::SemanticUnsupported, frontend::parser::astKindName(Node.getKind()));
    const ExprResult Result = analyzeExpr(State, *Node.object());
    const ir::Value *Object = Result.value();
    RETURN_IF_NOT(Object, nullptr);
    // TODO: Resolve instance members after runtime object access is supported.
    CHECK_ICE(Site, !Result.isLocation() && (ir::Module::classof(Object) || ir::Type::classof(Object)), core::DiagnosticKind::SemanticUnsupported, frontend::parser::astKindName(Node.getKind()));
    const auto Name = Node.member();
    const Binding *Found = State.nameResolver().lookupMember(*Object, core::NamePool::instance().find(Name.Text));
    RETURN_ERROR_IF_NOT(State.at(Name.Range), Found, core::DiagnosticKind::SemanticUnknownName, nullptr, Name.Text);
    return analyzeBinding(State, Site, *Found, IsCallCallee);
  }
} // namespace ink::semantic
