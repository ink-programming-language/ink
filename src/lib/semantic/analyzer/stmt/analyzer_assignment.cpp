#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  using namespace ir;
  using tokenizer::TokenKind;

  namespace
  {
    TokenKind assignmentOperation(TokenKind Operator)
    {
      switch (Operator)
      {
      case TokenKind::PlusAssign:
        return TokenKind::Plus;
      case TokenKind::MinusAssign:
        return TokenKind::Minus;
      case TokenKind::StarAssign:
        return TokenKind::Star;
      case TokenKind::SlashAssign:
        return TokenKind::Slash;
      case TokenKind::PercentAssign:
        return TokenKind::Percent;
      case TokenKind::AmpAssign:
        return TokenKind::Amp;
      case TokenKind::PipeAssign:
        return TokenKind::Pipe;
      case TokenKind::CaretAssign:
        return TokenKind::Caret;
      case TokenKind::ShiftLeftAssign:
        return TokenKind::ShiftLeft;
      case TokenKind::ShiftRightAssign:
        return TokenKind::ShiftRight;
      default:
        return Operator;
      }
    }
  } // namespace

  Analyzer::ExpressionResult Analyzer::analyzeSimpleItem(AnalysisState &State, const parser::SimpleItem &Node, std::size_t Depth)
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
    if (parser::ExprItem::classof(&Node))
    {
      const auto &Expression = *static_cast<const parser::ExprItem &>(Node).expression();
      ExpressionResult Result = analyzeExpr(State, Expression, Depth);
      if (Result.IntegerLiteral)
      {
        const Type *Target = State.ExpectedType ? State.ExpectedType : State.Context.typePool().getType<TypeKind::Integer>(32, true);
        return {convertExpression(State, Result, *Target, Expression)};
      }
      return Result;
    }
    const auto &Assignment = static_cast<const parser::AssignmentItem &>(Node);
    const parser::Expr *Destination = Assignment.left();
    while (parser::ParenExpr::classof(Destination))
    {
      Destination = static_cast<const parser::ParenExpr &>(*Destination).expression();
    }
    if (State.Evaluating && (parser::IndexExpr::classof(Destination) || parser::MemberExpr::classof(Destination)))
    {
      return assignComptimeArray(State, Assignment, Depth + 1);
    }
    if (!State.Evaluating && !State.CurrentFunction)
    {
      reportUnsupported(State, Node);
      return {};
    }
    const Value *Address = State.Evaluating ? resolveVariable(State, *Assignment.left()) : resolveAddress(State, *Assignment.left(), Depth + 1, false);
    if (!Address)
    {
      return {};
    }
    auto &Execution = State.Context.comptimeState();
    const auto Found = Execution.Variables.find(Address);
    const bool HasVariable = Found != Execution.Variables.end();
    if (State.Evaluating && ((HasVariable && !Found->second.Comptime) || FunctionParameter::classof(Address)))
    {
      reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
      return {};
    }
    if (HasVariable && (Found->second.Constant || (!State.Evaluating && (Found->second.Comptime || Found->second.Function != State.CurrentFunction))))
    {
      State.report<core::DiagnosticKind::SemanticInvalidAssignment>(Node.getSourceRange());
      return {};
    }
    const Type &Target = AllocaInstruction::classof(Address) ? static_cast<const AllocaInstruction &>(*Address).allocatedType() : (!State.Evaluating ? static_cast<const PointerType &>(Address->type()).pointeeType() : Address->type());
    std::optional<std::size_t> ConstructorField;
    if (State.Constructing && FieldPointerInstruction::classof(Address) && &static_cast<const FieldPointerInstruction *>(Address)->address() == State.CurrentFunction->parameters().front().get())
    {
      ConstructorField = static_cast<const FieldPointerInstruction *>(Address)->fieldIndex();
    }
    if (ClassType::classof(&Target) && Assignment.op() != TokenKind::Assign)
    {
      State.report<core::DiagnosticKind::SemanticInvalidMember>(Node.getSourceRange(), "the in-place operator protocol is not enabled");
      return {};
    }
    AnalysisState::EvaluationGuard Expected(State, State.Evaluating, &Target);
    const ExpressionResult Right = analyzeSimpleItem(State, *Assignment.right(), Depth + 1);
    if (!Right || !Right.ValueObject || &Right.ValueObject->type() != &Target)
    {
      if (Right)
      {
        State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), describeType(Target), Right.ValueObject ? describeType(Right.ValueObject->type()) : "void");
      }
      return {};
    }
    const Value *Stored = Right.ValueObject;
    if ((!ConstructorField || State.ConstructorFields[*ConstructorField]) && !destroyPrevious(State, *Address, Target, Node))
    {
      return {};
    }
    takeTemporary(State, Right.TemporaryAddress);
    if (State.Evaluating)
    {
      const auto Place = Execution.Engine.lookup(*State.Frame, Address);
      if (!reportExecution(State, Place.Status, Node))
      {
        return {};
      }
      if (!Constant::classof(Stored))
      {
        reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
        return {};
      }
      if (Assignment.op() != TokenKind::Assign)
      {
        const auto Previous = Execution.Engine.load(Place.Place);
        if (!reportExecution(State, Previous.Status, Node))
        {
          return {};
        }
        const auto Result = Execution.Engine.evaluateBinary(assignmentOperation(Assignment.op()), *Previous.Value, static_cast<const Constant &>(*Stored));
        if (!reportExecution(State, Result.Status, Node))
        {
          return {};
        }
        Stored = Result.Value;
      }
      if (!reportExecution(State, Execution.Engine.store(Place.Place, static_cast<const Constant &>(*Stored)), Node))
      {
        return {};
      }
    }
    else
    {
      if (Assignment.op() != TokenKind::Assign)
      {
        reportUnsupported(State, Node);
        return {};
      }
      if (!State.Builder.createStoreInstruction(*Address, *Stored))
      {
        State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
        return {};
      }
    }
    if (HasVariable)
    {
      Execution.Variables.find(Address)->second.Initialized = true;
    }
    if (ConstructorField)
    {
      State.ConstructorFields[*ConstructorField] = true;
    }
    for (auto &Lifetime : State.Lifetimes)
    {
      if (Lifetime.Address == Address && !Lifetime.TemporaryValue && Lifetime.Comptime == State.Evaluating)
      {
        Lifetime.Active = true;
        if (!State.Evaluating && !State.Builder.createStoreInstruction(*Lifetime.Initialized, State.Context.constantPool().getBoolConstant(true)))
        {
          return {};
        }
      }
    }
    return {Stored};
  }
} // namespace ink::semantic
