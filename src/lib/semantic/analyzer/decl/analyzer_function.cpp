#include "ink/semantic/analyzer/analyzer.h"

#include "ink/core/name/name_pool.h"
#include "ink/frontend/parser/parser.h"
#include "ink/ir/function/function.h"
#include "ink/ir/module/module.h"
#include "ink/semantic/analyzer/analyzer_state.h"
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
      if (Left.parameterCount() != Right.parameterCount())
      {
        return false;
      }
      for (std::size_t Index = 0; Index < Left.parameterCount(); ++Index)
      {
        const auto &LeftParameter = Left.parameters()[Index];
        const auto &RightParameter = Right.parameters()[Index];
        if (LeftParameter.ParameterType != RightParameter.ParameterType || LeftParameter.Variadic != RightParameter.Variadic)
        {
          return false;
        }
      }
      return true;
    }
  } // namespace

  bool Analyzer::analyzeFunctionDecl(AnalyzerState &State, const frontend::parser::FunctionDecl &Node)
  {
    auto &Context = State.semanticContext();
    auto &Resolver = State.nameResolver();
    if (Node.nativeSymbolKind() != frontend::parser::NativeSymbolKind::None && Node.nativeSymbolKind() != frontend::parser::NativeSymbolKind::Import && Node.nativeSymbolKind() != frontend::parser::NativeSymbolKind::Export)
    {
      Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "FunctionDecl native symbol kind must be None, Import or Export");
      return false;
    }
    if (Node.bodyKind() != frontend::parser::FunctionBodyKind::Definition && Node.bodyKind() != frontend::parser::FunctionBodyKind::DeclarationOnly)
    {
      Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "FunctionDecl body kind must be DeclarationOnly or Definition");
      return false;
    }
    if (Node.visibility() != frontend::parser::DeclarationVisibility::Default && Node.visibility() != frontend::parser::DeclarationVisibility::Public && Node.visibility() != frontend::parser::DeclarationVisibility::Private)
    {
      Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "FunctionDecl visibility must be Default, Public or Private");
      return false;
    }
    const bool HasBody = Node.bodyKind() == frontend::parser::FunctionBodyKind::Definition;
    if (HasBody && !Node.body())
    {
      Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "FunctionDecl definition requires a body");
      return false;
    }
    if (!HasBody && Node.body())
    {
      Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "FunctionDecl declaration-only form must not have a body");
      return false;
    }
    if (!Node.returnType())
    {
      Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "FunctionDecl requires a return type annotation");
      return false;
    }
    if (Node.name().Text.empty())
    {
      Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "FunctionDecl requires a name");
      return false;
    }
    if (Context.scopeStore().memberScope(State.Output) != &Resolver.currentScope())
    {
      // TODO: Define ownership and capture semantics for local and nested functions.
      return reportUnsupported(State, Node);
    }
    if (!Node.genericParameters().empty())
    {
      // TODO: Analyze generic parameters and instantiate generic function signatures and bodies.
      return reportUnsupported(State, Node);
    }
    if (!Node.attributes().empty())
    {
      // TODO: Validate function attributes and lower their supported semantics.
      return reportUnsupported(State, Node);
    }
    for (const auto &Parameter : Node.parameters())
    {
      if (Parameter.name().Text.empty())
      {
        Context.report<core::DiagnosticKind::SemanticInvalidAST>(Parameter.range(), "FunctionDecl parameter requires a name");
        return false;
      }
      if (!Parameter.type())
      {
        Context.report<core::DiagnosticKind::SemanticInvalidAST>(Parameter.range(), "FunctionDecl parameter requires a type annotation");
        return false;
      }
      if (Parameter.defaultValue())
      {
        // TODO: Analyze and retain default arguments for call-site argument completion.
        return reportUnsupported(State, Node);
      }
      if (Parameter.variadic())
      {
        // TODO: Lower variadic parameters and parameter-pack operands.
        return reportUnsupported(State, Node);
      }
    }

    ir::SymbolLinkage SymbolLink = ir::SymbolLinkage::Local;
    ir::LanguageLinkage LanguageLink = ir::LanguageLinkage::Ink;
    if (Node.nativeSymbolKind() != frontend::parser::NativeSymbolKind::None)
    {
      const auto *Linkage = Node.linkage();
      if (!Linkage)
      {
        Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "FunctionDecl native import/export requires a linkage expression");
        return false;
      }
      if (!frontend::parser::LiteralExpr::classof(Linkage))
      {
        Context.report<core::DiagnosticKind::SemanticInvalidAST>(Linkage->getSourceRange(), "FunctionDecl linkage must be a string literal");
        return false;
      }
      const auto &Literal = static_cast<const frontend::parser::LiteralExpr &>(*Linkage);
      if (Literal.literalKind() != frontend::parser::TokenKind::StringLiteral)
      {
        Context.report<core::DiagnosticKind::SemanticInvalidAST>(Linkage->getSourceRange(), "FunctionDecl linkage must be a string literal");
        return false;
      }
      const auto &Input = State.input().Unit->input();
      if (Literal.token() >= Input.size())
      {
        Context.report<core::DiagnosticKind::SemanticInvalidAST>(Linkage->getSourceRange(), "FunctionDecl linkage literal must reference a token in its input");
        return false;
      }
      const auto &Token = Input.token(Literal.token());
      const auto *String = std::get_if<frontend::tokenizer::StringInfo>(&Token.Payload);
      if (Token.Kind != frontend::parser::TokenKind::StringLiteral || !String)
      {
        Context.report<core::DiagnosticKind::SemanticInvalidAST>(Linkage->getSourceRange(), "FunctionDecl linkage literal must reference a string token with StringInfo metadata");
        return false;
      }
      if (String->Decoded != "C")
      {
        // TODO: Resolve and lower additional native language linkages.
        return reportUnsupported(State, Node);
      }
      SymbolLink = Node.nativeSymbolKind() == frontend::parser::NativeSymbolKind::Import ? ir::SymbolLinkage::Import : ir::SymbolLinkage::Export;
      LanguageLink = ir::LanguageLinkage::C;
    }
    else if (Node.linkage())
    {
      Context.report<core::DiagnosticKind::SemanticInvalidAST>(Node.getSourceRange(), "FunctionDecl without native import/export must not have a linkage expression");
      return false;
    }
    if (SymbolLink == ir::SymbolLinkage::Import && HasBody)
    {
      Context.report<core::DiagnosticKind::SemanticNativeImportHasBody>(Node.getSourceRange(), Node.name().Text);
      return false;
    }
    if (!HasBody && SymbolLink != ir::SymbolLinkage::Import)
    {
      if (SymbolLink == ir::SymbolLinkage::Export)
      {
        Context.report<core::DiagnosticKind::SemanticNativeExportRequiresBody>(Node.getSourceRange(), Node.name().Text);
      }
      else
      {
        Context.report<core::DiagnosticKind::SemanticFunctionRequiresBody>(Node.getSourceRange(), Node.name().Text);
      }
      return false;
    }

    const ir::Type *ReturnType = analyzeType(State, *Node.returnType());
    if (!ReturnType)
    {
      return false;
    }
    if (!ReturnType->hasLayout())
    {
      Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Node.returnType()->getSourceRange(), "a return type with runtime layout", ReturnType->toString());
      return false;
    }
    std::vector<ir::FunctionTypeParameter> Parameters;
    std::vector<core::Name> ParameterNames;
    std::unordered_set<core::Name> UniqueParameterNames;
    Parameters.reserve(Node.parameters().size());
    ParameterNames.reserve(Node.parameters().size());
    for (const auto &Parameter : Node.parameters())
    {
      const auto Name = core::NamePool::instance().intern(Parameter.name().Text);
      if (!Name.valid())
      {
        Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Parameter.range());
        return false;
      }
      if (!UniqueParameterNames.insert(Name).second)
      {
        Context.report<core::DiagnosticKind::SemanticDuplicateParameterName>(Parameter.name().Range, Parameter.name().Text);
        return false;
      }
      const ir::Type *Type = analyzeType(State, *Parameter.type());
      if (!Type)
      {
        return false;
      }
      if (!Type->hasLayout())
      {
        Context.report<core::DiagnosticKind::SemanticTypeMismatch>(Parameter.type()->getSourceRange(), "a parameter type with runtime layout", Type->toString());
        return false;
      }
      Parameters.emplace_back(Type);
      ParameterNames.push_back(Name);
    }
    const ir::FunctionType *Signature = Context.typePool().getFunctionType(ReturnType, Parameters);
    const auto Name = core::NamePool::instance().intern(Node.name().Text);
    if (!Signature || !Name.valid())
    {
      Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    if (const Binding *Existing = Resolver.lookupLocal(Name))
    {
      if (!Existing->isOverloadSet())
      {
        Context.report<core::DiagnosticKind::SemanticDuplicateName>(Node.name().Range, Node.name().Text);
        return false;
      }
      for (const ir::Value *Target : Existing->targets())
      {
        if (!ir::Function::classof(Target))
        {
          Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
          return false;
        }
        const auto &ExistingFunction = static_cast<const ir::Function &>(*Target);
        if (LanguageLink == ir::LanguageLinkage::C || ExistingFunction.languageLinkage() == ir::LanguageLinkage::C || sameParameterTypes(*Signature, *ExistingFunction.functionType()))
        {
          // TODO: Merge compatible native redeclarations once declaration identity and definition tracking exist.
          Context.report<core::DiagnosticKind::SemanticDuplicateName>(Node.name().Range, Node.name().Text);
          return false;
        }
      }
    }

    const auto Visibility = Node.visibility() == frontend::parser::DeclarationVisibility::Private ? ir::FunctionVisibility::Private : ir::FunctionVisibility::Public;
    ir::Function *Function = State.Output.createGlobalFunction(*Signature, Name, SymbolLink, LanguageLink, Visibility);
    if (!Function || Resolver.bindOverload(Name, *Function) != BindResult::Inserted)
    {
      Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    ScopeGuard FunctionScope(Resolver, *Function);
    if (!FunctionScope.scope())
    {
      Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    for (std::size_t Index = 0; Index < ParameterNames.size(); ++Index)
    {
      ir::Parameter *Parameter = Function->parameter(Index);
      Parameter->setName(ParameterNames[Index]);
      if (Resolver.bind(ParameterNames[Index], *Parameter) != BindResult::Inserted)
      {
        Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.parameters()[Index].range());
        return false;
      }
    }
    if (!HasBody)
    {
      return true;
    }

    auto &Builder = State.irBuilder();
    InsertPointGuard InsertPoint(Builder);
    ir::BasicBlock *Entry = Function->createBlock(core::NamePool::instance().intern("entry"));
    if (!Builder.setInsertPoint(*Entry))
    {
      Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    if (!analyzeBlockStmt(State, *Node.body()))
    {
      return false;
    }
    ir::BasicBlock *Exit = Builder.insertBlock();
    if (!Exit || Exit->function() != Function)
    {
      Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
      return false;
    }
    if (!Exit->hasTerminator())
    {
      if (ReturnType != Context.typePool().getVoidType())
      {
        Context.report<core::DiagnosticKind::SemanticMissingReturn>(Node.body()->getSourceRange(), Node.name().Text);
        return false;
      }
      if (!Builder.createReturn())
      {
        Context.report<core::DiagnosticKind::SemanticConstructionFailed>(Node.getSourceRange());
        return false;
      }
    }
    return true;
  }
} // namespace ink::semantic
