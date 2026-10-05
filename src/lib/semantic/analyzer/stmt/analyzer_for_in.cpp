#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

namespace ink::semantic
{
  using namespace ir;

  bool Analyzer::bindIterationValue(AnalysisState &State, const parser::BindingPattern &Pattern, const Value &ValueObject, bool Comptime, std::size_t Depth)
  {
    if (Depth >= State.ExpressionDepthLimit)
    {
      State.report<core::DiagnosticKind::SemanticNestingLimit>(Pattern.getSourceRange());
      return false;
    }
    if (parser::WildcardBindingPattern::classof(&Pattern))
    {
      return true;
    }
    if (parser::NameBindingPattern::classof(&Pattern))
    {
      const auto NameToken = static_cast<const parser::NameBindingPattern &>(Pattern).name();
      const auto Symbol = State.Context.namePool().intern(NameToken.Text);
      if (State.Resolver.lookupLocal(Symbol) || State.Resolver.lookupLocal<Decl *>(Symbol) || (State.CurrentClass && NameToken.Text == "this"))
      {
        State.report<core::DiagnosticKind::SemanticDuplicateName>(NameToken.Range, NameToken.Text);
        return false;
      }
      auto Storage = State.Builder.createDetachedAllocaInstruction(ValueObject.type());
      if (!Storage)
      {
        return false;
      }
      auto *Address = Storage.get();
      if (Comptime)
      {
        if (!Constant::classof(&ValueObject))
        {
          return reportExecution(State, execution::ExecutionStatus::RuntimeValue, Pattern);
        }
        const auto Place = State.Context.comptimeState().Engine.allocate(*State.Frame, Address, ValueObject.type(), true, static_cast<const Constant *>(&ValueObject));
        if (!reportExecution(State, Place.Status, Pattern))
        {
          return false;
        }
        State.Context.comptimeState().Bindings.push_back(std::move(Storage));
      }
      else if (!State.Builder.appendValue(*State.Builder.insertBlock(), std::move(Storage)) || !State.Builder.createStoreInstruction(*Address, ValueObject))
      {
        return false;
      }
      State.Context.comptimeState().Variables.insert_or_assign(Address, ComptimeState::Variable{Comptime, false, true, State.CurrentFunction});
      AnalysisState::EvaluationGuard Mode(State, Comptime);
      return State.Resolver.bind(Symbol, *Address) == NameResolver::BindResult::Inserted && trackObject(State, *Address, ValueObject.type(), true);
    }
    if (!parser::ArrayBindingPattern::classof(&Pattern) || !ArrayType::classof(&ValueObject.type()))
    {
      State.report<core::DiagnosticKind::SemanticInvalidIterationBinding>(Pattern.getSourceRange(), describeType(ValueObject.type()));
      return false;
    }
    const auto &ArrayPattern = static_cast<const parser::ArrayBindingPattern &>(Pattern);
    const auto &Type = static_cast<const ArrayType &>(ValueObject.type());
    const auto Elements = ArrayPattern.elements();
    const auto Rest = ArrayPattern.rest();
    if (Elements.size() > Type.elementCount() || (!Rest && Elements.size() != Type.elementCount()))
    {
      State.report<core::DiagnosticKind::SemanticInvalidIterationBinding>(Pattern.getSourceRange(), "array length mismatch");
      return false;
    }
    const auto *IndexType = State.Context.typePool().getType<TypeKind::Integer>(64, false);
    const auto Extract = [&](std::size_t Index) -> const Value *
    {
      if (ArrayConstant::classof(&ValueObject))
      {
        return static_cast<const ArrayConstant &>(ValueObject).elements()[Index];
      }
      const auto *Offset = State.Context.constantPool().getIntegerConstant(*IndexType, IntegerBits(64, Index));
      return State.Builder.createArrayExtractInstruction(ValueObject, *Offset);
    };
    for (std::size_t Index = 0; Index < Elements.size(); ++Index)
    {
      const auto *Element = Extract(Index);
      if (!Element || !bindIterationValue(State, *Elements[Index], *Element, Comptime, Depth + 1))
      {
        return false;
      }
    }
    if (Rest && !Rest->Wildcard)
    {
      const auto *RestType = State.Context.typePool().getType<TypeKind::Array>(Type.elementType(), Type.elementCount() - Elements.size());
      std::vector<const Value *> Values;
      std::vector<const Constant *> Constants;
      for (std::size_t Index = Elements.size(); Index < Type.elementCount(); ++Index)
      {
        const auto *Element = Extract(Index);
        if (!Element)
        {
          return false;
        }
        Values.push_back(Element);
        if (Constant::classof(Element))
        {
          Constants.push_back(static_cast<const Constant *>(Element));
        }
      }
      const Value *Remaining = Constants.size() == Values.size() ? static_cast<const Value *>(State.Context.constantPool().getArrayConstant(*RestType, Constants)) : State.Builder.createArrayInstruction(*RestType, Values);
      const parser::NameBindingPattern Binding(Rest->Name.Range, Rest->Name);
      return Remaining && bindIterationValue(State, Binding, *Remaining, Comptime, Depth + 1);
    }
    return true;
  }

