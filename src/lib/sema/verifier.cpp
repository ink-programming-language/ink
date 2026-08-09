#include "ink/sema/verifier.h"

#include "sema_internal.h"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace ink::sema
{
  namespace
  {
    void addError(SemanticVerificationResult &Result, std::string Message)
    {
      Result.Errors.push_back({std::move(Message)});
    }

    bool belongsToFile(const ast::AstFile &File, ast::AstNodeRef Ref) noexcept
    {
      switch (Ref.Category)
      {
      case ast::AstNodeCategory::Declaration:
        return File.declarations().contains(ast::AstDeclId::fromValue(Ref.Index));
      case ast::AstNodeCategory::Expression:
        return File.expressions().contains(ast::AstExprId::fromValue(Ref.Index));
      case ast::AstNodeCategory::Statement:
        return File.statements().contains(ast::AstStmtId::fromValue(Ref.Index));
      case ast::AstNodeCategory::Pattern:
        return File.patterns().contains(ast::AstPatternId::fromValue(Ref.Index));
      case ast::AstNodeCategory::Unknown:
        return false;
      }
      return false;
    }

    bool requiresRuntimeCategory(ast::AstKind Kind) noexcept
    {
      switch (Kind)
      {
      case ast::AstKind::LiteralExpression:
      case ast::AstKind::NameExpression:
      case ast::AstKind::GroupExpression:
      case ast::AstKind::UnaryExpression:
      case ast::AstKind::BinaryExpression:
      case ast::AstKind::CallExpression:
      case ast::AstKind::IfExpression:
      case ast::AstKind::ComptimeExpression:
        return true;
      default:
        return false;
      }
    }
  } // namespace

  bool SemanticVerificationResult::succeeded() const noexcept
  {
    return Errors.empty();
  }

  VerifiedSemanticModule::VerifiedSemanticModule(SemanticModel Model) noexcept : Model(std::move(Model))
  {
  }

  const SemanticModel &VerifiedSemanticModule::model() const noexcept
  {
    return Model;
  }

  core::SourceFileId VerifiedSemanticModule::sourceFile() const noexcept
  {
    return Model.sourceFile();
  }

  SemanticVerificationResult SemaVerifier::verify(const SemanticModel &Model) const
  {
    SemanticVerificationResult Result;
    if (!Model.Ast || !Model.Strings || !Model.Types)
    {
      addError(Result, "semantic model has a null owner context");
      return Result;
    }
    if (!Model.Ast->contains(Model.File))
    {
      addError(Result, "semantic model AST file does not belong to its AstContext");
      return Result;
    }
    if (!Model.File.sourceFile().isValid())
    {
      addError(Result, "semantic model source file ID is invalid");
    }
    if (Model.DeclSymbols.size() != Model.File.declarations().size() || Model.DeclScopes.size() != Model.File.declarations().size() || Model.DeclOwnedScopes.size() != Model.File.declarations().size() || Model.DeclaredTypes.size() != Model.File.declarations().size())
    {
      addError(Result, "declaration side-table size does not match the AST file declaration range");
    }
    if (Model.PatternSymbols.size() != Model.File.patterns().size() || Model.PatternScopes.size() != Model.File.patterns().size())
    {
      addError(Result, "pattern side-table size does not match the AST file pattern range");
    }
    if (Model.ExprScopes.size() != Model.File.expressions().size() || Model.ExprTypes.size() != Model.File.expressions().size() || Model.ExprKinds.size() != Model.File.expressions().size() || Model.ExprCategories.size() != Model.File.expressions().size() || Model.ExprWritable.size() != Model.File.expressions().size() || Model.ResolvedNames.size() != Model.File.expressions().size() || Model.ConstantValues.size() != Model.File.expressions().size())
    {
      addError(Result, "expression side-table size does not match the AST file expression range");
    }
    if (Model.StmtScopes.size() != Model.File.statements().size())
    {
      addError(Result, "statement side-table size does not match the AST file statement range");
    }
    if (!Model.contains(Model.GlobalScope))
    {
      addError(Result, "semantic model global scope is invalid");
    }
    for (std::size_t Index = 0; Index < Model.Scopes.size(); ++Index)
    {
      const Scope &Scope = Model.Scopes[Index];
      if (Scope.Id != ScopeId::fromValue(static_cast<ScopeId::ValueType>(Index)))
      {
        addError(Result, "scope ID does not match its table index");
      }
      if (Scope.Parent && (!Model.contains(*Scope.Parent) || Scope.Parent->value() >= Index))
      {
        addError(Result, "scope parent is invalid or does not precede its child");
      }
      if (!belongsToFile(Model.File, Scope.Owner))
      {
        addError(Result, "scope owner does not belong to the semantic model AST file");
      }
      std::vector<bool> SeenSymbols(Model.Symbols.size(), false);
      for (const SymbolId SymbolId : Scope.Symbols)
      {
        if (!Model.contains(SymbolId))
        {
          addError(Result, "scope contains an invalid symbol ID");
          continue;
        }
        if (SeenSymbols[SymbolId.value()])
        {
          addError(Result, "scope contains the same symbol more than once");
        }
        SeenSymbols[SymbolId.value()] = true;
        if (Model.symbol(SymbolId).Scope != Scope.Id)
        {
          addError(Result, "scope symbol ownership disagrees with the symbol table");
        }
      }
    }
    for (std::size_t Index = 0; Index < Model.Symbols.size(); ++Index)
    {
      const Symbol &Symbol = Model.Symbols[Index];
      if (Symbol.Id != SymbolId::fromValue(static_cast<SymbolId::ValueType>(Index)))
      {
        addError(Result, "symbol ID does not match its table index");
      }
      if (!Model.Strings->contains(Symbol.Name))
      {
        addError(Result, "symbol name does not belong to the semantic model string interner");
      }
      if (!Model.File.declarations().contains(Symbol.Declaration))
      {
        addError(Result, "symbol declaration does not belong to the semantic model AST file");
      }
      if (!Model.contains(Symbol.Scope))
      {
        addError(Result, "symbol scope is invalid");
      }
      if (!Model.Types->contains(Symbol.Type))
      {
        addError(Result, "symbol type does not belong to the semantic model TypeContext");
      }
      else
      {
        const type::TypeKind Kind = Model.Types->type(Symbol.Type).Kind;
        if (Kind == type::TypeKind::Error)
        {
          addError(Result, "verified semantic model contains a symbol with ErrorType");
        }
        if (Symbol.Kind == SymbolKind::Function && Kind != type::TypeKind::Function)
        {
          addError(Result, "function symbol does not have a function type");
        }
        if (Symbol.Kind == SymbolKind::Parameter && (Kind == type::TypeKind::Void || Kind == type::TypeKind::Never || Kind == type::TypeKind::Function))
        {
          addError(Result, "first-slice parameter has a non-value parameter type");
        }
        if (Symbol.Kind == SymbolKind::Binding && (Kind == type::TypeKind::Void || Kind == type::TypeKind::Function))
        {
          addError(Result, "first-slice binding has a non-value binding type");
        }
      }
      if (Model.File.declarations().contains(Symbol.Declaration))
      {
        const std::optional<SymbolId> DeclSymbol = Model.declarationSymbol(Symbol.Declaration);
        if (!DeclSymbol || *DeclSymbol != Symbol.Id)
        {
          addError(Result, "declaration-to-symbol side table disagrees with the symbol table");
        }
      }
    }
    for (std::uint32_t Value = Model.File.declarations().Begin.value(); Value < Model.File.declarations().End.value(); ++Value)
    {
      const ast::AstDeclId Id = ast::AstDeclId::fromValue(Value);
      const ast::Declaration &Node = Model.Ast->declaration(Id);
      if (!Model.contains(Model.declarationScope(Id)))
      {
        addError(Result, "declaration has no valid owning scope");
      }
      if (!Model.Types->contains(Model.declaredType(Id)))
      {
        addError(Result, "declaration type does not belong to the semantic model TypeContext");
      }
      if (const std::optional<ScopeId> Owned = Model.declarationOwnedScope(Id); Owned && !Model.contains(*Owned))
      {
        addError(Result, "declaration-owned scope is invalid");
      }
      const bool RequiresSymbol = Node.Kind == ast::AstKind::BindingDeclaration || Node.Kind == ast::AstKind::ParameterDeclaration || Node.Kind == ast::AstKind::FunctionDeclaration;
      if (RequiresSymbol && !Model.declarationSymbol(Id))
      {
        addError(Result, "named declaration has no symbol");
      }
      if (Node.Kind == ast::AstKind::ErrorDeclaration)
      {
        addError(Result, "verified semantic model contains an ErrorDeclaration");
      }
      if (Node.Kind == ast::AstKind::ImportDeclaration)
      {
        addError(Result, "verified semantic model contains an unsupported import declaration");
      }
      if (const ast::BindingPayload *Payload = std::get_if<ast::BindingPayload>(&Node.Payload); Node.Kind == ast::AstKind::BindingDeclaration && Payload && !Payload->TopLevel && !Payload->Initializer)
      {
        addError(Result, "first-slice local binding has no initializer");
      }
      if (const ast::ParameterPayload *Payload = std::get_if<ast::ParameterPayload>(&Node.Payload); Node.Kind == ast::AstKind::ParameterDeclaration && Payload && Payload->DefaultValue)
      {
        addError(Result, "first-slice parameter has a default argument");
      }
    }
    for (std::uint32_t Value = Model.File.patterns().Begin.value(); Value < Model.File.patterns().End.value(); ++Value)
    {
      const ast::AstPatternId Id = ast::AstPatternId::fromValue(Value);
      const ast::Pattern &Node = Model.Ast->pattern(Id);
      if (!Model.contains(Model.PatternScopes[Model.patternIndex(Id)]))
      {
        addError(Result, "pattern has no valid owning scope");
      }
      if (Node.Kind == ast::AstKind::BindingPattern && !Model.patternSymbol(Id))
      {
        addError(Result, "binding pattern has no symbol");
      }
      if (Node.Kind == ast::AstKind::ErrorPattern)
      {
        addError(Result, "verified semantic model contains an ErrorPattern");
      }
    }
    for (std::uint32_t Value = Model.File.expressions().Begin.value(); Value < Model.File.expressions().End.value(); ++Value)
    {
      const ast::AstExprId Id = ast::AstExprId::fromValue(Value);
      const ast::Expression &Node = Model.Ast->expression(Id);
      if (!Model.contains(Model.expressionScope(Id)))
      {
        addError(Result, "expression has no valid owning scope");
      }
      if (!Model.Types->contains(Model.expressionType(Id)))
      {
        addError(Result, "expression type does not belong to the semantic model TypeContext");
      }
      if (Model.expressionKind(Id) != Node.Kind)
      {
        addError(Result, "recorded expression kind disagrees with the AST");
      }
      if (requiresRuntimeCategory(Node.Kind) && Model.expressionCategory(Id) == ExpressionCategory::Unknown)
      {
        addError(Result, "runtime expression has no value category");
      }
      if (requiresRuntimeCategory(Node.Kind) && Model.Types->contains(Model.expressionType(Id)) && (Model.expressionType(Id) == Model.Types->errorType() || Model.expressionCategory(Id) == ExpressionCategory::Error))
      {
        addError(Result, "verified semantic model contains an erroneous runtime expression");
      }
      if (Model.expressionWritable(Id) && Model.expressionCategory(Id) != ExpressionCategory::Place)
      {
        addError(Result, "writable expression is not a place");
      }
      if (Node.Kind == ast::AstKind::NameExpression)
      {
        const ResolvedName Name = Model.resolvedName(Id);
        if (Name.Status != ResolvedNameStatus::Resolved || !Model.contains(Name.Symbol))
        {
          addError(Result, "name expression does not have one valid resolution");
        }
      }
      if (Node.Kind == ast::AstKind::ErrorExpression)
      {
        addError(Result, "verified semantic model contains an ErrorExpression");
      }
      if (Node.Kind == ast::AstKind::FunctionTypeExpression)
      {
        addError(Result, "verified first-slice semantic model contains a source function type");
      }
      if (Node.Kind == ast::AstKind::LiteralExpression)
      {
        const ast::LiteralPayload *Payload = std::get_if<ast::LiteralPayload>(&Node.Payload);
        const std::optional<ConstantValue> Constant = Model.constantValue(Id);
        if (!Payload)
        {
          addError(Result, "literal expression has no literal payload");
        }
        else if (Payload->Kind == ast::AstLiteralKind::Integer && (!Constant || !std::holds_alternative<std::int32_t>(*Constant) || Model.expressionType(Id) != Model.Types->i32Type()))
        {
          addError(Result, "i32 literal expression has no exact semantic constant value");
        }
        else if (Payload->Kind == ast::AstLiteralKind::Bool && (!Constant || !std::holds_alternative<bool>(*Constant) || Model.expressionType(Id) != Model.Types->boolType()))
        {
          addError(Result, "bool literal expression has no exact semantic constant value");
        }
      }
    }
    for (std::uint32_t Value = Model.File.statements().Begin.value(); Value < Model.File.statements().End.value(); ++Value)
    {
      const ast::AstStmtId Id = ast::AstStmtId::fromValue(Value);
      if (!Model.contains(Model.statementScope(Id)))
      {
        addError(Result, "statement has no valid owning scope");
      }
      if (Model.Ast->statement(Id).Kind == ast::AstKind::ErrorStatement)
      {
        addError(Result, "verified semantic model contains an ErrorStatement");
      }
    }
    return Result;
  }

  SemanticVerificationOutcome SemaVerifier::verifyAndSeal(SemanticModel Model) const
  {
    SemanticVerificationOutcome Outcome{verify(Model), std::nullopt};
    if (Model.hasSemanticErrors())
    {
      addError(Outcome.Verification, "semantic model records semantic analysis errors");
    }
    if (Outcome.Verification.succeeded())
    {
      Outcome.Module = VerifiedSemanticModule(std::move(Model));
    }
    return Outcome;
  }
} // namespace ink::sema
