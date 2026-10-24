#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/parser.h"
#include "ink/ir/function/function.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analyzer_state.h"
#include "ink/semantic/analyzer/analyzer_utils.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/scope_guard.h"

#include <unordered_set>
#include <variant>
#include <vector>

namespace ink::semantic
{
  namespace
  {
    class InsertPointGuard final
    {
      public:
        explicit InsertPointGuard(ir::IRBuilder &Builder) noexcept
            : Builder(Builder),
              SavedBlock(Builder.insertBlock())
        {
        }

        ~InsertPointGuard() noexcept
        {
          if (SavedBlock)
          {
            (void)Builder.setInsertPoint(*SavedBlock);
          }
          else
          {
            Builder.clearInsertPoint();
          }
        }

        InsertPointGuard(const InsertPointGuard &) = delete;
        InsertPointGuard &operator=(const InsertPointGuard &) = delete;

      private:
        ir::IRBuilder &Builder;
        ir::BasicBlock *SavedBlock;
    };

    bool sameParameterTypes(const ir::FunctionType &Left, const ir::FunctionType &Right) noexcept
    {
      RETURN_IF(Left.parameterCount() != Right.parameterCount(), false);
      for (std::size_t Index = 0; Index < Left.parameterCount(); ++Index)
      {
        const auto &LeftParameter = Left.parameters()[Index];
        const auto &RightParameter = Right.parameters()[Index];
        RETURN_IF(LeftParameter.ParameterType != RightParameter.ParameterType || LeftParameter.Variadic != RightParameter.Variadic, false);
      }
      return true;
    }

    bool registerFunctionExport(const AnalysisSite &Site, ir::Module &Output, const ir::Function &Function)
    {
      RETURN_IF(Function.visibility() != ir::FunctionVisibility::Public, true);
      const auto Export = Output.addExport(ir::SymbolId{Function.name()}, Function);
      CHECK_ICE(Site, Export, core::DiagnosticKind::SemanticConstructionFailed);
      return true;
    }
  } // namespace

