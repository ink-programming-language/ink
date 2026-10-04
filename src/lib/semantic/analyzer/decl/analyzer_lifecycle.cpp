#include "../analyzer_internal.h"

#include "ink/parser/ast.h"
#include "ink/ir/analysis/type_layout.h"

#include <algorithm>

namespace ink::semantic
{
  using namespace ir;

  bool Analyzer::needsDestruction(const Type &Type) const
  {
    if (ClassType::classof(&Type))
    {
      return true;
    }
    return ArrayType::classof(&Type) && static_cast<const ArrayType &>(Type).elementCount() != 0 && needsDestruction(static_cast<const ArrayType &>(Type).elementType());
  }

  void Analyzer::recordLifecycleDependency(AnalysisState &State, const Function &Function)
  {
    const auto Owner = State.Context.classState().MethodOwners.find(&Function);
    if (State.CurrentModule && Owner != State.Context.classState().MethodOwners.end() && State.Context.classState().Definitions.at(Owner->second).Module != State.CurrentModule)
    {
      State.Context.recordModuleImport(*State.CurrentModule, Function);
    }
    if (State.Modules && State.CurrentFunction)
    {
      State.Modules->Dependencies[State.CurrentFunction].insert(&Function);
    }
  }

  bool Analyzer::declareClassLifecycle(AnalysisState &State, const ClassType &Class)
  {
    const auto &Definition = State.Context.classState().Definitions.at(&Class);
    const Type *Parameters[] = {State.Context.typePool().getType<TypeKind::Pointer>(Class, AccessKind::ReadWrite)};
    const Name ParameterNames[] = {State.Context.namePool().intern("this")};
    const auto *Signature = State.Context.typePool().getType<TypeKind::Function>(State.Context.typePool().getType<TypeKind::Void>(), Parameters);
    for (std::string_view Text : {"__init__", "__del__"})
    {
      const Name Name = State.Context.namePool().intern(Text);
      if (const auto *Existing = State.Resolver.lookupLocal(Name))
      {
        if (!std::all_of(Existing->targets().begin(), Existing->targets().end(), [](const Value *Value)
                         {
                           return Function::classof(Value);
                         }))
        {
          State.report<core::DiagnosticKind::SemanticInvalidClass>(Definition.Declaration->getSourceRange(), "lifecycle names are reserved for methods");
          return false;
        }
        continue;
      }
      // An implicit constructor exists only when every field has a default.
      if (Text == "__init__" && std::any_of(Definition.Fields.begin(), Definition.Fields.end(), [](const parser::FieldDecl *Field)
                                            {
                                              return !Field->initializer();
                                            }))
      {
        continue;
      }
      auto Owner = State.Builder.createFunction(Name, *Signature, {}, ParameterNames);
      if (!Owner || State.Resolver.bind(Name, *Owner) != NameResolver::BindResult::Inserted || !State.Builder.setClassMethod(Class, *Owner))
      {
        return false;
      }
      State.Context.classState().MethodOwners.emplace(Owner.get(), &Class);
      if (!State.Builder.appendValue(Definition.Module->entryBlock(), std::move(Owner)))
      {
        return false;
      }
    }
    return true;
  }

  bool Analyzer::initializeClassFields(AnalysisState &State, const parser::ASTNodeBase &Node)
  {
    const auto &Class = *State.CurrentClass;
    const auto &Definition = State.Context.classState().Definitions.at(&Class);
    const Value &Receiver = *State.CurrentFunction->parameters().front();
    State.ConstructorFields.assign(Class.fields().size(), false);
    if (Class.fields().empty())
    {
      const auto *Empty = State.Context.constantPool().getClassConstant(Class, {});
      return Empty && State.Builder.createStoreInstruction(Receiver, *Empty);
    }
    for (std::size_t Index = 0; Index < Class.fields().size(); ++Index)
    {
      const Function *Default = Definition.Defaults[Index];
      if (!Default)
      {
        continue;
      }
      recordLifecycleDependency(State, *Default);
      const auto *Value = State.Builder.createCallInstruction(*Default);
      const auto *Address = State.Builder.createFieldPointerInstruction(Receiver, Index);
      if (!Value || !Address || !State.Builder.createStoreInstruction(*Address, *Value))
      {
        State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
        return false;
      }
      State.ConstructorFields[Index] = true;
    }
    return true;
  }

