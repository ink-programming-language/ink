#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  using namespace ink::ir;

  const Value *Analyzer::resolveAddress(AnalysisState &State, const parser::Expr &Node, std::size_t Depth, bool RequireInitialized)
  {
    const auto ReportInvalidOperand = [&]()
    {
      if (RequireInitialized)
      {
        State.report<core::DiagnosticKind::SemanticInvalidAddressOperand>(Node.getSourceRange());
      }
      else
      {
        State.report<core::DiagnosticKind::SemanticInvalidAssignment>(Node.getSourceRange());
      }
    };
    if (Depth >= State.ExpressionDepthLimit)
    {
      State.report<core::DiagnosticKind::SemanticNestingLimit>(Node.getSourceRange());
      return nullptr;
    }
    if (State.Evaluating || Node.isComptime())
    {
      ReportInvalidOperand();
      return nullptr;
    }
    if (parser::ParenExpr::classof(&Node))
    {
      return resolveAddress(State, *static_cast<const parser::ParenExpr &>(Node).expression(), Depth + 1, RequireInitialized);
    }
    if (parser::IndexExpr::classof(&Node))
    {
      const auto &Index = static_cast<const parser::IndexExpr &>(Node);
      if (Index.optional())
      {
        State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.getSourceRange(), "array index", "optional index");
        return nullptr;
      }
      const Value *Object = resolveAddress(State, *Index.object(), Depth + 1, true);
      if (!Object)
      {
        return nullptr;
      }
      const Type &ObjectType = static_cast<const PointerType &>(Object->type()).pointeeType();
      if (!ArrayType::classof(&ObjectType))
      {
        State.report<core::DiagnosticKind::SemanticTypeMismatch>(Index.object()->getSourceRange(), "array", describeType(ObjectType));
        return nullptr;
      }
      const Value *Offset = analyzeArrayIndex(State, *Index.index(), static_cast<const ArrayType &>(ObjectType).elementCount(), Depth + 1);
      return Offset ? State.Builder.createArrayElementPointerInstruction(*Object, *Offset) : nullptr;
    }
    if (parser::UnaryExpr::classof(&Node) && static_cast<const parser::UnaryExpr &>(Node).op() == tokenizer::TokenKind::Star)
    {
      const auto &Unary = static_cast<const parser::UnaryExpr &>(Node);
      AnalysisState::EvaluationGuard Expected(State, false, nullptr);
      const ExpressionResult Pointer = analyzeExpr(State, *Unary.operand(), Depth + 1);
      if (!Pointer)
      {
        return nullptr;
      }
      if (!Pointer.ValueObject || !PointerType::classof(&Pointer.ValueObject->type()) || static_cast<const PointerType &>(Pointer.ValueObject->type()).pointeeType().typeKind() == TypeKind::Void)
      {
        State.report<core::DiagnosticKind::SemanticInvalidDereference>(Node.getSourceRange());
        return nullptr;
      }
      if (static_cast<const PointerType &>(Pointer.ValueObject->type()).access() != AccessKind::ReadWrite)
      {
        State.report<core::DiagnosticKind::SemanticInvalidAssignment>(Node.getSourceRange());
        return nullptr;
      }
      return Pointer.ValueObject;
    }
    if (!parser::NameExpr::classof(&Node))
    {
      ReportInvalidOperand();
      return nullptr;
    }
    const auto Name = static_cast<const parser::NameExpr &>(Node).name();
    const auto *Binding = State.Resolver.lookup(State.Context.namePool().find(Name.Text));
    if (!Binding)
    {
      State.report<core::DiagnosticKind::SemanticUnknownName>(Node.getSourceRange(), Name.Text);
      return nullptr;
    }
    if (Binding->targets().size() != 1)
    {
      State.report<core::DiagnosticKind::SemanticAmbiguousName>(Node.getSourceRange());
      return nullptr;
    }
    const Value *Address = Binding->targets().front();
    const auto &Variables = State.Context.comptimeState().Variables;
    const auto Variable = Variables.find(Address);
    if (!AllocaInstruction::classof(Address) || Variable == Variables.end() || Variable->second.Comptime || Variable->second.Constant)
    {
      ReportInvalidOperand();
      return nullptr;
    }
    if (Variable->second.Function != State.CurrentFunction)
    {
      State.report<core::DiagnosticKind::SemanticInvalidCapture>(Node.getSourceRange(), Name.Text);
      return nullptr;
    }
    if (RequireInitialized && !Variable->second.Initialized)
    {
      State.report<core::DiagnosticKind::SemanticUninitializedRead>(Node.getSourceRange(), Name.Text);
      return nullptr;
    }
    return Address;
  }

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
    if (State.Modules && State.CurrentFunction && Function::classof(Result))
    {
      State.Modules->Dependencies[State.CurrentFunction].insert(static_cast<const Function *>(Result));
    }
    auto &Execution = State.Context.comptimeState();
    if (!State.Evaluating && !State.ComptimeFunction && Function::classof(Result))
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
      reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
      return {};
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
