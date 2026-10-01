#include "../analyzer_internal.h"

#include "ink/parser/ast.h"

#include <algorithm>
#include <unordered_set>
#include <variant>
#include <vector>

namespace ink::semantic
{
  using namespace ink::ir;

  std::optional<LanguageLinkage> Analyzer::analyzeFunctionLinkage(AnalysisState &State, const parser::FunctionDecl &Node)
  {
    const parser::Expr *LinkageNode = Node.linkage();
    if (!LinkageNode)
    {
      return LanguageLinkage::Ink;
    }
    if (LinkageNode->isComptime() || !parser::LiteralExpr::classof(LinkageNode))
    {
      reportUnsupported(State, *LinkageNode);
      return std::nullopt;
    }
    const auto &Literal = static_cast<const parser::LiteralExpr &>(*LinkageNode);
    const auto *Text = std::get_if<tokenizer::StringInfo>(&State.Input.token(Literal.token()).Payload);
    if (!Text || Text->Decoded != "C")
    {
      reportUnsupported(State, *LinkageNode);
      return std::nullopt;
    }
    return LanguageLinkage::C;
  }

  bool Analyzer::checkFunctionConflicts(AnalysisState &State, const parser::FunctionDecl &Node, const FunctionType &Signature, LanguageLinkage Linkage)
  {
    const Name FunctionName = State.Context.namePool().find(Node.name().Text);
    const auto ParameterTypes = Signature.parameterTypes();
    if (const auto *Binding = State.Resolver.lookupLocal(FunctionName))
    {
      for (const Value *Target : Binding->targets())
      {
        if (!Function::classof(Target))
        {
          State.report<core::DiagnosticKind::SemanticDuplicateName>(Node.name().Range, Node.name().Text);
          return false;
        }
        const auto &Existing = static_cast<const Function &>(*Target);
        const auto ExistingTypes = Existing.functionType().parameterTypes();
        const bool SameParameters = std::equal(ParameterTypes.begin(), ParameterTypes.end(), ExistingTypes.begin(), ExistingTypes.end());
        const auto Definition = State.Context.comptimeState().Functions.find(&Existing);
        const bool HasDefinition = Existing.hasBody() || (Definition != State.Context.comptimeState().Functions.end() && Definition->second.AST->body());
        // Compatible redeclarations must agree on language linkage as well as types.
        // Linkage is function metadata, so canonical FunctionType identity alone is insufficient.
        if (SameParameters && &Existing.functionType().returnType() == &Signature.returnType() && Existing.languageLinkage() == Linkage && (!HasDefinition || !Node.body()))
        {
          State.report<core::DiagnosticKind::SemanticUnsupported>(Node.getSourceRange(), "function redeclarations");
          return false;
        }
        if (SameParameters || Linkage == LanguageLinkage::C || Existing.languageLinkage() == LanguageLinkage::C)
        {
          State.report<core::DiagnosticKind::SemanticDuplicateName>(Node.name().Range, Node.name().Text);
          return false;
        }
      }
    }
    return true;
  }

