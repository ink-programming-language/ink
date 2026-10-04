#include "../analyzer_internal.h"

#include "ink/parser/ast.h"
#include "ink/parser/parser.h"
#include "ink/ir/constant/class_constant.h"
#include "ink/ir/analysis/type_layout.h"

#include <algorithm>

namespace ink::semantic
{
  using namespace ir;

  bool Analyzer::isMutablePlace(AnalysisState &State, const parser::Expr &Node)
  {
    const parser::Expr *Current = &Node;
    for (std::size_t Depth = 0; Depth < State.ExpressionDepthLimit; ++Depth)
    {
      if (parser::ParenExpr::classof(Current))
      {
        Current = static_cast<const parser::ParenExpr *>(Current)->expression();
      }
      else if (parser::IndexExpr::classof(Current))
      {
        Current = static_cast<const parser::IndexExpr *>(Current)->object();
      }
      else if (parser::MemberExpr::classof(Current))
      {
        const auto &Member = static_cast<const parser::MemberExpr &>(*Current);
        if (Member.access() == tokenizer::TokenKind::Arrow)
        {
          return true;
        }
        Current = Member.object();
      }
      else if (parser::UnaryExpr::classof(Current))
      {
        return static_cast<const parser::UnaryExpr *>(Current)->op() == tokenizer::TokenKind::Star;
      }
      else if (parser::NameExpr::classof(Current))
      {
        const auto Name = static_cast<const parser::NameExpr *>(Current)->name();
        const auto *Binding = State.Resolver.lookup(State.Context.namePool().find(Name.Text));
        if (!Binding || Binding->targets().size() != 1)
        {
          return false;
        }
        if (Name.Text == "this" && State.CurrentClass)
        {
          return true;
        }
        const auto Found = State.Context.comptimeState().Variables.find(Binding->targets().front());
        return Found != State.Context.comptimeState().Variables.end() && !Found->second.Constant && Found->second.Comptime == State.Evaluating;
      }
      else
      {
        return false;
      }
    }
    return false;
  }

  std::optional<std::size_t> Analyzer::lookupClassField(AnalysisState &State, const ClassType &Class, const parser::MemberExpr &Node)
  {
    const Name Name = State.Context.namePool().find(Node.member().Text);
    for (std::size_t Index = 0; Index < Class.fields().size(); ++Index)
    {
      const auto &Field = Class.fields()[Index];
      if (Field.FieldName == Name)
      {
        if (Field.Visibility == core::VisibilityKind::Private && State.CurrentClass != &Class)
        {
          State.report<core::DiagnosticKind::SemanticInvalidMember>(Node.member().Range, "field is private");
          return std::nullopt;
        }
        return Index;
      }
    }
    State.report<core::DiagnosticKind::SemanticInvalidMember>(Node.member().Range, "field does not exist or method was used without a call");
    return std::nullopt;
  }

