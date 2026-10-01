#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  using namespace ink::ir;
  using namespace execution;

  Analyzer::ExpressionResult Analyzer::analyzeUpdateExpr(AnalysisState &State, const parser::Expr &Operand, tokenizer::TokenKind Operator, bool Postfix, const parser::Expr &Node, std::size_t Depth)
  {
    if (!State.Evaluating)
    {
      reportUnsupported(State, Node);
      return {};
    }
    const Value *Binding = resolveVariable(State, Operand);
    if (!Binding)
    {
      return {};
    }
    const auto Variable = State.Context.comptimeState().Variables.find(Binding);
    if ((Variable != State.Context.comptimeState().Variables.end() && !Variable->second.Comptime) || (FunctionParameter::classof(Binding) && !State.ExecutingFunction))
    {
      reportExecution(State, ExecutionStatus::RuntimeValue, Node);
      return {};
    }
    auto &Engine = State.Context.comptimeState().Engine;
    const auto Place = Engine.lookup(*State.Frame, Binding);
    if (!reportExecution(State, Place.Status, Node))
    {
      return {};
    }
    const ExecutionResult Old = Engine.load(Place.Place);
    if (!reportExecution(State, Old.Status, Node))
    {
      return {};
    }
    if (!Old.Value || !IntegerConstant::classof(Old.Value))
    {
      reportExecution(State, ExecutionStatus::TypeMismatch, Node);
      return {};
    }
    const auto &Type = static_cast<const IntegerType &>(Old.Value->type());
    const Constant *One = State.Context.constantPool().getIntegerConstant(Type, IntegerBits(Type.bitWidth(), 1));
    const ExecutionResult Updated = Engine.evaluateBinary(Operator == tokenizer::TokenKind::PlusPlus ? tokenizer::TokenKind::Plus : tokenizer::TokenKind::Minus, *Old.Value, *One);
    if (!reportExecution(State, Updated.Status, Node) || !reportExecution(State, Engine.store(Place.Place, *Updated.Value), Node))
    {
      return {};
    }
    (void)Depth;
    return {Postfix ? Old.Value : Updated.Value};
  }
} // namespace ink::semantic
