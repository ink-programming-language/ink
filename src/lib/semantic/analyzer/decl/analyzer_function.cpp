#include "../analyzer_internal.h"

#include "ink/execution/ffi/ffi_type.h"
#include "ink/ir/analysis/type_layout.h"
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

  bool Analyzer::checkFunctionConflicts(AnalysisState &State, const parser::FunctionDecl &Node, const FunctionType &Signature, core::FunctionBinding NativeBinding)
  {
    const Name FunctionName = State.Context.namePool().find(Node.name().Text);
    if (const auto *Declarations = State.Resolver.lookupLocal<Decl *>(FunctionName); Declarations && (NativeBinding != core::FunctionBinding::Local || !Declarations->isOverloadSet()))
    {
      State.report<core::DiagnosticKind::SemanticDuplicateName>(Node.name().Range, Node.name().Text);
      return false;
    }
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
        if (SameParameters || NativeBinding != core::FunctionBinding::Local || Existing.binding() != core::FunctionBinding::Local)
        {
          State.report<core::DiagnosticKind::SemanticDuplicateName>(Node.name().Range, Node.name().Text);
          return false;
        }
      }
    }
    return true;
  }

  std::unique_ptr<Function> Analyzer::declareFunction(AnalysisState &State, const parser::FunctionDecl &Node, bool Instance)
  {
    if (State.CurrentClass && Node.name().Text == "this")
    {
      State.report<core::DiagnosticKind::SemanticDuplicateName>(Node.name().Range, Node.name().Text);
      return nullptr;
    }
    if (State.Evaluating)
    {
      reportExecution(State, execution::ExecutionStatus::UnsupportedOperation, Node);
      return nullptr;
    }
    if (!Node.genericParameters().empty() && !Instance)
    {
      State.report<core::DiagnosticKind::SemanticInvalidGeneric>(Node.getSourceRange(), "generic instance methods are not supported");
      return nullptr;
    }

    const std::optional<LanguageLinkage> Linkage = analyzeFunctionLinkage(State, Node);
    if (!Linkage)
    {
      return nullptr;
    }
    const core::FunctionBinding Binding = Node.nativeSymbolKind();
    if (State.DeclaringClass && (*Linkage != LanguageLinkage::Ink || Binding != core::FunctionBinding::Local || Node.name().Text == "this"))
    {
      State.report<core::DiagnosticKind::SemanticInvalidClass>(Node.getSourceRange(), "instance methods require Ink linkage and cannot be named this");
      return nullptr;
    }
    if (Binding == core::FunctionBinding::Import && Node.body())
    {
      State.report<core::DiagnosticKind::SemanticNativeImportHasBody>(Node.getSourceRange(), Node.name().Text);
      return nullptr;
    }
    if (Binding == core::FunctionBinding::Export && !Node.body())
    {
      State.report<core::DiagnosticKind::SemanticNativeExportRequiresBody>(Node.getSourceRange(), Node.name().Text);
      return nullptr;
    }
    const bool Local = State.CurrentFunction || State.BlockDepth != 0;
    if (Binding == core::FunctionBinding::Export && Local)
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
    if (State.DeclaringClass)
    {
      const Name ReceiverName = State.Context.namePool().intern("this");
      ParameterNames.push_back(ReceiverName);
      ParameterTypes.push_back(State.Context.typePool().getType<TypeKind::Pointer>(*State.DeclaringClass, AccessKind::ReadWrite));
      SeenNames.insert(ReceiverName);
    }
    for (const parser::Parameter &Parameter : Node.parameters())
    {
      const Name ParameterName = State.Context.namePool().intern(Parameter.name().Text);
      ParameterNames.push_back(ParameterName);
      if (!SeenNames.insert(ParameterName).second || (State.CurrentClass && !State.DeclaringClass && Parameter.name().Text == "this"))
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
      else if ((ClassType::classof(ParameterType) || ArrayType::classof(ParameterType)) && !computeTypeLayout(*ParameterType, State.Context.compilationContext().targetContext()))
      {
        State.report<core::DiagnosticKind::SemanticInvalidClass>(Parameter.type()->getSourceRange(), "by-value parameters require a complete finite layout");
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
    else if ((ClassType::classof(ReturnType) || ArrayType::classof(ReturnType)) && !computeTypeLayout(*ReturnType, State.Context.compilationContext().targetContext()))
    {
      State.report<core::DiagnosticKind::SemanticInvalidClass>(Node.returnType()->getSourceRange(), "by-value returns require a complete finite layout");
      Succeeded = false;
    }
    if (!Succeeded)
    {
      return nullptr;
    }
    if (State.DeclaringClass && (Node.name().Text == "__init__" || Node.name().Text == "__del__"))
    {
      if (ReturnType->typeKind() != TypeKind::Void || Node.isComptime() || (Node.name().Text == "__del__" && (ParameterTypes.size() != 1 || Node.visibility() == parser::DeclarationVisibility::Private)))
      {
        State.report<core::DiagnosticKind::SemanticInvalidClass>(Node.getSourceRange(), "__init__ and __del__ must return void and support runtime calls; __del__ must be public and have no explicit parameters");
        return nullptr;
      }
    }

    if (*Linkage == LanguageLinkage::C && Binding != core::FunctionBinding::Import)
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
    if (!Instance && !checkFunctionConflicts(State, Node, *Signature, Binding))
    {
      return nullptr;
    }

    if (Binding != core::FunctionBinding::Import && !Node.body())
    {
      State.report<core::DiagnosticKind::SemanticFunctionRequiresBody>(Node.getSourceRange(), Node.name().Text);
      return nullptr;
    }
    if (Local && Node.visibility() == parser::DeclarationVisibility::Public)
    {
      State.report<core::DiagnosticKind::SemanticPublicLocal>(Node.getSourceRange(), Node.name().Text);
      return nullptr;
    }

    const Name BoundName = State.Context.namePool().intern(Node.name().Text);
    const Name FunctionName = BoundName;
    auto FunctionOwner = State.Builder.createFunction(FunctionName, *Signature, {}, ParameterNames, CallingConvention::C, *Linkage, Binding);
    if (!FunctionOwner || !State.Builder.insertBlock())
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return nullptr;
    }
    Function &FunctionValue = *FunctionOwner;
    if (!State.Builder.setFunctionLexicalScope(FunctionValue, State.LexicalScope))
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return nullptr;
    }
    if (!State.Builder.setFunctionVisibility(FunctionValue, Local || Node.visibility() == parser::DeclarationVisibility::Private ? core::VisibilityKind::Private : core::VisibilityKind::Public))
    {
      State.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return nullptr;
    }
    // Bind before checking the body so recursive lookup can see this function.
    // The detached owner removes all bindings and child values on any failure.
    if (!Instance && State.Resolver.bind(BoundName, FunctionValue) != NameResolver::BindResult::Inserted)
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
    FunctionState.CurrentClass = State.CurrentClass;
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
    const bool ClassMethod = State.Context.classState().MethodOwners.contains(&FunctionValue);
    FunctionState.Constructing = ClassMethod && Node.name().Text == "__init__";
    FunctionState.Destroying = ClassMethod && Node.name().Text == "__del__";
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
      for (const auto &Parameter : FunctionValue.parameters())
      {
        if (!needsDestruction(FunctionState, Parameter->type()))
        {
          continue;
        }
        auto *Storage = FunctionState.Builder.createAllocaInstruction(Parameter->type());
        if (!Storage || !FunctionState.Builder.createStoreInstruction(*Storage, *Parameter) || !trackObject(FunctionState, *Storage, Parameter->type(), true))
        {
          return false;
        }
      }
      if (FunctionState.Constructing && !initializeClassFields(FunctionState, Node))
      {
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
        if (!checkConstructorComplete(FunctionState, Node) || !cleanupObjects(FunctionState, 0, false, Node) || !destroyClassFields(FunctionState, Node))
        {
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
    if (!Node.genericParameters().empty())
    {
      return registerGenericFunction(State, Node);
    }
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