  const Value *Analyzer::resolveFieldAddress(AnalysisState &State, const parser::MemberExpr &Node, std::size_t Depth, bool RequireInitialized)
  {
    const parser::Expr *Object = Node.object();
    while (parser::ParenExpr::classof(Object))
    {
      Object = static_cast<const parser::ParenExpr *>(Object)->expression();
    }
    const bool This = parser::NameExpr::classof(Object) && static_cast<const parser::NameExpr *>(Object)->name().Text == "this" && State.CurrentClass;
    if (Node.access() != tokenizer::TokenKind::Dot && Node.access() != tokenizer::TokenKind::Arrow)
    {
      State.report<core::DiagnosticKind::SemanticInvalidMember>(Node.getSourceRange(), "optional member access is not supported");
      return nullptr;
    }
    const Value *Address = nullptr;
    if (Node.access() == tokenizer::TokenKind::Arrow || This)
    {
      AnalysisState::EvaluationGuard Expected(State, State.Evaluating, nullptr);
      const bool Saved = State.AccessingConstructorField;
      State.AccessingConstructorField = This;
      Address = analyzeExpr(State, *Node.object(), Depth + 1).ValueObject;
      State.AccessingConstructorField = Saved;
    }
    else
    {
      Address = resolveAddress(State, *Node.object(), Depth + 1);
    }
    if (!Address)
    {
      return nullptr;
    }
    if (!PointerType::classof(&Address->type()) || !ClassType::classof(&static_cast<const PointerType &>(Address->type()).pointeeType()))
    {
      State.report<core::DiagnosticKind::SemanticInvalidMember>(Node.getSourceRange(), "member storage requires a class object or class pointer");
      return nullptr;
    }
    const auto &Pointer = static_cast<const PointerType &>(Address->type());
    if (Pointer.access() != AccessKind::ReadWrite)
    {
      State.report<core::DiagnosticKind::SemanticInvalidAssignment>(Node.getSourceRange());
      return nullptr;
    }
    const auto Index = lookupClassField(State, static_cast<const ClassType &>(Pointer.pointeeType()), Node);
    if (Index && This && State.Constructing && RequireInitialized && !State.ConstructorFields[*Index])
    {
      State.report<core::DiagnosticKind::SemanticUninitializedRead>(Node.getSourceRange(), Node.member().Text);
      return nullptr;
    }
    return Index ? State.Builder.createFieldPointerInstruction(*Address, *Index) : nullptr;
  }