  bool Analyzer::analyzeFunctionDecl(AnalysisState &State, const parser::FunctionDecl &Node)
  {
    if (State.Evaluating)
    {
      return reportExecution(State, execution::ExecutionStatus::UnsupportedOperation, Node);
    }
    if (!Node.genericParameters().empty() || !Node.attributes().empty())
    {
      return reportUnsupported(State, Node);
    }

    const std::optional<LanguageLinkage> Linkage = analyzeFunctionLinkage(State, Node);
    if (!Linkage)
    {
      return false;
    }
    if (Node.isComptime() && !Node.body())
    {
      State.report<core::DiagnosticKind::SemanticComptimeFunctionRequiresBody>(Node.getSourceRange());
      return false;
    }
    if (Node.isComptime() && *Linkage != LanguageLinkage::Ink)
    {
      State.report<core::DiagnosticKind::SemanticComptimeFunctionLinkage>(Node.getSourceRange());
      return false;
    }

    bool Succeeded = true;
    std::vector<const Type *> ParameterTypes;
    std::vector<Name> ParameterNames;
    std::unordered_set<Name> SeenNames;
    for (const parser::Parameter &Parameter : Node.parameters())
    {
      const Name ParameterName = State.Context.namePool().intern(Parameter.name().Text);
      ParameterNames.push_back(ParameterName);
      if (!SeenNames.insert(ParameterName).second)
      {
        State.report<core::DiagnosticKind::SemanticDuplicateParameterName>(Parameter.name().Range, Parameter.name().Text);
        Succeeded = false;
      }
      if (Parameter.defaultValue() || Parameter.variadic())
      {
        State.report<core::DiagnosticKind::SemanticUnsupported>(Parameter.range(), Parameter.variadic() ? "variadic parameters" : "default arguments");
        Succeeded = false;
      }
      const Type *ParameterType = analyzeType(State, *Parameter.type()->expression());
      ParameterTypes.push_back(ParameterType);
      if (!ParameterType)
      {
        Succeeded = false;
      }
      else if (ParameterType->typeKind() == TypeKind::Void || ParameterType->typeKind() == TypeKind::Meta)
      {
        State.report<core::DiagnosticKind::SemanticTypeMismatch>(Parameter.type()->getSourceRange(), "runtime parameter type", ParameterType->typeKind() == TypeKind::Void ? "void" : "type");
        Succeeded = false;
      }
    }
    const Type *ReturnType = analyzeType(State, *Node.returnType()->expression());
    if (!ReturnType)
    {
      Succeeded = false;
    }
    else if (ReturnType->typeKind() == TypeKind::Meta)
    {
      State.report<core::DiagnosticKind::SemanticUnsupported>(Node.returnType()->getSourceRange(), "type-valued function returns");
      Succeeded = false;
    }
    if (!Succeeded)
    {
      return false;
    }

    const FunctionType *Signature = State.Context.typePool().getType<TypeKind::Function>(*ReturnType, ParameterTypes);
    if (!Signature)
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    if (!checkFunctionConflicts(State, Node, *Signature, *Linkage))
    {
      return false;
    }

    const Name FunctionName = State.Context.namePool().intern(Node.name().Text);
    auto FunctionOwner = State.Builder.createFunction(FunctionName, *Signature, {}, ParameterNames, CallingConvention::C, *Linkage);
    if (!FunctionOwner || !State.Builder.insertBlock())
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    Function &FunctionValue = *FunctionOwner;
    // Bind before checking the body so recursive lookup can see this function.
    // The detached owner removes all bindings and child values on any failure.
    if (State.Resolver.bind(FunctionName, FunctionValue) != NameResolver::BindResult::Inserted)
    {
      State.report<core::DiagnosticKind::SemanticDuplicateName>(Node.name().Range, Node.name().Text);
      return false;
    }
    AnalysisState FunctionState(State.Context, State.Resolver.currentScope(), State.Input);
    FunctionState.Frame = State.Frame;
    AnalysisState::FrameGuard Frame(FunctionState, execution::ExecutionFrameKind::Analysis);
    if (!Frame)
    {
      return reportExecution(State, State.Context.comptimeState().Engine.lastStatus(), Node);
    }
    FunctionState.BlockDepth = State.BlockDepth;
    FunctionState.CurrentFunction = &FunctionValue;
    Scope *DefinitionScope = State.Context.scopeStore().snapshotScope(State.Resolver.currentScope());
    if (!DefinitionScope)
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    State.Context.comptimeState().Functions[&FunctionValue] = {&Node, &State.Input, DefinitionScope, State.Frame, Node.isComptime()};
    NameResolver::ScopeGuard FunctionScope(FunctionState.Resolver, FunctionValue);
    if (!FunctionScope.scope())
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    for (const auto &Parameter : FunctionValue.parameters())
    {
      if (FunctionState.Resolver.bind(Parameter->name(), *Parameter) != NameResolver::BindResult::Inserted)
      {
        State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
        return false;
      }
    }
    // Compile-time-only bodies are checked on their actual execution path with
    // bound argument values. Their callable identity has no runtime IR body.
    if (Node.body() && !Node.isComptime())
    {
      BasicBlock *Body = FunctionState.Builder.createFunctionBody(FunctionValue);
      if (!Body || !FunctionState.Builder.setInsertPoint(*Body))
      {
        State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
        return false;
      }
      if (!analyzeStmt(FunctionState, *Node.body()))
      {
        return false;
      }
      if (!FunctionState.Terminated)
      {
        if (ReturnType->typeKind() != TypeKind::Void)
        {
          State.report<core::DiagnosticKind::SemanticMissingReturn>(Node.body()->getSourceRange(), Node.name().Text);
          return false;
        }
        if (!FunctionState.Builder.createReturnInstruction())
        {
          State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.body()->getSourceRange());
          return false;
        }
      }
    }
    if (!State.Builder.appendValue(*State.Builder.insertBlock(), std::move(FunctionOwner)))
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    return true;
  }
} // namespace ink::semantic
