#include "analyzer_internal.h"

#include "ink/execution/support/execution_diagnostic.h"
#include "ink/parser/ast.h"

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
    auto &Engine = State.Context.comptimeState().Engine;
    bool Entered = false;
    const ExecutionResult Result = Engine.executeOnce(*State.Frame, &Node, [&]() -> ExecutionResult
    {
      Entered = true;
      AnalysisState::EvaluationGuard Guard(State, true, State.ExpectedType);
      const ExpressionResult Value = analyzeExpr(State, Node);
      if (!Value)
      {
        return {ExecutionStatus::UnsupportedOperation, nullptr};
      }
      if (Value.Void)
      {
        return {};
      }
      if (!Value.ValueObject || !Constant::classof(Value.ValueObject))
      {
        reportExecution(State, ExecutionStatus::RuntimeValue, Node);
        return {ExecutionStatus::RuntimeValue, nullptr};
      }
      return {ExecutionStatus::Success, static_cast<const Constant *>(Value.ValueObject)};
    });
    if (!Result)
    {
      if (!Entered)
      {
        reportExecution(State, Result.Status, Node);
      }
      return {};
    }
    return {Result.Value, nullptr, false, !Result.Value};
  }

  Analyzer::ExpressionResult Analyzer::callComptime(AnalysisState &State, const Function &FunctionValue, std::span<const Value *const> Arguments, const parser::Expr &Node)
  {
    auto &Comptime = State.Context.comptimeState();
    const auto Found = Comptime.Functions.find(&FunctionValue);
    const bool External = FunctionValue.languageLinkage() == LanguageLinkage::C;
    if (!External && (Found == Comptime.Functions.end() || !Found->second.AST->body()))
    {
      reportExecution(State, ExecutionStatus::MissingBody, Node);
      return {};
    }
    if (External)
    {
      ExecutionFrame &DefinitionFrame = Found == Comptime.Functions.end() ? *State.Frame : *Found->second.DefinitionFrame;
      const ExecutionResult Result = Comptime.Engine.call(FunctionValue, DefinitionFrame, Arguments, {});
      if (!reportExecution(State, Result.Status, Node))
      {
        return {};
      }
      return {Result.Value, nullptr, false, FunctionValue.functionType().returnType().typeKind() == TypeKind::Void};
    }
    const auto Definition = Found->second;
    bool Reported = false;
    const ExecutionResult Result = Comptime.Engine.call(FunctionValue, *Definition.DefinitionFrame, Arguments, [&](ExecutionFrame &CallFrame) -> ExecutionResult
    {
      AnalysisState CallState(State.Context, *Definition.DefinitionScope, *Definition.Input);
      CallState.Frame = &CallFrame;
      NameResolver::ScopeGuard Scope(CallState.Resolver);
      CallState.Evaluating = true;
      CallState.ExecutingFunction = true;
      CallState.CurrentFunction = const_cast<Function *>(&FunctionValue);
      for (const auto &Parameter : FunctionValue.parameters())
      {
        if (CallState.Resolver.bind(Parameter->name(), *Parameter) != NameResolver::BindResult::Inserted)
        {
          reportExecution(State, ExecutionStatus::InvalidBinding, Node);
          Reported = true;
          return {ExecutionStatus::InvalidBinding};
        }
      }
      if (!analyzeStmt(CallState, *Definition.AST->body()))
      {
        Reported = true;
        return {ExecutionStatus::UnsupportedOperation};
      }
      if (!CallState.Terminated && FunctionValue.functionType().returnType().typeKind() != TypeKind::Void)
      {
        State.report<core::DiagnosticKind::SemanticMissingReturn>(Node.getSourceRange(), State.Context.namePool().text(FunctionValue.name()));
        Reported = true;
        return {ExecutionStatus::TypeMismatch};
      }
      return {ExecutionStatus::Success, CallState.ReturnedValue};
    });
    if (!Result)
    {
      if (!Reported)
      {
        reportExecution(State, Result.Status, Node);
      }
      return {};
    }
    return {Result.Value, nullptr, false, FunctionValue.functionType().returnType().typeKind() == TypeKind::Void};
  }
} // namespace ink::semantic