  bool Analyzer::checkConstructorComplete(AnalysisState &State, const parser::ASTNodeBase &Node)
  {
    if (State.Constructing && std::find(State.ConstructorFields.begin(), State.ConstructorFields.end(), false) != State.ConstructorFields.end())
    {
      State.report<core::DiagnosticKind::SemanticInvalidConstruction>(Node.getSourceRange(), "__init__ must initialize every field before returning or exposing this");
      return false;
    }
    return true;
  }

  bool Analyzer::destroyObject(AnalysisState &State, const Type &Type, const Value &Address, const parser::ASTNodeBase &Node)
  {
    if (ClassType::classof(&Type))
    {
      const auto &Class = static_cast<const ClassType &>(Type);
      if (!ensureClassDefinition(State, Class, Node))
      {
        return false;
      }
      const auto *Binding = State.Resolver.lookupMember(*const_cast<ClassType *>(&Class), State.Context.namePool().find("__del__"));
      if (!Binding || Binding->targets().size() != 1)
      {
        return false;
      }
      const auto &Destructor = static_cast<const Function &>(*Binding->targets().front());
      recordLifecycleDependency(State, Destructor);
      const Value *Arguments[] = {&Address};
      return State.Evaluating ? static_cast<bool>(callComptime(State, Destructor, Arguments, Node)) : State.Builder.createCallInstruction(Destructor, Arguments) != nullptr;
    }
    if (ArrayType::classof(&Type) && needsDestruction(Type))
    {
      const auto &Array = static_cast<const ArrayType &>(Type);
      const auto *IndexType = State.Context.typePool().getType<TypeKind::Integer>(64, false);
      for (std::uint64_t Index = Array.elementCount(); Index != 0; --Index)
      {
        const auto *Offset = State.Context.constantPool().getIntegerConstant(*IndexType, IntegerBits(64, Index - 1));
        if (State.Evaluating)
        {
          auto &Execution = State.Context.comptimeState();
          auto Projection = State.Builder.createDetachedArrayElementPointerInstruction(Address, *Offset);
          const auto Pointer = Execution.Engine.resolveValue(Address, *State.Frame);
          const auto Layout = computeTypeLayout(Array.elementType(), State.Context.compilationContext().targetContext());
          if (!Projection || !Pointer || !Layout)
          {
            return false;
          }
          const auto ElementPointer = Execution.Engine.heap().pointer(Projection->type(), Pointer.Value.pointer().offsetBy(static_cast<std::size_t>((Index - 1) * Layout->Stride)));
          const auto Place = Execution.Engine.allocateValue(*State.Frame, Projection.get(), Projection->type(), false, &ElementPointer);
          const Value *Element = Projection.get();
          Execution.Projections.push_back(std::move(Projection));
          if (!reportExecution(State, Place.Status, Node) || !destroyObject(State, Array.elementType(), *Element, Node))
          {
            return false;
          }
        }
        else
        {
          const auto *Element = State.Builder.createArrayElementPointerInstruction(Address, *Offset);
          if (!Element || !destroyObject(State, Array.elementType(), *Element, Node))
          {
            return false;
          }
        }
      }
    }
    return true;
  }

  bool Analyzer::destroyClassFields(AnalysisState &State, const parser::ASTNodeBase &Node)
  {
    if (!State.Destroying)
    {
      return true;
    }
    const auto Fields = State.CurrentClass->fields();
    for (std::size_t Index = Fields.size(); Index != 0; --Index)
    {
      const auto &Field = Fields[Index - 1];
      if (!needsDestruction(*Field.FieldType))
      {
        continue;
      }
      const Value *Address = State.Builder.createFieldPointerInstruction(*State.CurrentFunction->parameters().front(), Index - 1);
      if (!Address || !destroyObject(State, *Field.FieldType, *Address, Node))
      {
        return false;
      }
    }
    return true;
  }

  bool Analyzer::trackObject(AnalysisState &State, const Value &Address, const Type &Type, bool Initialized, const Value *TemporaryValue)
  {
    if (!needsDestruction(Type))
    {
      return true;
    }
    const Value *Flag = nullptr;
    if (!State.Evaluating)
    {
      // Flags dominate every branch, including skipped short-circuit temporaries.
      IRBuilder Entry(State.Context.irContext());
      BasicBlock &Block = *State.CurrentFunction->entryBlock();
      Entry.setInsertPoint(Block, Block.values().empty() ? nullptr : Block.values().front().get());
      Flag = Entry.createAllocaInstruction(State.Context.typePool().getType<TypeKind::Bool>());
      if (!Flag || !Entry.createStoreInstruction(*Flag, State.Context.constantPool().getBoolConstant(false)) || (Initialized && !State.Builder.createStoreInstruction(*Flag, State.Context.constantPool().getBoolConstant(true))))
      {
        return false;
      }
    }
    State.Lifetimes.push_back({&Address, &Type, Flag, TemporaryValue, State.Evaluating, !State.Evaluating || Initialized});
    return true;
  }

