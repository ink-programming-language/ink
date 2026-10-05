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
      const Value *Address = resolveAddress(State, Operand, Depth + 1);
      if (!Address)
      {
        return {};
      }
      const auto &Type = static_cast<const PointerType &>(Address->type()).pointeeType();
      if (!IntegerType::classof(&Type))
      {
        State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "integer update operand", describeType(Type));
        return {};
      }
      const auto &Integer = static_cast<const IntegerType &>(Type);
      ExecutionInteger Delta(Integer.bitWidth(), 1);
      if (Operator == tokenizer::TokenKind::MinusMinus)
      {
        Delta.negate(Delta);
      }
      const auto *Amount = State.Context.constantPool().getIntegerConstant(Integer, Delta.bits());
      const auto *Old = State.Builder.createLoadInstruction(*Address);
      const auto *Updated = Old ? State.Builder.createAddInstruction(*Old, *Amount) : nullptr;
      if (!Updated || !State.Builder.createStoreInstruction(*Address, *Updated))
      {
        State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
        return {};
      }
      return {Postfix ? static_cast<const Value *>(Old) : Updated};
    }
    const Value *Binding = resolveVariable(State, Operand);
    if (!Binding)
    {
      return {};
    }
    const auto Variable = State.Context.comptimeState().Variables.find(Binding);
    if ((Variable != State.Context.comptimeState().Variables.end() && !Variable->second.Comptime) || FunctionParameter::classof(Binding))
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
