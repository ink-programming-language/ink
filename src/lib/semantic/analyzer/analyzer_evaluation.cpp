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
    const std::size_t Begin = State.Lifetimes.size();
    const ExpressionResult Value = analyzeExpr(State, Node);
    if (!Value)
    {
      return {};
    }
    if (!cleanupObjects(State, Begin, true, Node))
    {
      return {};
    }
    State.Lifetimes.resize(Begin);
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

  Analyzer::ExpressionResult Analyzer::callComptime(AnalysisState &State, const Function &FunctionValue, std::span<const Value *const> Arguments, const parser::ASTNodeBase &Node)
  {
    if (!prepareComptimeFunctions(State, FunctionValue, Node))
    {
      return {};
    }
    auto &Engine = State.Context.comptimeState().Engine;
    std::vector<ExecutionValueRef> ResolvedArguments;
    ResolvedArguments.reserve(Arguments.size());
    for (const Value *Argument : Arguments)
    {
      ExecutionValueResult Resolved = Engine.resolveValue(*Argument, *State.Frame);
      if (!reportExecution(State, Resolved.Status, Node))
      {
        return {};
      }
      ResolvedArguments.push_back(std::move(Resolved.Value));
    }
    const ExecutionValueResult Result = Engine.execute(FunctionValue, ResolvedArguments);
    if (!reportExecution(State, Result.Status, Node))
    {
      return {};
    }
    if (Result.Value.kind() == RuntimeKind::Void)
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