  bool Analyzer::analyzeForInStmt(AnalysisState &State, const parser::ForInStmt &Node)
  {
    const bool Comptime = State.Evaluating || Node.isComptime();
    if (!Comptime && !State.CurrentFunction)
    {
      return reportUnsupported(State, Node);
    }
    NameResolver::ScopeGuard Scope(State.Resolver);
    AnalysisState::FrameGuard Frame(State, execution::ExecutionFrameKind::Block);
    if (!Frame)
    {
      return reportExecution(State, State.Context.comptimeState().Engine.lastStatus(), Node);
    }
    const std::size_t Begin = State.Lifetimes.size();
    ExpressionResult Iterable;
    {
      AnalysisState::EvaluationGuard Mode(State, Comptime);
      Iterable = analyzeExpr(State, *Node.iterable());
      if (!Iterable)
      {
        return false;
      }
      if (!Iterable.ValueObject || !ArrayType::classof(&Iterable.ValueObject->type()))
      {
        State.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.iterable()->getSourceRange(), "fixed-size array", Iterable.ValueObject ? describeType(Iterable.ValueObject->type()) : "non-array value");
        return false;
      }
      // The iterable is a value snapshot evaluated once, owned until loop exit.
      if (needsDestruction(State, Iterable.ValueObject->type()))
      {
        const auto *Address = Iterable.TemporaryAddress ? Iterable.TemporaryAddress : materializeClassReceiver(State, *Iterable.ValueObject, *Node.iterable());
        if (!Address || !takeTemporary(State, Address) || !trackObject(State, *Address, Iterable.ValueObject->type(), true))
        {
          return false;
        }
      }
      if (!cleanupObjects(State, 0, true, Node))
      {
        return false;
      }
    }
    const auto &Type = static_cast<const ArrayType &>(Iterable.ValueObject->type());
    std::unordered_set<std::string_view> Names;
    const auto Validate = [&](auto &&Self, const parser::BindingPattern &Pattern, const ir::Type &ElementType, std::size_t Depth) -> bool
    {
      if (Depth >= State.ExpressionDepthLimit)
      {
        State.report<core::DiagnosticKind::SemanticNestingLimit>(Pattern.getSourceRange());
        return false;
      }
      if (parser::WildcardBindingPattern::classof(&Pattern))
      {
        return true;
      }
      if (parser::NameBindingPattern::classof(&Pattern))
      {
        const auto Name = static_cast<const parser::NameBindingPattern &>(Pattern).name();
        if (!Names.insert(Name.Text).second || (State.CurrentClass && Name.Text == "this"))
        {
          State.report<core::DiagnosticKind::SemanticDuplicateName>(Name.Range, Name.Text);
          return false;
        }
        return true;
      }
      if (!parser::ArrayBindingPattern::classof(&Pattern) || !ArrayType::classof(&ElementType))
      {
        State.report<core::DiagnosticKind::SemanticInvalidIterationBinding>(Pattern.getSourceRange(), describeType(ElementType));
        return false;
      }
      const auto &ArrayPattern = static_cast<const parser::ArrayBindingPattern &>(Pattern);
      const auto &Array = static_cast<const ArrayType &>(ElementType);
      if (ArrayPattern.elements().size() > Array.elementCount() || (!ArrayPattern.rest() && ArrayPattern.elements().size() != Array.elementCount()))
      {
        State.report<core::DiagnosticKind::SemanticInvalidIterationBinding>(Pattern.getSourceRange(), "array length mismatch");
        return false;
      }
      for (const auto *Element : ArrayPattern.elements())
      {
        if (!Self(Self, *Element, Array.elementType(), Depth + 1))
        {
          return false;
        }
      }
      if (const auto Rest = ArrayPattern.rest(); Rest && !Rest->Wildcard && !Names.insert(Rest->Name.Text).second)
      {
        State.report<core::DiagnosticKind::SemanticDuplicateName>(Rest->Name.Range, Rest->Name.Text);
        return false;
      }
      return true;
    };
    if (!Validate(Validate, *Node.binding(), Type.elementType(), 0))
    {
      return finishObjectScope(State, Begin, Node, false);
    }
    bool Succeeded = true;
    if (Comptime)
    {
      if (!ArrayConstant::classof(Iterable.ValueObject))
      {
        return reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
      }
      AnalysisState::LoopGuard Loop(State);
      for (const auto *Element : static_cast<const ArrayConstant &>(*Iterable.ValueObject).elements())
      {
        if (!reportExecution(State, State.Context.comptimeState().Engine.consumeStep(), Node))
        {
          Succeeded = false;
          break;
        }
        NameResolver::ScopeGuard IterationScope(State.Resolver);
        AnalysisState::FrameGuard Iteration(State, execution::ExecutionFrameKind::Block);
        const std::size_t IterationBegin = State.Lifetimes.size();
        Succeeded = Iteration ? bindIterationValue(State, *Node.binding(), *Element, true) && analyzeStmt(State, *Node.body()) : reportExecution(State, State.Context.comptimeState().Engine.lastStatus(), Node);
        Succeeded = finishObjectScope(State, IterationBegin, Node, Succeeded);
        State.Continuing = false;
        if (!Succeeded || State.Terminated || State.Breaking)
        {
          break;
        }
      }
      State.Breaking = false;
    }
    else
    {
      const auto *IndexType = State.Context.typePool().getType<TypeKind::Integer>(64, false);
      const auto *Zero = State.Context.constantPool().getIntegerConstant(*IndexType, IntegerBits(64, 0));
      const auto *One = State.Context.constantPool().getIntegerConstant(*IndexType, IntegerBits(64, 1));
      const auto *Count = State.Context.constantPool().getIntegerConstant(*IndexType, IntegerBits(64, Type.elementCount()));
      const auto *Index = State.Builder.createAllocaInstruction(*IndexType);
      if (!Index || !State.Builder.createStoreInstruction(*Index, *Zero))
      {
        return false;
      }
      const auto Condition = [&]() -> const Value *
      {
        const auto *Position = State.Builder.createLoadInstruction(*Index);
        return Position ? State.Builder.createCompareInstruction(core::ComparisonPredicate::Less, *Position, *Count) : nullptr;
      };
      const auto Body = [&]()
      {
        const auto *Position = State.Builder.createLoadInstruction(*Index);
        const auto *Element = Position ? State.Builder.createArrayExtractInstruction(*Iterable.ValueObject, *Position) : nullptr;
        return Element && bindIterationValue(State, *Node.binding(), *Element, false) && analyzeStmt(State, *Node.body());
      };
      const auto Step = [&]()
      {
        const auto *Position = State.Builder.createLoadInstruction(*Index);
        const auto *Next = Position ? State.Builder.createAddInstruction(*Position, *One) : nullptr;
        return Next && State.Builder.createStoreInstruction(*Index, *Next);
      };
      Succeeded = analyzeRuntimeLoop(State, Node, Condition, Body, Step);
    }
    return finishObjectScope(State, Begin, Node, Succeeded);
  }
} // namespace ink::semantic
