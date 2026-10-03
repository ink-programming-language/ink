#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  Analyzer::ExpressionResult Analyzer::analyzeExpr(AnalysisState &State, const parser::Expr &Node, std::size_t Depth)
  {
    AnalysisState::TraversalGuard Traversal(State);
    if (!reportExecution(State, Traversal.status(), Node))
    {
      return {};
    }
    if (Depth >= State.ExpressionDepthLimit)
    {
      State.report<core::DiagnosticKind::SemanticNestingLimit>(Node.getSourceRange());
      return {};
    }
    if (Node.isComptime() && !State.Evaluating)
    {
      return evaluateComptime(State, Node);
    }
    if (parser::ParenExpr::classof(&Node))
    {
      return analyzeParenExpr(State, static_cast<const parser::ParenExpr &>(Node), Depth);
    }
    if (parser::LiteralExpr::classof(&Node))
    {
      return analyzeLiteralExpr(State, static_cast<const parser::LiteralExpr &>(Node));
    }
    if (parser::NameExpr::classof(&Node))
    {
      return analyzeNameExpr(State, static_cast<const parser::NameExpr &>(Node));
    }
    if (parser::MemberExpr::classof(&Node))
    {
      return analyzeMemberExpr(State, static_cast<const parser::MemberExpr &>(Node), Depth);
    }
    if (parser::UnaryExpr::classof(&Node))
    {
      return analyzeUnaryExpr(State, static_cast<const parser::UnaryExpr &>(Node), Depth);
    }
    if (parser::CallExpr::classof(&Node))
    {
      return analyzeCallExpr(State, static_cast<const parser::CallExpr &>(Node), Depth);
    }
    if (parser::BinaryExpr::classof(&Node))
    {
      return analyzeBinaryExpr(State, static_cast<const parser::BinaryExpr &>(Node), Depth);
    }
    if (parser::PostfixUpdateExpr::classof(&Node))
    {
      const auto &Update = static_cast<const parser::PostfixUpdateExpr &>(Node);
      return analyzeUpdateExpr(State, *Update.operand(), Update.op(), true, Node, Depth);
    }
    if (State.Evaluating && parser::ConditionalExpr::classof(&Node))
    {
      const auto &Conditional = static_cast<const parser::ConditionalExpr &>(Node);
      const ExpressionResult Condition = analyzeExpr(State, *Conditional.condition(), Depth + 1);
      if (!Condition || !Condition.ValueObject || !ir::BoolConstant::classof(Condition.ValueObject))
      {
        if (Condition)
        {
          reportExecution(State, execution::ExecutionStatus::TypeMismatch, Node);
        }
        return {};
      }
      return analyzeExpr(State, static_cast<const ir::BoolConstant &>(*Condition.ValueObject).value() ? *Conditional.then() : *Conditional.elseValue(), Depth + 1);
    }
    reportUnsupported(State, Node);
    return {};
  }
} // namespace ink::semantic