  Analyzer::ExpressionResult Analyzer::trackTemporary(AnalysisState &State, const Value *Value, const parser::Expr &Node)
  {
    if (!Value || !needsDestruction(Value->type()))
    {
      return {Value};
    }
    const auto *Address = materializeClassReceiver(State, *Value, Node);
    return Address ? ExpressionResult{Value, nullptr, false, false, Address} : ExpressionResult{};
  }

  const Value *Analyzer::takeTemporary(AnalysisState &State, const Value *Address)
  {
    if (!Address)
    {
      return nullptr;
    }
    for (auto &Lifetime : State.Lifetimes)
    {
      if (Lifetime.Active && Lifetime.TemporaryValue && Lifetime.Address == Address && Lifetime.Comptime == State.Evaluating)
      {
        Lifetime.Active = false;
        return Lifetime.Address;
      }
    }
    return nullptr;
  }

  bool Analyzer::cleanupObjects(AnalysisState &State, std::size_t Begin, bool TemporariesOnly, const parser::ASTNodeBase &Node)
  {
    for (std::size_t Index = State.Lifetimes.size(); Index > Begin; --Index)
    {
      const auto Lifetime = State.Lifetimes[Index - 1];
      if (!Lifetime.Active || Lifetime.Comptime != State.Evaluating || (TemporariesOnly && !Lifetime.TemporaryValue))
      {
        continue;
      }
      if (State.Evaluating)
      {
        if (!destroyObject(State, *Lifetime.Type, *Lifetime.Address, Node))
        {
          return false;
        }
      }
      else
      {
        const auto *Ready = State.Builder.createLoadInstruction(*Lifetime.Initialized);
        auto *Destroy = State.Builder.createBasicBlock(*State.CurrentFunction);
        auto *Continue = State.Builder.createBasicBlock(*State.CurrentFunction);
        if (!Ready || !Destroy || !Continue || !State.Builder.createConditionalBranchInstruction(*Ready, *Destroy, *Continue) || !State.Builder.setInsertPoint(*Destroy) || !destroyObject(State, *Lifetime.Type, *Lifetime.Address, Node) || !State.Builder.createStoreInstruction(*Lifetime.Initialized, State.Context.constantPool().getBoolConstant(false)) || !State.Builder.createBranchInstruction(*Continue) || !State.Builder.setInsertPoint(*Continue))
        {
          return false;
        }
      }
      if (TemporariesOnly)
      {
        State.Lifetimes[Index - 1].Active = false;
      }
    }
    return true;
  }

  bool Analyzer::destroyPrevious(AnalysisState &State, const Value &Address, const Type &Type, const parser::ASTNodeBase &Node)
  {
    if (!needsDestruction(Type))
    {
      return true;
    }
    for (const auto &Lifetime : State.Lifetimes)
    {
      if (Lifetime.TemporaryValue || Lifetime.Address != &Address || Lifetime.Comptime != State.Evaluating)
      {
        continue;
      }
      if (!Lifetime.Active)
      {
        return true;
      }
      if (State.Evaluating)
      {
        return destroyObject(State, Type, Address, Node);
      }
      const auto *Ready = State.Builder.createLoadInstruction(*Lifetime.Initialized);
      auto *Destroy = State.Builder.createBasicBlock(*State.CurrentFunction);
      auto *Continue = State.Builder.createBasicBlock(*State.CurrentFunction);
      return Ready && Destroy && Continue && State.Builder.createConditionalBranchInstruction(*Ready, *Destroy, *Continue) && State.Builder.setInsertPoint(*Destroy) && destroyObject(State, Type, Address, Node) && State.Builder.createBranchInstruction(*Continue) && State.Builder.setInsertPoint(*Continue);
    }
    return destroyObject(State, Type, Address, Node);
  }

  bool Analyzer::finishObjectScope(AnalysisState &State, std::size_t Begin, const parser::ASTNodeBase &Node, bool Succeeded)
  {
    if (Succeeded && !State.Terminated)
    {
      Succeeded = cleanupObjects(State, Begin, false, Node);
    }
    if (Succeeded && !State.Evaluating)
    {
      AnalysisState::EvaluationGuard CompileTime(State);
      Succeeded = cleanupObjects(State, Begin, false, Node);
    }
    State.Lifetimes.resize(Begin);
    return Succeeded;
  }
} // namespace ink::semantic