  const Value *Analyzer::resolveComptimeReceiver(AnalysisState &State, const parser::Expr &Node, std::size_t Depth, std::span<const std::size_t> EvaluatedPath)
  {
    std::vector<const parser::Expr *> Path;
    const parser::Expr *Root = &Node;
    while (parser::ParenExpr::classof(Root) || parser::MemberExpr::classof(Root) || parser::IndexExpr::classof(Root))
    {
      if (++Depth >= State.ExpressionDepthLimit)
      {
        State.report<core::DiagnosticKind::SemanticNestingLimit>(Node.getSourceRange());
        return nullptr;
      }
      if (parser::ParenExpr::classof(Root))
      {
        Root = static_cast<const parser::ParenExpr *>(Root)->expression();
      }
      else
      {
        Path.push_back(Root);
        Root = parser::MemberExpr::classof(Root) ? static_cast<const parser::MemberExpr *>(Root)->object() : static_cast<const parser::IndexExpr *>(Root)->object();
      }
    }
    const Value *Address = resolveVariable(State, *Root);
    auto &Execution = State.Context.comptimeState();
    const auto Found = Execution.Variables.find(Address);
    if (!Address || Found == Execution.Variables.end() || !Found->second.Comptime || Found->second.Constant)
    {
      State.report<core::DiagnosticKind::SemanticInvalidMember>(Node.getSourceRange(), "compile-time method requires a mutable compile-time object");
      return nullptr;
    }
    const auto RootPlace = Execution.Engine.lookup(*State.Frame, Address);
    if (!reportExecution(State, RootPlace.Status, Node) || !reportExecution(State, Execution.Engine.load(RootPlace.Place).Status, Node))
    {
      return nullptr;
    }
    const Type *ProjectedType = &static_cast<const AllocaInstruction *>(Address)->allocatedType();
    std::size_t ByteOffset = 0;
    std::reverse(Path.begin(), Path.end());
    if (!EvaluatedPath.empty() && EvaluatedPath.size() != Path.size())
    {
      return nullptr;
    }
    std::size_t ProjectionIndex = 0;
    for (const parser::Expr *Projection : Path)
    {
      std::unique_ptr<Value> Descriptor;
      const auto Layout = computeTypeLayout(*ProjectedType, State.Context.compilationContext().targetContext());
      if (parser::MemberExpr::classof(Projection))
      {
        const auto &Member = static_cast<const parser::MemberExpr &>(*Projection);
        if (Member.access() != tokenizer::TokenKind::Dot || !ClassType::classof(ProjectedType) || !Layout)
        {
          State.report<core::DiagnosticKind::SemanticInvalidMember>(Projection->getSourceRange(), "invalid compile-time field receiver");
          return nullptr;
        }
        const auto &Class = static_cast<const ClassType &>(*ProjectedType);
        const auto Index = lookupClassField(State, Class, Member);
        if (!Index)
        {
          return nullptr;
        }
        ByteOffset += static_cast<std::size_t>(Layout->FieldOffsets[*Index]);
        Descriptor = State.Builder.createDetachedFieldPointerInstruction(*Address, *Index);
        ProjectedType = Class.fields()[*Index].FieldType;
      }
      else
      {
        const auto &Index = static_cast<const parser::IndexExpr &>(*Projection);
        if (Index.optional() || !ArrayType::classof(ProjectedType) || !Layout)
        {
          State.report<core::DiagnosticKind::SemanticInvalidMember>(Projection->getSourceRange(), "invalid compile-time array receiver");
          return nullptr;
        }
        const auto &Array = static_cast<const ArrayType &>(*ProjectedType);
        // Assignment has already evaluated its indices before the right-hand side.
        const Value *Offset = EvaluatedPath.empty() ? analyzeArrayIndex(State, *Index.index(), Array.elementCount(), Depth + 1) : State.Context.constantPool().getIntegerConstant(*State.Context.typePool().getType<TypeKind::Integer>(64, false), IntegerBits(64, EvaluatedPath[ProjectionIndex]));
        if (!Offset || !IntegerConstant::classof(Offset))
        {
          return nullptr;
        }
        const auto ElementLayout = computeTypeLayout(Array.elementType(), State.Context.compilationContext().targetContext());
        if (!ElementLayout)
        {
          return nullptr;
        }
        ByteOffset += static_cast<std::size_t>(static_cast<const IntegerConstant *>(Offset)->value().words().front() * ElementLayout->Stride);
        Descriptor = State.Builder.createDetachedArrayElementPointerInstruction(*Address, *Offset);
        ProjectedType = &Array.elementType();
      }
      if (!Descriptor)
      {
        State.report<core::DiagnosticKind::SemanticConstructionFailed>(Projection->getSourceRange());
        return nullptr;
      }
      const auto Pointer = Execution.Engine.heap().pointer(Descriptor->type(), execution::ExecutionPointer::fromPlace(RootPlace.Place, ByteOffset));
      const auto Binding = Execution.Engine.allocateValue(*State.Frame, Descriptor.get(), Descriptor->type(), false, &Pointer);
      if (!reportExecution(State, Binding.Status, *Projection))
      {
        return nullptr;
      }
      Address = Descriptor.get();
      Execution.Projections.push_back(std::move(Descriptor));
      ++ProjectionIndex;
    }
    return Address;
  }

