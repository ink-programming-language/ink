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
  } // namespace

  bool Analyzer::analyzeFunctionDecl(AnalyzerState &State, const frontend::parser::FunctionDecl &Node)
  {
    auto &Context = State.semanticContext();
    auto &Resolver = State.nameResolver();
    RETURN_REPORT_IF(Context, Node.nativeSymbolKind() != frontend::parser::NativeSymbolKind::None && Node.nativeSymbolKind() != frontend::parser::NativeSymbolKind::Import && Node.nativeSymbolKind() != frontend::parser::NativeSymbolKind::Export, core::DiagnosticKind::SemanticInvalidAST, false, Node.getSourceRange(), "FunctionDecl native symbol kind must be None, Import or Export");
    RETURN_REPORT_IF(Context, Node.bodyKind() != frontend::parser::FunctionBodyKind::Definition && Node.bodyKind() != frontend::parser::FunctionBodyKind::DeclarationOnly, core::DiagnosticKind::SemanticInvalidAST, false, Node.getSourceRange(), "FunctionDecl body kind must be DeclarationOnly or Definition");
    RETURN_REPORT_IF(Context, Node.visibility() != frontend::parser::DeclarationVisibility::Default && Node.visibility() != frontend::parser::DeclarationVisibility::Public && Node.visibility() != frontend::parser::DeclarationVisibility::Private, core::DiagnosticKind::SemanticInvalidAST, false, Node.getSourceRange(), "FunctionDecl visibility must be Default, Public or Private");
    const bool HasBody = Node.bodyKind() == frontend::parser::FunctionBodyKind::Definition;
    RETURN_REPORT_IF(Context, HasBody && !Node.body(), core::DiagnosticKind::SemanticInvalidAST, false, Node.getSourceRange(), "FunctionDecl definition requires a body");
    RETURN_REPORT_IF(Context, !HasBody && Node.body(), core::DiagnosticKind::SemanticInvalidAST, false, Node.getSourceRange(), "FunctionDecl declaration-only form must not have a body");
    RETURN_REPORT_IF_FALSE(Context, Node.returnType(), core::DiagnosticKind::SemanticInvalidAST, false, Node.getSourceRange(), "FunctionDecl requires a return type annotation");
    RETURN_REPORT_IF(Context, Node.name().Text.empty(), core::DiagnosticKind::SemanticInvalidAST, false, Node.getSourceRange(), "FunctionDecl requires a name");
    // TODO: Define ownership and capture semantics for local and nested functions.
    UNSUPPORT_IF(State, Context.scopeStore().memberScope(State.Output) != &Resolver.currentScope(), false, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
    // TODO: Analyze generic parameters and instantiate generic function signatures and bodies.
    UNSUPPORT_IF(State, !Node.genericParameters().empty(), false, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
    // TODO: Validate function attributes and lower their supported semantics.
    UNSUPPORT_IF(State, !Node.attributes().empty(), false, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
    for (const auto &Parameter : Node.parameters())
    {
      RETURN_REPORT_IF(Context, Parameter.name().Text.empty(), core::DiagnosticKind::SemanticInvalidAST, false, Parameter.range(), "FunctionDecl parameter requires a name");
      RETURN_REPORT_IF_FALSE(Context, Parameter.type(), core::DiagnosticKind::SemanticInvalidAST, false, Parameter.range(), "FunctionDecl parameter requires a type annotation");
      // TODO: Analyze and retain default arguments for call-site argument completion.
      UNSUPPORT_IF(State, Parameter.defaultValue(), false, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
      // TODO: Lower variadic parameters and parameter-pack operands.
      UNSUPPORT_IF(State, Parameter.variadic(), false, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
    }

    ir::SymbolLinkage SymbolLink = ir::SymbolLinkage::Local;
    ir::LanguageLinkage LanguageLink = ir::LanguageLinkage::Ink;
    if (Node.nativeSymbolKind() != frontend::parser::NativeSymbolKind::None)
    {
      const auto *Linkage = Node.linkage();
      RETURN_REPORT_IF_FALSE(Context, Linkage, core::DiagnosticKind::SemanticInvalidAST, false, Node.getSourceRange(), "FunctionDecl native import/export requires a linkage expression");
      RETURN_REPORT_IF_FALSE(Context, frontend::parser::LiteralExpr::classof(Linkage), core::DiagnosticKind::SemanticInvalidAST, false, Linkage->getSourceRange(), "FunctionDecl linkage must be a string literal");
      const auto &Literal = static_cast<const frontend::parser::LiteralExpr &>(*Linkage);
      RETURN_REPORT_IF(Context, Literal.literalKind() != frontend::parser::TokenKind::StringLiteral, core::DiagnosticKind::SemanticInvalidAST, false, Linkage->getSourceRange(), "FunctionDecl linkage must be a string literal");
      const auto &Token = State.input().requireToken(Literal.token(), Context.diagnosticEngine(), Linkage->getSourceRange());
      const auto *String = std::get_if<frontend::tokenizer::StringInfo>(&Token.Payload);
      RETURN_REPORT_IF(Context, Token.Kind != frontend::parser::TokenKind::StringLiteral || !String, core::DiagnosticKind::SemanticInvalidAST, false, Linkage->getSourceRange(), "FunctionDecl linkage literal must reference a string token with StringInfo metadata");
      // TODO: Resolve and lower additional native language linkages.
      UNSUPPORT_IF(State, String->Decoded != "C", false, Node.getSourceRange(), frontend::parser::astKindName(Node.getKind()));
      SymbolLink = Node.nativeSymbolKind() == frontend::parser::NativeSymbolKind::Import ? ir::SymbolLinkage::Import : ir::SymbolLinkage::Export;
      LanguageLink = ir::LanguageLinkage::C;
    }
    else
    {
      RETURN_REPORT_IF(Context, Node.linkage(), core::DiagnosticKind::SemanticInvalidAST, false, Node.getSourceRange(), "FunctionDecl without native import/export must not have a linkage expression");
    }
    RETURN_REPORT_IF(Context, SymbolLink == ir::SymbolLinkage::Import && HasBody, core::DiagnosticKind::SemanticNativeImportHasBody, false, Node.getSourceRange(), Node.name().Text);
    RETURN_REPORT_IF(Context, !HasBody && SymbolLink != ir::SymbolLinkage::Import, core::DiagnosticKind::SemanticFunctionRequiresBody, false, Node.getSourceRange(), Node.name().Text);

    const ir::Type *ReturnType = analyzeType(State, *Node.returnType());
    RETURN_IF_NULL(ReturnType, false);
    CHECK_HAS_LAYOUT(Context, Node.returnType()->getSourceRange(), ReturnType, "a return type with runtime layout", false);
    std::vector<ir::FunctionTypeParameter> Parameters;
    std::vector<core::Name> ParameterNames;
    std::unordered_set<core::Name> UniqueParameterNames;
    Parameters.reserve(Node.parameters().size());
    ParameterNames.reserve(Node.parameters().size());
    for (const auto &Parameter : Node.parameters())
    {
      const auto Name = core::NamePool::instance().intern(Parameter.name().Text);
      CHECK_VALID_NAME(Context, Parameter.range(), Name, false);
      RETURN_REPORT_IF_FALSE(Context, UniqueParameterNames.insert(Name).second, core::DiagnosticKind::SemanticDuplicateParameterName, false, Parameter.name().Range, Parameter.name().Text);
      const ir::Type *Type = analyzeType(State, *Parameter.type());
      RETURN_IF_NULL(Type, false);
      CHECK_HAS_LAYOUT(Context, Parameter.type()->getSourceRange(), Type, "a parameter type with runtime layout", false);
      Parameters.emplace_back(Type);
      ParameterNames.push_back(Name);
    }
    const ir::FunctionType *Signature = Context.typePool().getFunctionType(ReturnType, Parameters);
    const auto Name = core::NamePool::instance().intern(Node.name().Text);
    RETURN_REPORT_IF_FALSE(Context, Signature, core::DiagnosticKind::SemanticConstructionFailed, false, Node.getSourceRange());
    CHECK_VALID_NAME(Context, Node.getSourceRange(), Name, false);
    if (const Binding *Existing = Resolver.lookupLocal(Name))
    {
      RETURN_REPORT_IF_FALSE(Context, Existing->isOverloadSet(), core::DiagnosticKind::SemanticDuplicateName, false, Node.name().Range, Node.name().Text);
      for (const ir::Value *Target : Existing->targets())
      {
        RETURN_REPORT_IF_FALSE(Context, ir::Function::classof(Target), core::DiagnosticKind::SemanticConstructionFailed, false, Node.getSourceRange());
        const auto &ExistingFunction = static_cast<const ir::Function &>(*Target);
        // TODO: Merge compatible native redeclarations once declaration identity and definition tracking exist.
        RETURN_REPORT_IF(Context, LanguageLink == ir::LanguageLinkage::C || ExistingFunction.languageLinkage() == ir::LanguageLinkage::C || sameParameterTypes(*Signature, *ExistingFunction.functionType()), core::DiagnosticKind::SemanticDuplicateName, false, Node.name().Range, Node.name().Text);
      }
    }

    const auto Visibility = Node.visibility() == frontend::parser::DeclarationVisibility::Private ? ir::FunctionVisibility::Private : ir::FunctionVisibility::Public;
    // TODO: Support function definitions in nested scopes with appropriate ownership and lexical bindings.
    ir::Function *Function = State.Output.createGlobalFunction(*Signature, Name, SymbolLink, LanguageLink, Visibility);
    RETURN_REPORT_IF(Context, !Function || Resolver.bindOverload(Name, *Function) != BindResult::Inserted, core::DiagnosticKind::SemanticConstructionFailed, false, Node.getSourceRange());
    ScopeGuard FunctionScope(Resolver, *Function);
    RETURN_REPORT_IF_FALSE(Context, FunctionScope.scope(), core::DiagnosticKind::SemanticConstructionFailed, false, Node.getSourceRange());
    for (std::size_t Index = 0; Index < ParameterNames.size(); ++Index)
    {
      ir::Parameter *Parameter = Function->parameter(Index);
      Parameter->setName(ParameterNames[Index]);
      RETURN_REPORT_IF(Context, Resolver.bind(ParameterNames[Index], *Parameter) != BindResult::Inserted, core::DiagnosticKind::SemanticConstructionFailed, false, Node.parameters()[Index].range());
    }
    RETURN_IF(!HasBody, true);

    auto &Builder = State.irBuilder();
    InsertPointGuard InsertPoint(Builder);
    ir::BasicBlock *Entry = Function->createBlock(core::NamePool::instance().intern("entry"));
    RETURN_REPORT_IF_FALSE(Context, Builder.setInsertPoint(*Entry), core::DiagnosticKind::SemanticConstructionFailed, false, Node.getSourceRange());
    RETURN_IF(!analyzeBlockStmt(State, *Node.body()), false);
    ir::BasicBlock *Exit = Builder.insertBlock();
    RETURN_REPORT_IF(Context, !Exit || Exit->function() != Function, core::DiagnosticKind::SemanticConstructionFailed, false, Node.getSourceRange());
    if (!Exit->hasTerminator())
    {
      RETURN_REPORT_IF(Context, !ir::VoidType::classof(ReturnType), core::DiagnosticKind::SemanticMissingReturn, false, Node.body()->getSourceRange(), Node.name().Text);
      RETURN_REPORT_IF_FALSE(Context, Builder.createReturn(), core::DiagnosticKind::SemanticConstructionFailed, false, Node.getSourceRange());
    }
    return true;
  }
} // namespace ink::semantic
