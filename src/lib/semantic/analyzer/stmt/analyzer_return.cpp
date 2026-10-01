#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  using namespace ink::ir;

  bool Analyzer::analyzeReturnStmt(AnalysisState &State, const parser::ReturnStmt &Node)
  {
    if (!State.CurrentFunction)
    {
      State.report<core::DiagnosticKind::SemanticReturnOutsideFunction>(Node.getSourceRange());
      return false;
    }
    if (State.Evaluating && !State.ExecutingFunction)
    {
      State.report<core::DiagnosticKind::SemanticComptimeReturnAcrossRuntimeBoundary>(Node.getSourceRange());
      return false;
    }
    State.Terminated = true;
    const Type &ReturnType = State.CurrentFunction->functionType().returnType();
    const Value *ReturnedValue = nullptr;
    if (Node.value())
    {
      AnalysisState::EvaluationGuard Expected(State, State.Evaluating, &ReturnType);
      const ExpressionResult Result = analyzeExpr(State, *Node.value());
      if (!Result)
      {
        return false;
      }
      if (ReturnType.typeKind() == TypeKind::Void)
      {
        State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.value()->getSourceRange(), "no return value", Result.IntegerLiteral ? "integer literal" : (Result.ValueObject ? describeType(Result.ValueObject->type()) : "void"));
        return false;
      }
      ReturnedValue = convertExpression(State, Result, ReturnType, *Node.value());
      if (!ReturnedValue)
      {
        return false;
      }
    }
    else if (ReturnType.typeKind() != TypeKind::Void)
    {
      State.report<core::DiagnosticKind::SemanticMissingReturn>(Node.getSourceRange(), State.Context.namePool().text(State.CurrentFunction->name()));
      return false;
    }
    if (State.Evaluating)
    {
      if (ReturnedValue && !Constant::classof(ReturnedValue))
      {
        return reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
      }
      State.ReturnedValue = static_cast<const Constant *>(ReturnedValue);
      return true;
    }
    if (!State.Builder.createReturnInstruction(ReturnedValue))
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    return true;
  }
} // namespace ink::semantic
