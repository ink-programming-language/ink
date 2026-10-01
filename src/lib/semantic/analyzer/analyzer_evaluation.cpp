#include "analyzer_internal.h"

#include "ink/execution/support/execution_diagnostic.h"
#include "ink/parser/ast.h"

#include <utility>
#include <vector>

namespace ink::semantic
{
  using namespace ink::ir;
  using namespace execution;

  bool Analyzer::reportExecution(AnalysisState &State, ExecutionStatus Status, const parser::ASTNodeBase &Node)
  {
    if (Status == ExecutionStatus::Success)
    {
      return true;
    }
    if (const auto Diagnostic = makeExecutionDiagnostic(Status, State.Source, Node.getSourceRange(), "compile-time execution failed"))
    {
      State.Context.compilationContext().diagnosticEngine().report(*Diagnostic);
    }
    return false;
  }

  Analyzer::ExpressionResult Analyzer::evaluateComptime(AnalysisState &State, const parser::Expr &Node)
  {
    AnalysisState::EvaluationGuard Guard(State, true, State.ExpectedType);
    const ExpressionResult Value = analyzeExpr(State, Node);
    if (!Value)
    {
      return {};
    }
    if (Value.Void)
    {
      return {nullptr, nullptr, false, true};
    }
    if (!Value.ValueObject || !Constant::classof(Value.ValueObject))
    {
      reportExecution(State, ExecutionStatus::RuntimeValue, Node);
      return {};
    }
    return {Value.ValueObject};
  }

  Analyzer::ExpressionResult Analyzer::callComptime(AnalysisState &State, const Function &FunctionValue, std::span<const Value *const> Arguments, const parser::Expr &Node)
  {
    auto &Engine = State.Context.comptimeState().Engine;
    std::vector<ExecutionValueRef> EvaluatedArguments;
    EvaluatedArguments.reserve(Arguments.size());
    for (const Value *Argument : Arguments)
    {
      ExecutionValueResult Evaluated = Engine.evaluate(*Argument, *State.Frame);
      if (!reportExecution(State, Evaluated.Status, Node))
      {
        return {};
      }
      EvaluatedArguments.push_back(std::move(Evaluated.Value));
    }
    const ExecutionValueResult Result = Engine.execute(FunctionValue, EvaluatedArguments);
    if (!reportExecution(State, Result.Status, Node))
    {
      return {};
    }
    if (Result.Value.kind() == ExecutionValueKind::Void)
    {
      return {nullptr, nullptr, false, true};
    }
    const Constant *Value = Result.Value.toConstant(State.Context.irContext());
    if (!Value)
    {
      reportExecution(State, ExecutionStatus::UnsupportedOperation, Node);
      return {};
    }
    return {Value};
  }
} // namespace ink::semantic
