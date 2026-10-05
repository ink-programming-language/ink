#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  using namespace ir;
  using namespace execution;

  bool Analyzer::analyzeVarDecl(AnalysisState &State, const parser::VarDecl &Node)
  {
    const bool Comptime = State.Evaluating || Node.isComptime();
    if ((!Comptime && !State.CurrentFunction) || !Node.attributes().empty() || !parser::NameBindingPattern::classof(Node.binding()))
    {
      return reportUnsupported(State, Node);
    }
    const auto NameToken = static_cast<const parser::NameBindingPattern *>(Node.binding())->name();
    if (State.CurrentClass && NameToken.Text == "this")
    {
      State.report<core::DiagnosticKind::SemanticDuplicateName>(NameToken.Range, NameToken.Text);
      return false;
    }
    const Name Symbol = State.Context.namePool().intern(NameToken.Text);
    if (State.Resolver.lookupLocal(Symbol) || State.Resolver.lookupLocal<Decl *>(Symbol))
    {
      State.report<core::DiagnosticKind::SemanticDuplicateName>(NameToken.Range, NameToken.Text);
      return false;
    }
    const Type *ValueType = Node.type() ? analyzeType(State, *Node.type()->expression()) : nullptr;
    if (Node.type() && !ValueType)
    {
      return false;
    }
    if (!ValueType && !Node.initializer())
    {
      State.report<core::DiagnosticKind::SemanticMissingVariableType>(Node.getSourceRange());
      return false;
    }
    const Value *Initial = nullptr;
    const Value *InitialTemporary = nullptr;
    if (Node.initializer())
    {
      ExpressionResult Result;
      AnalysisState::EvaluationGuard Expected(State, Comptime, ValueType);
      Result = analyzeExpr(State, *Node.initializer());
      if (!Result)
      {
        return false;
      }
      if (!ValueType)
      {
        ValueType = Result.IntegerLiteral ? State.Context.typePool().getType<TypeKind::Integer>(32, true) : (Result.ValueObject ? &Result.ValueObject->type() : &State.Context.typePool().getType<TypeKind::Void>());
      }
      Initial = convertExpression(State, Result, *ValueType, *Node.initializer());
      InitialTemporary = Result.TemporaryAddress;
      if (!Initial)
      {
        return false;
      }
    }
    if (!ValueType || (Comptime && ValueType->typeKind() != TypeKind::Integer && ValueType->typeKind() != TypeKind::Bool && ValueType->typeKind() != TypeKind::Float && ValueType->typeKind() != TypeKind::Array && ValueType->typeKind() != TypeKind::Class && !StringConstant::classof(Initial)))
    {
      return reportExecution(State, ExecutionStatus::UnsupportedOperation, Node);
    }
    AnalysisState::EvaluationGuard LifetimeMode(State, Comptime);
    const Value *Temporary = takeTemporary(State, InitialTemporary);
    auto Storage = Temporary ? nullptr : State.Builder.createDetachedAllocaInstruction(*ValueType);
    if (!Temporary && !Storage)
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    AllocaInstruction *Address = Temporary ? const_cast<AllocaInstruction *>(static_cast<const AllocaInstruction *>(Temporary)) : Storage.get();
    auto &Execution = State.Context.comptimeState();
    if (Comptime && !Temporary)
    {
      if (Initial && !Constant::classof(Initial))
      {
        return reportExecution(State, ExecutionStatus::RuntimeValue, Node);
      }
      const auto Place = Execution.Engine.allocate(*State.Frame, Address, *ValueType, !Node.constant() || needsDestruction(State, *ValueType), static_cast<const Constant *>(Initial));
      if (!reportExecution(State, Place.Status, Node))
      {
        return false;
      }
      Execution.Bindings.push_back(std::move(Storage));
    }
    else if (!Comptime && !Temporary)
    {
      if (!State.Builder.appendValue(*State.Builder.insertBlock(), std::move(Storage)))
      {
        return false;
      }
      if (Initial && !State.Builder.createStoreInstruction(*Address, *Initial))
      {
        State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
        return false;
      }
    }
    Execution.Variables.insert_or_assign(Address, ComptimeState::Variable{Comptime, Node.constant(), Initial != nullptr, State.CurrentFunction});
    if (State.Resolver.bind(Symbol, *Address) != NameResolver::BindResult::Inserted)
    {
      State.report<core::DiagnosticKind::SemanticDuplicateName>(NameToken.Range, NameToken.Text);
      return false;
    }
    return trackObject(State, *Address, *ValueType, Initial != nullptr) && cleanupObjects(State, 0, true, Node);
  }
} // namespace ink::semantic