  const Value *Analyzer::materializeClassReceiver(AnalysisState &State, const Value &Object, const parser::Expr &Node)
  {
    if (State.Evaluating)
    {
      if (!Constant::classof(&Object))
      {
        reportExecution(State, execution::ExecutionStatus::RuntimeValue, Node);
        return nullptr;
      }
      auto Storage = State.Builder.createDetachedAllocaInstruction(Object.type());
      if (!Storage)
      {
        State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
        return nullptr;
      }
      auto &Execution = State.Context.comptimeState();
      const auto Place = Execution.Engine.allocate(*State.Frame, Storage.get(), Object.type(), true, static_cast<const Constant *>(&Object));
      if (!reportExecution(State, Place.Status, Node))
      {
        return nullptr;
      }
      const Value *Address = Storage.get();
      Execution.Bindings.push_back(std::move(Storage));
      return trackObject(State, *Address, Object.type(), true, &Object) ? Address : nullptr;
    }
    auto *Storage = State.Builder.createAllocaInstruction(Object.type());
    if (!Storage || !State.Builder.createStoreInstruction(*Storage, Object))
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return nullptr;
    }
    return trackObject(State, *Storage, Object.type(), true, &Object) ? Storage : nullptr;
  }

  Analyzer::ExpressionResult Analyzer::analyzeMethodCall(AnalysisState &State, const parser::MemberExpr &Member, const parser::CallExpr &Node, std::size_t Depth)
  {
    if (Member.member().Text == "__init__" || Member.member().Text == "__del__")
    {
      State.report<core::DiagnosticKind::SemanticInvalidMember>(Member.member().Range, "lifecycle methods are invoked automatically; use the class name to construct an object");
      return {};
    }
    if (Member.access() != tokenizer::TokenKind::Dot && Member.access() != tokenizer::TokenKind::Arrow)
    {
      State.report<core::DiagnosticKind::SemanticInvalidMember>(Member.getSourceRange(), "optional member calls are not supported");
      return {};
    }
    AnalysisState::EvaluationGuard Expected(State, State.Evaluating, nullptr);
    const bool This = State.CurrentClass && parser::NameExpr::classof(Member.object()) && static_cast<const parser::NameExpr *>(Member.object())->name().Text == "this";
    const Value *Receiver = nullptr;
    const Value *Object = nullptr;
    const Value *TemporaryReceiver = nullptr;
    if (!State.Evaluating && (Member.access() == tokenizer::TokenKind::Arrow || This))
    {
      Receiver = analyzeExpr(State, *Member.object(), Depth + 1).ValueObject;
    }
    else if (isMutablePlace(State, *Member.object()))
    {
      Receiver = State.Evaluating ? resolveComptimeReceiver(State, *Member.object(), Depth + 1) : resolveAddress(State, *Member.object(), Depth + 1);
    }
    else
    {
      const auto Result = analyzeExpr(State, *Member.object(), Depth + 1);
      Object = Result.ValueObject;
      TemporaryReceiver = Result.TemporaryAddress;
    }
    if (!Receiver && !Object)
    {
      return {};
    }
    if (Object && ClassType::classof(Object))
    {
      const auto *Binding = State.Resolver.lookupMember(*const_cast<Value *>(Object), State.Context.namePool().find(Member.member().Text));
      if (Binding && Binding->targets().size() == 1 && ClassType::classof(Binding->targets().front()))
      {
        const auto &Nested = State.Context.classState().Definitions.at(static_cast<const ClassType *>(Binding->targets().front()));
        if (Nested.Declaration->visibility() == parser::DeclarationVisibility::Private && State.CurrentClass != Object)
        {
          State.report<core::DiagnosticKind::SemanticInvalidMember>(Member.member().Range, "nested class is private");
          return {};
        }
        return analyzeClassConstruction(State, static_cast<const ClassType &>(*Binding->targets().front()), Node, Depth);
      }
    }
    std::vector<const Value *> Candidates;
    std::vector<ExpressionResult> Arguments;
    std::vector<const parser::Expr *> ArgumentNodes;
    if (Object && Module::classof(Object))
    {
      const auto *TypeBinding = State.Resolver.lookupMember(*const_cast<Value *>(Object), State.Context.namePool().find(Member.member().Text));
      if (TypeBinding && TypeBinding->targets().size() == 1 && ClassType::classof(TypeBinding->targets().front()))
      {
        const auto &Class = static_cast<const ClassType &>(*TypeBinding->targets().front());
        const auto &Definition = State.Context.classState().Definitions.at(&Class);
        if (Definition.Module != Object)
        {
          State.report<core::DiagnosticKind::SemanticUnsupportedImport>(Member.member().Range);
          return {};
        }
        if (State.CurrentModule != Definition.Module && Definition.Declaration->visibility() == parser::DeclarationVisibility::Private)
        {
          State.report<core::DiagnosticKind::SemanticInvalidMember>(Member.member().Range, "class is private");
          return {};
        }
        return analyzeClassConstruction(State, Class, Node, Depth);
      }
      if (!resolveMemberFunctions(State, Member, Candidates, Depth))
      {
        return {};
      }
    }
    else
    {
      if (Object && ClassType::classof(&Object->type()))
      {
        const parser::Expr *Root = Member.object();
        while (parser::ParenExpr::classof(Root) || parser::MemberExpr::classof(Root) || parser::IndexExpr::classof(Root))
        {
          Root = parser::ParenExpr::classof(Root) ? static_cast<const parser::ParenExpr *>(Root)->expression() : parser::MemberExpr::classof(Root) ? static_cast<const parser::MemberExpr *>(Root)->object()
                                                                                                                                                   : static_cast<const parser::IndexExpr *>(Root)->object();
        }
        if (parser::NameExpr::classof(Root))
        {
          State.report<core::DiagnosticKind::SemanticInvalidMember>(Member.getSourceRange(), "mutable methods cannot be called on const objects or value parameters");
          return {};
        }
        Receiver = TemporaryReceiver ? TemporaryReceiver : materializeClassReceiver(State, *Object, Node);
      }
      if (!Receiver || !PointerType::classof(&Receiver->type()) || !ClassType::classof(&static_cast<const PointerType &>(Receiver->type()).pointeeType()) || static_cast<const PointerType &>(Receiver->type()).access() != AccessKind::ReadWrite)
      {
        State.report<core::DiagnosticKind::SemanticInvalidMember>(Member.getSourceRange(), "method requires writable class storage");
        return {};
      }
      const auto &Class = static_cast<const ClassType &>(static_cast<const PointerType &>(Receiver->type()).pointeeType());
      const auto *Binding = State.Resolver.lookupMember(*const_cast<ClassType *>(&Class), State.Context.namePool().find(Member.member().Text));
      if (Binding)
      {
        for (const Value *Target : Binding->targets())
        {
          if (Function::classof(Target) && (static_cast<const Function *>(Target)->visibility() != core::VisibilityKind::Private || State.CurrentClass == &Class))
          {
            Candidates.push_back(Target);
          }
        }
      }
      if (Candidates.empty())
      {
        State.report<core::DiagnosticKind::SemanticInvalidMember>(Member.member().Range, "method does not exist or is private");
        return {};
      }
      Arguments.push_back({Receiver});
      ArgumentNodes.push_back(Member.object());
    }
    for (const auto &Argument : Node.arguments())
    {
      if (Argument.form() != parser::ArgumentKind::Positional)
      {
        State.report<core::DiagnosticKind::SemanticInvalidMember>(Argument.range(), "method calls accept positional arguments only");
        return {};
      }
      const auto Parameters = static_cast<const FunctionType &>(Candidates.front()->type()).parameterTypes();
      const Type *Parameter = Candidates.size() == 1 && Arguments.size() < Parameters.size() ? Parameters[Arguments.size()] : nullptr;
      const bool Deferred = State.Evaluating && !Parameter && findDeferredIntegerLiteral(*Argument.value(), Depth + 1, State.ExpressionDepthLimit);
      if (Deferred && !prepareIntegerLiteral(State, *Argument.value(), Depth + 1))
      {
        return {};
      }
      AnalysisState::EvaluationGuard Guard(State, State.Evaluating && !Deferred, Parameter);
      const ExpressionResult Value = analyzeExpr(State, *Argument.value(), Depth + 1);
      if (!Value)
      {
        return {};
      }
      Arguments.push_back(Value);
      ArgumentNodes.push_back(Argument.value());
    }
    return finishCall(State, Candidates, Arguments, ArgumentNodes, Node);
  }

  Analyzer::ExpressionResult Analyzer::callClassOperator(AnalysisState &State, const ExpressionResult &Receiver, std::string_view Method, std::span<const ExpressionResult> Operands, const parser::Expr &Node)
  {
    const auto &Class = static_cast<const ClassType &>(Receiver.ValueObject->type());
    const auto *Binding = State.Resolver.lookupMember(*const_cast<ClassType *>(&Class), State.Context.namePool().find(Method));
    std::vector<const Value *> Candidates;
    if (Binding)
    {
      for (const Value *Target : Binding->targets())
      {
        if (Function::classof(Target) && (static_cast<const Function *>(Target)->visibility() != core::VisibilityKind::Private || State.CurrentClass == &Class))
        {
          Candidates.push_back(Target);
        }
      }
    }
    if (Candidates.empty())
    {
      State.report<core::DiagnosticKind::SemanticInvalidMember>(Node.getSourceRange(), "operator method is missing or private");
      return {};
    }
    const Value *Address = Receiver.TemporaryAddress ? Receiver.TemporaryAddress : materializeClassReceiver(State, *Receiver.ValueObject, Node);
    if (!Address)
    {
      return {};
    }
    std::vector<ExpressionResult> Arguments{{Address}};
    Arguments.insert(Arguments.end(), Operands.begin(), Operands.end());
    std::vector<const parser::Expr *> Nodes(Arguments.size(), &Node);
    return finishCall(State, Candidates, Arguments, Nodes, Node);
  }

  bool Analyzer::resolveMemberFunctions(AnalysisState &State, const parser::MemberExpr &Node, std::vector<const ir::Value *> &Functions, std::size_t Depth)
  {
    if (Node.access() != tokenizer::TokenKind::Dot)
    {
      State.report<core::DiagnosticKind::SemanticModuleMemberExpected>(Node.getSourceRange());
      return false;
    }
    const ExpressionResult Object = analyzeExpr(State, *Node.object(), Depth + 1);
    if (!Object)
    {
      return false;
    }
    if (!Object.ValueObject || !ir::Module::classof(Object.ValueObject))
    {
      State.report<core::DiagnosticKind::SemanticModuleMemberExpected>(Node.getSourceRange());
      return false;
    }
    auto &Module = *const_cast<ir::Module *>(static_cast<const ir::Module *>(Object.ValueObject));
    const auto Member = Node.member();
    const std::string_view ModuleName = State.Context.namePool().text(Module.name());
    const auto *Binding = State.Resolver.lookupMember(Module, State.Context.namePool().find(Member.Text));
    bool Private = false;
    if (Binding)
    {
      for (const ir::Value *Target : Binding->targets())
      {
        if (!ir::Function::classof(Target) || Target->outer() != &Module.entryBlock())
        {
          continue;
        }
        const auto &Function = static_cast<const ir::Function &>(*Target);
        if (State.CurrentModule != &Module && Function.visibility() != core::VisibilityKind::Public)
        {
          Private = true;
          continue;
        }
        Functions.push_back(Target);
        if (State.CurrentModule && State.CurrentModule != &Module)
        {
          State.Context.recordModuleImport(*State.CurrentModule, Function);
        }
      }
    }
    if (!Functions.empty())
    {
      return true;
    }
    if (Private)
    {
      State.report<core::DiagnosticKind::SemanticPrivateImport>(Member.Range, Member.Text, ModuleName);
    }
    else if (Binding || (State.Modules && State.Modules->Modules.contains(std::string(ModuleName)) && moduleHasOtherDeclaration(*State.Modules->Modules.find(std::string(ModuleName))->second->Source->Input->Unit->root(), Member.Text)))
    {
      State.report<core::DiagnosticKind::SemanticUnsupportedImport>(Member.Range);
    }
    else
    {
      State.report<core::DiagnosticKind::SemanticImportNotFound>(Member.Range, ModuleName, Member.Text);
    }
    return false;
  }

  Analyzer::ExpressionResult Analyzer::analyzeMemberExpr(AnalysisState &State, const parser::MemberExpr &Node, std::size_t Depth)
  {
    AnalysisState::EvaluationGuard Expected(State, State.Evaluating, nullptr);
    if (!State.Evaluating && isMutablePlace(State, Node))
    {
      const Value *Address = resolveFieldAddress(State, Node, Depth);
      return {Address ? State.Builder.createLoadInstruction(*Address) : nullptr};
    }
    const ExpressionResult Object = analyzeExpr(State, *Node.object(), Depth + 1);
    if (!Object)
    {
      return {};
    }
    if (Object.ValueObject && ClassType::classof(Object.ValueObject))
    {
      const auto *Binding = State.Resolver.lookupMember(*const_cast<Value *>(Object.ValueObject), State.Context.namePool().find(Node.member().Text));
      if (Node.access() == tokenizer::TokenKind::Dot && Binding && Binding->targets().size() == 1 && ClassType::classof(Binding->targets().front()))
      {
        const auto &Nested = State.Context.classState().Definitions.at(static_cast<const ClassType *>(Binding->targets().front()));
        if (Nested.Declaration->visibility() == parser::DeclarationVisibility::Private && State.CurrentClass != Object.ValueObject)
        {
          State.report<core::DiagnosticKind::SemanticInvalidMember>(Node.member().Range, "nested class is private");
          return {};
        }
        return {Binding->targets().front()};
      }
      State.report<core::DiagnosticKind::SemanticInvalidMember>(Node.member().Range, "class type access requires a nested type");
      return {};
    }
    if (Object.ValueObject && ClassType::classof(&Object.ValueObject->type()))
    {
      if (Node.access() != tokenizer::TokenKind::Dot)
      {
        State.report<core::DiagnosticKind::SemanticInvalidMember>(Node.getSourceRange(), "class values use dot member access");
        return {};
      }
      const auto Index = lookupClassField(State, static_cast<const ClassType &>(Object.ValueObject->type()), Node);
      if (!Index)
      {
        return {};
      }
      if (ClassConstant::classof(Object.ValueObject))
      {
        return {static_cast<const ClassConstant *>(Object.ValueObject)->fields()[*Index]};
      }
      return {State.Builder.createFieldExtractInstruction(*Object.ValueObject, *Index)};
    }
    if (!Object.ValueObject || !Module::classof(Object.ValueObject))
    {
      State.report<core::DiagnosticKind::SemanticInvalidMember>(Node.getSourceRange(), "member access requires a class value, class pointer or module");
      return {};
    }
    const auto *TypeBinding = State.Resolver.lookupMember(*const_cast<Value *>(Object.ValueObject), State.Context.namePool().find(Node.member().Text));
    if (TypeBinding && TypeBinding->targets().size() == 1 && ClassType::classof(TypeBinding->targets().front()))
    {
      const auto &Class = static_cast<const ClassType &>(*TypeBinding->targets().front());
      const auto &Definition = State.Context.classState().Definitions.at(&Class);
      if (Definition.Module != Object.ValueObject)
      {
        State.report<core::DiagnosticKind::SemanticUnsupportedImport>(Node.member().Range);
        return {};
      }
      if (State.CurrentModule != Definition.Module && Definition.Declaration->visibility() == parser::DeclarationVisibility::Private)
      {
        State.report<core::DiagnosticKind::SemanticInvalidMember>(Node.member().Range, "class is private");
        return {};
      }
      return {&Class};
    }
    std::vector<const ir::Value *> Functions;
    if (!resolveMemberFunctions(State, Node, Functions, Depth))
    {
      return {};
    }
    if (Functions.size() != 1)
    {
      State.report<core::DiagnosticKind::SemanticAmbiguousName>(Node.getSourceRange());
      return {};
    }
    const auto *Function = Functions.front();
    if (State.Modules && State.CurrentFunction)
    {
      State.Modules->Dependencies[State.CurrentFunction].insert(static_cast<const ir::Function *>(Function));
    }
    const auto &Definitions = State.Context.comptimeState().Functions;
    if (const auto Found = Definitions.find(Function); !State.Evaluating && !State.ComptimeFunction && Found != Definitions.end() && Found->second.Comptime)
    {
      State.report<core::DiagnosticKind::SemanticComptimeFunctionAtRuntime>(Node.getSourceRange());
      return {};
    }
    return {Function};
  }
} // namespace ink::semantic
