#include "../analyzer_internal.h"

#include "ink/execution/ffi/ffi_type.h"
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
    for (const parser::Attribute &Attribute : Node.attributes())
    {
      if (Attribute.path().size() != 1 || Attribute.path()[0].Text != "abi")
      {
        State.report<core::DiagnosticKind::SemanticUnsupportedFunctionAttribute>(Attribute.range());
        return std::nullopt;
      }
      if (LinkageNode)
      {
        State.report<core::DiagnosticKind::SemanticDuplicateAbi>(Attribute.range());
        return std::nullopt;
      }
      if (Attribute.suffix() != parser::AttributeSuffix::Arguments || Attribute.arguments().size() != 1 || Attribute.arguments()[0].form() != parser::ArgumentKind::Positional)
      {
        State.report<core::DiagnosticKind::SemanticInvalidAbiAttribute>(Attribute.range());
        return std::nullopt;
      }
      LinkageNode = Attribute.arguments()[0].value();
    }
    if (!LinkageNode)
    {
      return LanguageLinkage::Ink;
    }
    if (LinkageNode->isComptime() || !parser::LiteralExpr::classof(LinkageNode))
    {
      State.report<core::DiagnosticKind::SemanticInvalidAbiAttribute>(LinkageNode->getSourceRange());
      return std::nullopt;
    }
    const auto &Literal = static_cast<const parser::LiteralExpr &>(*LinkageNode);
    const auto *Text = std::get_if<tokenizer::StringInfo>(&State.Input.token(Literal.token()).Payload);
    if (Literal.literalKind() != tokenizer::TokenKind::StringLiteral || !Text)
    {
      State.report<core::DiagnosticKind::SemanticInvalidAbiAttribute>(LinkageNode->getSourceRange());
      return std::nullopt;
    }
    if (Text->Decoded != "C")
    {
      State.report<core::DiagnosticKind::SemanticUnsupportedAbi>(LinkageNode->getSourceRange());
      return std::nullopt;
    }
    return LanguageLinkage::C;
  }

  bool Analyzer::checkFunctionConflicts(AnalysisState &State, const parser::FunctionDecl &Node, const FunctionType &Signature, FunctionBinding NativeBinding)
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
        if (SameParameters || NativeBinding != FunctionBinding::Local || Existing.binding() != FunctionBinding::Local)
        {
          State.report<core::DiagnosticKind::SemanticDuplicateName>(Node.name().Range, Node.name().Text);
          return false;
        }
      }
    }
    return true;
  }

  std::unique_ptr<Function> Analyzer::declareFunction(AnalysisState &State, const parser::FunctionDecl &Node)
  {
    if (State.Evaluating)
    {
      reportExecution(State, execution::ExecutionStatus::UnsupportedOperation, Node);
      return nullptr;
    }
    if (!Node.genericParameters().empty())
    {
      reportUnsupported(State, Node);
      return nullptr;
    }

    const std::optional<LanguageLinkage> Linkage = analyzeFunctionLinkage(State, Node);
    if (!Linkage)
    {
      return nullptr;
    }
    const FunctionBinding Binding = Node.nativeSymbolKind() == parser::NativeSymbolKind::Import ? FunctionBinding::Import : (Node.nativeSymbolKind() == parser::NativeSymbolKind::Export ? FunctionBinding::Export : FunctionBinding::Local);
    if (Binding == FunctionBinding::Import && Node.body())
    {
      State.report<core::DiagnosticKind::SemanticNativeImportHasBody>(Node.getSourceRange(), Node.name().Text);
      return nullptr;
    }
    if (Binding == FunctionBinding::Export && !Node.body())
    {
      State.report<core::DiagnosticKind::SemanticNativeExportRequiresBody>(Node.getSourceRange(), Node.name().Text);
      return nullptr;
    }
    const bool Local = State.CurrentFunction || State.BlockDepth != 0;
    if (Binding == FunctionBinding::Export && Local)
    {
      State.report<core::DiagnosticKind::SemanticNativeExportRequiresTopLevel>(Node.getSourceRange());
      return nullptr;
    }
    if (Node.isComptime() && !Node.body())
    {
      State.report<core::DiagnosticKind::SemanticComptimeFunctionRequiresBody>(Node.getSourceRange());
      return nullptr;
    }
    if (Node.isComptime() && *Linkage != LanguageLinkage::Ink)
    {
      State.report<core::DiagnosticKind::SemanticComptimeFunctionLinkage>(Node.getSourceRange());
      return nullptr;
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
      return nullptr;
    }

    if (*Linkage == LanguageLinkage::C && Binding != FunctionBinding::Import)
    {
      if (!execution::ffiType(*ReturnType, execution::FfiTypeUsage::Return) || std::any_of(ParameterTypes.begin(), ParameterTypes.end(), [](const Type *Parameter)
      {
        return !execution::ffiType(*Parameter, execution::FfiTypeUsage::Argument);
      }))
      {
        State.report<core::DiagnosticKind::SemanticUnsupportedAbiSignature>(Node.getSourceRange(), Node.name().Text);
        return nullptr;
      }
    }

    const FunctionType *Signature = State.Context.typePool().getType<TypeKind::Function>(*ReturnType, ParameterTypes);
    if (!Signature)
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return nullptr;
    }
    if (!checkFunctionConflicts(State, Node, *Signature, Binding))
    {
      return nullptr;
    }

    if (Binding != FunctionBinding::Import && !Node.body())
    {
      State.report<core::DiagnosticKind::SemanticFunctionRequiresBody>(Node.getSourceRange(), Node.name().Text);
      return nullptr;
    }
    if (Local && Node.visibility() == parser::DeclarationVisibility::Public)
    {
      State.report<core::DiagnosticKind::SemanticPublicLocal>(Node.getSourceRange(), Node.name().Text);
      return nullptr;
    }

    const Name FunctionName = State.Context.namePool().intern(Node.name().Text);
    auto FunctionOwner = State.Builder.createFunction(FunctionName, *Signature, {}, ParameterNames, CallingConvention::C, *Linkage, Binding);
    if (!FunctionOwner || !State.Builder.insertBlock())
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return nullptr;
    }
    Function &FunctionValue = *FunctionOwner;
    if (!State.Builder.setFunctionVisibility(FunctionValue, Local || Node.visibility() == parser::DeclarationVisibility::Private ? VisibilityKind::Private : VisibilityKind::Public))
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return nullptr;
    }
    // Bind before checking the body so recursive lookup can see this function.
    // The detached owner removes all bindings and child values on any failure.
    if (State.Resolver.bind(FunctionName, FunctionValue) != NameResolver::BindResult::Inserted)
    {
      State.report<core::DiagnosticKind::SemanticDuplicateName>(Node.name().Range, Node.name().Text);
      return nullptr;
    }
    State.Context.comptimeState().Functions[&FunctionValue] = {Node.isComptime(), State.Source, Node.name().Range};
    return FunctionOwner;
  }

  bool Analyzer::analyzeFunctionBody(AnalysisState &State, const parser::FunctionDecl &Node, Function &FunctionValue)
  {
    struct ActiveBodyGuard
    {
        ModuleGraph *Graph;
        const Function *Value;

        ~ActiveBodyGuard()
        {
          if (Graph)
          {
            Graph->ActiveBodies.erase(Value);
          }
        }
    };
    const ActiveBodyGuard Active{State.Modules, &FunctionValue};
    if (State.Modules)
    {
      State.Modules->ActiveBodies.insert(&FunctionValue);
    }
    AnalysisState FunctionState(State.Context, State.Resolver.currentScope(), State.Input);
    FunctionState.CurrentModule = State.CurrentModule;
    FunctionState.Modules = State.Modules;
    FunctionState.Frame = State.Frame;
    AnalysisState::FrameGuard Frame(FunctionState, execution::ExecutionFrameKind::Analysis);
    if (!Frame)
    {
      return reportExecution(State, State.Context.comptimeState().Engine.lastStatus(), Node);
    }
    FunctionState.BlockDepth = State.BlockDepth;
    FunctionState.CurrentFunction = &FunctionValue;
    FunctionState.ComptimeFunction = Node.isComptime();
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
    // Every non-generic function is checked and lowered when it is defined.
    // Compile-time calls execute this same IR instead of revisiting its AST.
    if (Node.body())
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
        if (FunctionValue.functionType().returnType().typeKind() != TypeKind::Void)
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
    return true;
  }

  bool Analyzer::analyzeFunctionDecl(AnalysisState &State, const parser::FunctionDecl &Node)
  {
    auto FunctionOwner = declareFunction(State, Node);
    if (!FunctionOwner || !analyzeFunctionBody(State, Node, *FunctionOwner))
    {
      return false;
    }
    if (!State.Builder.appendValue(*State.Builder.insertBlock(), std::move(FunctionOwner)))
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    return true;
  }
} // namespace ink::semantic