  bool Analyzer::analyzeFunctionDecl(AnalyzerState &State, const frontend::parser::FunctionDecl &Node)
  {
    const auto Site = State.at(Node);
    auto &Context = State.semanticContext();
    auto &Resolver = State.nameResolver();
    CHECK_ICE(Site, Node.nativeSymbolKind() == frontend::parser::NativeSymbolKind::None || Node.nativeSymbolKind() == frontend::parser::NativeSymbolKind::Import || Node.nativeSymbolKind() == frontend::parser::NativeSymbolKind::Export, core::DiagnosticKind::SemanticInvalidAST, "FunctionDecl native symbol kind must be None, Import or Export");
    CHECK_ICE(Site, Node.bodyKind() == frontend::parser::FunctionBodyKind::Definition || Node.bodyKind() == frontend::parser::FunctionBodyKind::DeclarationOnly, core::DiagnosticKind::SemanticInvalidAST, "FunctionDecl body kind must be DeclarationOnly or Definition");
    CHECK_ICE(Site, Node.visibility() == frontend::parser::DeclarationVisibility::Default || Node.visibility() == frontend::parser::DeclarationVisibility::Public || Node.visibility() == frontend::parser::DeclarationVisibility::Private, core::DiagnosticKind::SemanticInvalidAST, "FunctionDecl visibility must be Default, Public or Private");
    const bool HasBody = Node.bodyKind() == frontend::parser::FunctionBodyKind::Definition;
    CHECK_ICE(Site, !HasBody || Node.body(), core::DiagnosticKind::SemanticInvalidAST, "FunctionDecl definition requires a body");
    CHECK_ICE(Site, HasBody || !Node.body(), core::DiagnosticKind::SemanticInvalidAST, "FunctionDecl declaration-only form must not have a body");
    CHECK_ICE(Site, Node.returnType(), core::DiagnosticKind::SemanticInvalidAST, "FunctionDecl requires a return type annotation");
    CHECK_ICE(Site, !Node.name().Text.empty(), core::DiagnosticKind::SemanticInvalidAST, "FunctionDecl requires a name");
    // TODO: Define ownership and capture semantics for local and nested functions.
    CHECK_ICE(Site, Context.scopeStore().memberScope(State.Output) == &Resolver.currentScope(), core::DiagnosticKind::SemanticUnsupported, frontend::parser::astKindName(Node.getKind()));
    // TODO: Analyze generic parameters and instantiate generic function signatures and bodies.
    CHECK_ICE(Site, Node.genericParameters().empty(), core::DiagnosticKind::SemanticUnsupported, frontend::parser::astKindName(Node.getKind()));
    // TODO: Validate function attributes and lower their supported semantics.
    CHECK_ICE(Site, Node.attributes().empty(), core::DiagnosticKind::SemanticUnsupported, frontend::parser::astKindName(Node.getKind()));
    for (const auto &Parameter : Node.parameters())
    {
      const auto ParameterSite = State.at(Parameter.range());
      CHECK_ICE(ParameterSite, !Parameter.name().Text.empty(), core::DiagnosticKind::SemanticInvalidAST, "FunctionDecl parameter requires a name");
      CHECK_ICE(ParameterSite, Parameter.type(), core::DiagnosticKind::SemanticInvalidAST, "FunctionDecl parameter requires a type annotation");
      // TODO: Analyze and retain default arguments for call-site argument completion.
      CHECK_ICE(Site, !Parameter.defaultValue(), core::DiagnosticKind::SemanticUnsupported, frontend::parser::astKindName(Node.getKind()));
      // TODO: Lower variadic parameters and parameter-pack operands.
      CHECK_ICE(Site, !Parameter.variadic(), core::DiagnosticKind::SemanticUnsupported, frontend::parser::astKindName(Node.getKind()));
    }

    ir::SymbolLinkage SymbolLink = ir::SymbolLinkage::Local;
    ir::LanguageLinkage LanguageLink = ir::LanguageLinkage::Ink;
    if (Node.nativeSymbolKind() != frontend::parser::NativeSymbolKind::None)
    {
      const auto *Linkage = Node.linkage();
      CHECK_ICE(Site, Linkage, core::DiagnosticKind::SemanticInvalidAST, "FunctionDecl native import/export requires a linkage expression");
      const auto LinkageSite = State.at(*Linkage);
      CHECK_ICE(LinkageSite, frontend::parser::LiteralExpr::classof(Linkage), core::DiagnosticKind::SemanticInvalidAST, "FunctionDecl linkage must be a string literal");
      const auto &Literal = static_cast<const frontend::parser::LiteralExpr &>(*Linkage);
      CHECK_ICE(LinkageSite, Literal.literalKind() == frontend::parser::TokenKind::StringLiteral, core::DiagnosticKind::SemanticInvalidAST, "FunctionDecl linkage must be a string literal");
      const auto &Token = State.input().requireToken(Literal.token(), Context.diagnosticEngine(), Linkage->getSourceRange());
      const auto *String = std::get_if<frontend::tokenizer::StringInfo>(&Token.Payload);
      CHECK_ICE(LinkageSite, Token.Kind == frontend::parser::TokenKind::StringLiteral && String, core::DiagnosticKind::SemanticInvalidAST, "FunctionDecl linkage literal must reference a string token with StringInfo metadata");
      // TODO: Resolve and lower additional native language linkages.
      CHECK_ICE(Site, String->Decoded == "C", core::DiagnosticKind::SemanticUnsupported, frontend::parser::astKindName(Node.getKind()));
      SymbolLink = Node.nativeSymbolKind() == frontend::parser::NativeSymbolKind::Import ? ir::SymbolLinkage::Import : ir::SymbolLinkage::Export;
      LanguageLink = ir::LanguageLinkage::C;
    }
    else
    {
      CHECK_ICE(Site, !Node.linkage(), core::DiagnosticKind::SemanticInvalidAST, "FunctionDecl without native import/export must not have a linkage expression");
    }
    RETURN_ERROR_IF(Site, SymbolLink == ir::SymbolLinkage::Import && HasBody, core::DiagnosticKind::SemanticNativeImportHasBody, false, Node.name().Text);
    RETURN_ERROR_IF(Site, !HasBody && SymbolLink != ir::SymbolLinkage::Import, core::DiagnosticKind::SemanticFunctionRequiresBody, false, Node.name().Text);

    const ir::Type *ReturnType = analyzeTypeWithLayout(State, *Node.returnType(), "a return type with runtime layout");
    RETURN_IF_NOT(ReturnType, false);
    std::vector<ir::FunctionTypeParameter> Parameters;
    std::vector<core::Name> ParameterNames;
    std::unordered_set<core::Name> UniqueParameterNames;
    Parameters.reserve(Node.parameters().size());
    ParameterNames.reserve(Node.parameters().size());
    for (const auto &Parameter : Node.parameters())
    {
      const auto Name = core::NamePool::instance().intern(Parameter.name().Text);
      CHECK_ICE(State.at(Parameter.range()), Name.valid(), core::DiagnosticKind::SemanticConstructionFailed);
      RETURN_ERROR_IF_NOT(State.at(Parameter.name().Range), UniqueParameterNames.insert(Name).second, core::DiagnosticKind::SemanticDuplicateParameterName, false, Parameter.name().Text);
      const ir::Type *Type = analyzeTypeWithLayout(State, *Parameter.type(), "a parameter type with runtime layout");
      RETURN_IF_NOT(Type, false);
      Parameters.emplace_back(Type);
      ParameterNames.push_back(Name);
    }
    const ir::FunctionType *Signature = Context.typePool().get<ir::FunctionType>(ReturnType, Parameters);
    const auto Name = core::NamePool::instance().intern(Node.name().Text);
    CHECK_ICE(Site, Signature, core::DiagnosticKind::SemanticConstructionFailed);
    CHECK_ICE(Site, Name.valid(), core::DiagnosticKind::SemanticConstructionFailed);
    if (const Binding *Existing = Resolver.lookupLocal(Name))
    {
      const auto NameSite = State.at(Node.name().Range);
      RETURN_ERROR_IF_NOT(NameSite, Existing->isOverloadSet(), core::DiagnosticKind::SemanticDuplicateName, false, Node.name().Text);
      for (const ir::Value *Target : Existing->targets())
      {
        CHECK_ICE(Site, ir::Function::classof(Target), core::DiagnosticKind::SemanticConstructionFailed);
        const auto &ExistingFunction = static_cast<const ir::Function &>(*Target);
        // TODO: Merge compatible native redeclarations once declaration identity and definition tracking exist.
        RETURN_ERROR_IF(NameSite, LanguageLink == ir::LanguageLinkage::C || ExistingFunction.languageLinkage() == ir::LanguageLinkage::C || sameParameterTypes(*Signature, *ExistingFunction.functionType()), core::DiagnosticKind::SemanticDuplicateName, false, Node.name().Text);
      }
    }

    const auto Visibility = Node.visibility() == frontend::parser::DeclarationVisibility::Private ? ir::FunctionVisibility::Private : ir::FunctionVisibility::Public;
    // TODO: Support function definitions in nested scopes with appropriate ownership and lexical bindings.
    ir::Function *Function = State.Output.createGlobalFunction(*Signature, Name, SymbolLink, LanguageLink, Visibility);
    CHECK_ICE(Site, Function && Resolver.bindOverload(Name, *Function) == BindResult::Inserted, core::DiagnosticKind::SemanticConstructionFailed);
    ScopeGuard FunctionScope(Resolver, *Function);
    CHECK_ICE(Site, FunctionScope.scope(), core::DiagnosticKind::SemanticConstructionFailed);
    for (std::size_t Index = 0; Index < ParameterNames.size(); ++Index)
    {
      ir::Parameter *Parameter = Function->parameter(Index);
      Parameter->setName(ParameterNames[Index]);
      CHECK_ICE(State.at(Node.parameters()[Index].range()), Resolver.bind(ParameterNames[Index], *Parameter) == BindResult::Inserted, core::DiagnosticKind::SemanticConstructionFailed);
    }
    RETURN_IF_NOT(HasBody, registerFunctionExport(Site, State.Output, *Function));

    auto &Builder = State.irBuilder();
    InsertPointGuard InsertPoint(Builder);
    ir::BasicBlock *Entry = Function->createBlock(core::NamePool::instance().intern("entry"));
    CHECK_ICE(Site, Builder.setInsertPoint(*Entry), core::DiagnosticKind::SemanticConstructionFailed);
    RETURN_IF_NOT(analyzeBlockStmt(State, *Node.body()), false);
    ir::BasicBlock *Exit = Builder.insertBlock();
    CHECK_ICE(Site, Exit && Exit->function() == Function, core::DiagnosticKind::SemanticConstructionFailed);
    if (!Exit->hasTerminator())
    {
      RETURN_ERROR_IF(State.at(*Node.body()), !ir::VoidType::classof(ReturnType), core::DiagnosticKind::SemanticMissingReturn, false, Node.name().Text);
      CHECK_ICE(Site, Builder.createReturn(), core::DiagnosticKind::SemanticConstructionFailed);
    }
    return registerFunctionExport(Site, State.Output, *Function);
  }
} // namespace ink::semantic
