#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  bool Analyzer::analyzeIfStmt(AnalysisState &State, const parser::IfStmt &Node)
  {
    if (!State.Evaluating && !Node.isComptime())
    {
      return reportUnsupported(State, Node);
    }
    const ExpressionResult Condition = State.Evaluating ? analyzeExpr(State, *Node.condition()) : evaluateComptime(State, *Node.condition());
    if (!Condition)
    {
      return false;
    }
    if (!Condition.ValueObject || !ir::BoolConstant::classof(Condition.ValueObject))
    {
      return reportExecution(State, execution::ExecutionStatus::TypeMismatch, Node);
    }
    const parser::Stmt *Selected = static_cast<const ir::BoolConstant &>(*Condition.ValueObject).value() ? Node.thenBranch() : Node.elseBranch();
    return !Selected || analyzeStmt(State, *Selected);
  }
} // namespace ink::semantic
