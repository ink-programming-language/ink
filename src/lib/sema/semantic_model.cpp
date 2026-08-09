#include "ink/sema/semantic_model.h"

#include <stdexcept>

namespace ink::sema
{
  const char *symbolKindName(SymbolKind Kind) noexcept
  {
    switch (Kind)
    {
    case SymbolKind::Function:
      return "Function";
    case SymbolKind::Parameter:
      return "Parameter";
    case SymbolKind::Binding:
      return "Binding";
    }
    return "Unknown";
  }

  SemanticModel::SemanticModel(const ast::AstContext &AstContext, const ast::AstFile &AstFile, const core::StringInterner &Strings, type::TypeContext &Types) : Ast(&AstContext), File(AstFile), Strings(&Strings), Types(&Types)
  {
    if (!AstContext.contains(AstFile))
    {
      throw std::invalid_argument("semantic model requires an AST file owned by its AstContext");
    }
    DeclSymbols.resize(File.declarations().size());
    DeclScopes.resize(File.declarations().size());
    DeclOwnedScopes.resize(File.declarations().size());
    DeclaredTypes.resize(File.declarations().size(), Types.errorType());
    PatternSymbols.resize(File.patterns().size());
    PatternScopes.resize(File.patterns().size());
    ExprScopes.resize(File.expressions().size());
    ExprTypes.resize(File.expressions().size(), Types.errorType());
    ExprKinds.reserve(File.expressions().size());
    ExprCategories.resize(File.expressions().size(), ExpressionCategory::Unknown);
    ExprWritable.resize(File.expressions().size(), false);
    ResolvedNames.resize(File.expressions().size());
    ConstantValues.resize(File.expressions().size());
    ExprChecked.resize(File.expressions().size(), false);
    StmtScopes.resize(File.statements().size());
    ReportedUnsupportedDecls.resize(File.declarations().size(), false);
    ReportedUnsupportedExprs.resize(File.expressions().size(), false);
    ReportedUnsupportedStmts.resize(File.statements().size(), false);
    ReportedUnsupportedPatterns.resize(File.patterns().size(), false);
    for (std::uint32_t Index = File.expressions().Begin.value(); Index < File.expressions().End.value(); ++Index)
    {
      ExprKinds.push_back(AstContext.expression(ast::AstExprId::fromValue(Index)).Kind);
    }
  }

  const ast::AstContext &SemanticModel::astContext() const noexcept
  {
    return *Ast;
  }

  const ast::AstFile &SemanticModel::astFile() const noexcept
  {
    return File;
  }

  const core::StringInterner &SemanticModel::strings() const noexcept
  {
    return *Strings;
  }

  const type::TypeContext &SemanticModel::typeContext() const noexcept
  {
    return *Types;
  }

  core::SourceFileId SemanticModel::sourceFile() const noexcept
  {
    return File.sourceFile();
  }

  ScopeId SemanticModel::globalScope() const noexcept
  {
    return GlobalScope;
  }

  bool SemanticModel::contains(ScopeId Id) const noexcept
  {
    return Id.isValid() && Id.value() < Scopes.size();
  }

  bool SemanticModel::contains(SymbolId Id) const noexcept
  {
    return Id.isValid() && Id.value() < Symbols.size();
  }

  const Scope &SemanticModel::scope(ScopeId Id) const
  {
    if (!contains(Id))
    {
      throw std::out_of_range("ScopeId does not identify a scope in this SemanticModel");
    }
    return Scopes[Id.value()];
  }

  const Symbol &SemanticModel::symbol(SymbolId Id) const
  {
    if (!contains(Id))
    {
      throw std::out_of_range("SymbolId does not identify a symbol in this SemanticModel");
    }
    return Symbols[Id.value()];
  }

  const std::vector<Scope> &SemanticModel::scopes() const noexcept
  {
    return Scopes;
  }

  const std::vector<Symbol> &SemanticModel::symbols() const noexcept
  {
    return Symbols;
  }

  std::optional<SymbolId> SemanticModel::declarationSymbol(ast::AstDeclId Id) const
  {
    return DeclSymbols[declarationIndex(Id)];
  }

  ScopeId SemanticModel::declarationScope(ast::AstDeclId Id) const
  {
    return DeclScopes[declarationIndex(Id)];
  }

  std::optional<ScopeId> SemanticModel::declarationOwnedScope(ast::AstDeclId Id) const
  {
    return DeclOwnedScopes[declarationIndex(Id)];
  }

  std::optional<SymbolId> SemanticModel::patternSymbol(ast::AstPatternId Id) const
  {
    return PatternSymbols[patternIndex(Id)];
  }

  ScopeId SemanticModel::expressionScope(ast::AstExprId Id) const
  {
    return ExprScopes[expressionIndex(Id)];
  }

  ScopeId SemanticModel::statementScope(ast::AstStmtId Id) const
  {
    return StmtScopes[statementIndex(Id)];
  }

  type::TypeId SemanticModel::declaredType(ast::AstDeclId Id) const
  {
    return DeclaredTypes[declarationIndex(Id)];
  }

  type::TypeId SemanticModel::expressionType(ast::AstExprId Id) const
  {
    return ExprTypes[expressionIndex(Id)];
  }

  ast::AstKind SemanticModel::expressionKind(ast::AstExprId Id) const
  {
    return ExprKinds[expressionIndex(Id)];
  }

  ExpressionCategory SemanticModel::expressionCategory(ast::AstExprId Id) const
  {
    return ExprCategories[expressionIndex(Id)];
  }

  bool SemanticModel::expressionWritable(ast::AstExprId Id) const
  {
    return ExprWritable[expressionIndex(Id)];
  }

  ResolvedName SemanticModel::resolvedName(ast::AstExprId Id) const
  {
    return ResolvedNames[expressionIndex(Id)];
  }

  std::optional<ConstantValue> SemanticModel::constantValue(ast::AstExprId Id) const
  {
    return ConstantValues[expressionIndex(Id)];
  }

  bool SemanticModel::hasSemanticErrors() const noexcept
  {
    return HasSemanticErrors;
  }

  std::size_t SemanticModel::declarationIndex(ast::AstDeclId Id) const
  {
    if (!File.declarations().contains(Id))
    {
      throw std::out_of_range("AstDeclId does not belong to this SemanticModel");
    }
    return static_cast<std::size_t>(Id.value() - File.declarations().Begin.value());
  }

  std::size_t SemanticModel::expressionIndex(ast::AstExprId Id) const
  {
    if (!File.expressions().contains(Id))
    {
      throw std::out_of_range("AstExprId does not belong to this SemanticModel");
    }
    return static_cast<std::size_t>(Id.value() - File.expressions().Begin.value());
  }

  std::size_t SemanticModel::statementIndex(ast::AstStmtId Id) const
  {
    if (!File.statements().contains(Id))
    {
      throw std::out_of_range("AstStmtId does not belong to this SemanticModel");
    }
    return static_cast<std::size_t>(Id.value() - File.statements().Begin.value());
  }

  std::size_t SemanticModel::patternIndex(ast::AstPatternId Id) const
  {
    if (!File.patterns().contains(Id))
    {
      throw std::out_of_range("AstPatternId does not belong to this SemanticModel");
    }
    return static_cast<std::size_t>(Id.value() - File.patterns().Begin.value());
  }
} // namespace ink::sema
