#include "../analyzer_internal.h"

#include "ink/parser/ast.h"
#include "ink/ir/constant/class_constant.h"

#include <algorithm>
#include <vector>

namespace ink::semantic
{
  using namespace ir;
  using tokenizer::TokenKind;

  namespace
  {
    TokenKind arrayAssignmentOperation(TokenKind Operator)
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

  Analyzer::ExpressionResult Analyzer::assignComptimeArray(AnalysisState &State, const parser::AssignmentItem &Node, std::size_t Depth)
  {
    std::vector<const parser::Expr *> Indices;
    const parser::Expr *Root = Node.left();
    while (parser::ParenExpr::classof(Root) || parser::IndexExpr::classof(Root) || parser::MemberExpr::classof(Root))
    {
      if (++Depth >= State.ExpressionDepthLimit)
      {
        State.report<core::DiagnosticKind::SemanticNestingLimit>(Node.getSourceRange());
        return {};
      }
      if (parser::ParenExpr::classof(Root))
      {
        Root = static_cast<const parser::ParenExpr &>(*Root).expression();
      }
      else if (parser::MemberExpr::classof(Root))
      {
        const auto &Member = static_cast<const parser::MemberExpr &>(*Root);
        if (Member.access() != TokenKind::Dot)
        {
          State.report<core::DiagnosticKind::SemanticInvalidMember>(Member.getSourceRange(), "compile-time bindings use dot field access");
          return {};
        }
        Indices.push_back(&Member);
        Root = Member.object();
      }
      else
      {
        const auto &Index = static_cast<const parser::IndexExpr &>(*Root);
        if (Index.optional())
        {
          State.report<core::DiagnosticKind::SemanticInvalidAssignment>(Node.getSourceRange());
          return {};
        }
        Indices.push_back(&Index);
        Root = Index.object();
      }
    }
    const Value *Binding = resolveVariable(State, *Root);
    if (!Binding)
    {
      return {};
    }
    auto &Execution = State.Context.comptimeState();
    const auto Found = Execution.Variables.find(Binding);
    if (Found == Execution.Variables.end() || !Found->second.Comptime)
    {
      reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
      return {};
    }
    if (Found->second.Constant)
    {
      State.report<core::DiagnosticKind::SemanticInvalidAssignment>(Node.getSourceRange());
      return {};
    }
    const auto Place = Execution.Engine.lookup(*State.Frame, Binding);
    if (!reportExecution(State, Place.Status, Node))
    {
      return {};
    }
    const auto Initial = Execution.Engine.load(Place.Place);
    if (!reportExecution(State, Initial.Status, Node))
    {
      return {};
    }
    const Type *ElementType = &Initial.Value->type();
    std::vector<std::size_t> Path;
    std::reverse(Indices.begin(), Indices.end());
    for (const parser::Expr *Projection : Indices)
    {
      if (parser::MemberExpr::classof(Projection))
      {
        const auto &Member = static_cast<const parser::MemberExpr &>(*Projection);
        if (!ClassType::classof(ElementType))
        {
          State.report<core::DiagnosticKind::SemanticInvalidMember>(Member.getSourceRange(), "field projection requires a class value");
          return {};
        }
        const auto &Class = static_cast<const ClassType &>(*ElementType);
        const auto Index = lookupClassField(State, Class, Member);
        if (!Index)
        {
          return {};
        }
        Path.push_back(*Index);
        ElementType = Class.fields()[*Index].FieldType;
        continue;
      }
      const auto *Index = static_cast<const parser::IndexExpr *>(Projection);
      if (!ArrayType::classof(ElementType))
      {
        State.report<core::DiagnosticKind::SemanticTypeMismatch>(Index->object()->getSourceRange(), "array", describeType(*ElementType));
        return {};
      }
      const auto &ArrayType = static_cast<const ir::ArrayType &>(*ElementType);
      const Value *Offset = analyzeArrayIndex(State, *Index->index(), ArrayType.elementCount(), Depth);
      if (!Offset)
      {
        return {};
      }
      if (!IntegerConstant::classof(Offset))
      {
        reportExecution(State, execution::ExecutionStatus::RuntimeValue, *Index);
        return {};
      }
      Path.push_back(static_cast<std::size_t>(static_cast<const IntegerConstant &>(*Offset).value().words().front()));
      ElementType = &ArrayType.elementType();
    }
    AnalysisState::EvaluationGuard Expected(State, true, ElementType);
    const ExpressionResult Right = analyzeSimpleItem(State, *Node.right(), Depth + 1);
    if (!Right)
    {
      return {};
    }
    if (!Right.ValueObject || &Right.ValueObject->type() != ElementType)
    {
      State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), describeType(*ElementType), Right.ValueObject ? describeType(Right.ValueObject->type()) : "void");
      return {};
    }
    if (!Constant::classof(Right.ValueObject))
    {
      reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
      return {};
    }
    if (needsDestruction(State, *ElementType))
    {
      const Value *Address = resolveComptimeReceiver(State, *Node.left(), Depth + 1, Path);
      if (!Address || !destroyObject(State, *ElementType, *Address, Node))
      {
        return {};
      }
      takeTemporary(State, Right.TemporaryAddress);
    }
    // Reload after evaluating the RHS so writes to sibling elements are preserved.
    const auto Current = Execution.Engine.load(Place.Place);
    if (!reportExecution(State, Current.Status, Node))
    {
      return {};
    }
    const Constant *Previous = Current.Value;
    std::vector<const Constant *> Parents;
    for (const std::size_t Index : Path)
    {
      Parents.push_back(Previous);
      Previous = ClassConstant::classof(Previous) ? static_cast<const ClassConstant *>(Previous)->fields()[Index] : static_cast<const ArrayConstant *>(Previous)->elements()[Index];
    }
    const Constant *Stored = static_cast<const Constant *>(Right.ValueObject);
    if (Node.op() != TokenKind::Assign)
    {
      const auto Updated = Execution.Engine.evaluateBinary(arrayAssignmentOperation(Node.op()), *Previous, *Stored);
      if (!reportExecution(State, Updated.Status, Node))
      {
        return {};
      }
      Stored = Updated.Value;
    }
    const Constant *Updated = Stored;
    for (std::size_t Index = Path.size(); Index > 0; --Index)
    {
      const auto &Parent = *Parents[Index - 1];
      const auto Children = ClassConstant::classof(&Parent) ? static_cast<const ClassConstant &>(Parent).fields() : static_cast<const ArrayConstant &>(Parent).elements();
      std::vector<const Constant *> Elements(Children.begin(), Children.end());
      Elements[Path[Index - 1]] = Updated;
      Updated = ClassConstant::classof(&Parent) ? static_cast<const Constant *>(State.Context.constantPool().getClassConstant(static_cast<const ClassConstant &>(Parent).classType(), Elements)) : State.Context.constantPool().getArrayConstant(static_cast<const ArrayConstant &>(Parent).arrayType(), Elements);
    }
    return reportExecution(State, Execution.Engine.store(Place.Place, *Updated), Node) ? ExpressionResult{Stored} : ExpressionResult{};
  }
} // namespace ink::semantic
