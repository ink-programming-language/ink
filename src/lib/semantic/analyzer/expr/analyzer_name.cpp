#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  using namespace ink::ir;

  const Value *Analyzer::resolveVariable(AnalysisState &State, const parser::Expr &Node)
  {
    const parser::Expr *Expression = &Node;
    std::size_t Depth = 0;
    while (parser::ParenExpr::classof(Expression) && Depth++ < State.ExpressionDepthLimit)
    {
      Expression = static_cast<const parser::ParenExpr *>(Expression)->expression();
    }
    if (parser::NameExpr::classof(Expression))
    {
      const auto Name = static_cast<const parser::NameExpr *>(Expression)->name();
      const auto *Binding = State.Resolver.lookup(State.Context.namePool().find(Name.Text));
      if (Binding && Binding->targets().size() == 1)
      {
        const Value *Target = Binding->targets().front();
        if (AllocaInstruction::classof(Target) || FunctionParameter::classof(Target))
        {
          return Target;
        }
      }
    }
    State.report<core::DiagnosticKind::SemanticInvalidAssignment>(Node.getSourceRange());
    return nullptr;
  }

  Analyzer::ExpressionResult Analyzer::analyzeNameExpr(AnalysisState &State, const parser::NameExpr &Node)
  {
    const auto NameToken = Node.name();
    const Name Symbol = State.Context.namePool().find(NameToken.Text);
    const auto *Binding = State.Resolver.lookup(Symbol);
    if (!Binding)
    {
      if (State.Resolver.lookup<Decl *>(Symbol))
      {
        reportUnsupported(State, Node);
        return {};
      }
      if (NameToken.Text == "true" || NameToken.Text == "false")
      {
        return {&State.Context.constantPool().getBoolConstant(NameToken.Text == "true")};
      }
      State.report<core::DiagnosticKind::SemanticUnknownName>(NameToken.Range, NameToken.Text);
      return {};
    }
    if (Binding->targets().size() != 1)
    {
      State.report<core::DiagnosticKind::SemanticAmbiguousName>(Node.getSourceRange());
      return {};
    }
    const Value *Result = Binding->targets().front();
    auto &Execution = State.Context.comptimeState();
    if (!State.Evaluating && Function::classof(Result))
    {
      const auto Definition = Execution.Functions.find(Result);
      if (Definition != Execution.Functions.end() && Definition->second.Comptime)
      {
        State.report<core::DiagnosticKind::SemanticComptimeFunctionAtRuntime>(Node.getSourceRange());
        return {};
      }
    }
    if (State.Evaluating && FunctionParameter::classof(Result))
    {
      if (!State.ExecutingFunction)
      {
        reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
        return {};
      }
      const auto Place = Execution.Engine.lookup(*State.Frame, Result);
      if (!reportExecution(State, Place.Status, Node))
      {
        return {};
      }
      const auto Loaded = Execution.Engine.load(Place.Place);
      return reportExecution(State, Loaded.Status, Node) ? ExpressionResult{Loaded.Value} : ExpressionResult{};
    }
    if (const auto Variable = Execution.Variables.find(Result); AllocaInstruction::classof(Result) && Variable != Execution.Variables.end())
    {
      if (Variable->second.Comptime || State.Evaluating)
      {
        if (!Variable->second.Comptime)
        {
          reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
          return {};
        }
        const auto Place = Execution.Engine.lookup(*State.Frame, Result);
        if (!reportExecution(State, Place.Status, Node))
        {
          return {};
        }
        const auto Loaded = Execution.Engine.load(Place.Place);
        return reportExecution(State, Loaded.Status, Node) ? ExpressionResult{Loaded.Value} : ExpressionResult{};
      }
      if (Variable->second.Function != State.CurrentFunction)
      {
        State.report<core::DiagnosticKind::SemanticInvalidCapture>(Node.getSourceRange(), NameToken.Text);
        return {};
      }
      if (!Variable->second.Initialized)
      {
        State.report<core::DiagnosticKind::SemanticUninitializedRead>(Node.getSourceRange(), NameToken.Text);
        return {};
      }
      return {State.Builder.createLoadInstruction(*Result)};
    }
    if (FunctionParameter::classof(Result) && &static_cast<const FunctionParameter &>(*Result).function() != State.CurrentFunction)
    {
      State.report<core::DiagnosticKind::SemanticInvalidCapture>(Node.getSourceRange(), NameToken.Text);
      return {};
    }
    return {Result};
  }
} // namespace ink::semantic
