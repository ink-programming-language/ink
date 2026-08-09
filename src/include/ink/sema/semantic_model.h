#ifndef INK_SEMA_SEMANTIC_MODEL_H
#define INK_SEMA_SEMANTIC_MODEL_H

#include "ink/ast/ast_context.h"
#include "ink/core/string_interner.h"
#include "ink/sema/scope.h"
#include "ink/sema/symbol.h"
#include "ink/type/type_context.h"

#include <cstdint>
#include <optional>
#include <variant>
#include <vector>

namespace ink::sema
{
  enum class ExpressionCategory : std::uint8_t
  {
    Unknown,
    Error,
    Value,
    Place,
  };

  enum class ResolvedNameStatus : std::uint8_t
  {
    NotApplicable,
    Resolved,
    Unresolved,
    Ambiguous,
  };

  struct ResolvedName
  {
    ResolvedNameStatus Status = ResolvedNameStatus::NotApplicable;
    SymbolId Symbol;
  };

  using ConstantValue = std::variant<bool, std::int32_t>;

  class DeclarationCollector;
  class SignatureResolver;
  class NameResolver;
  class TypeChecker;
  class ControlFlowChecker;
  class SemaVerifier;
  struct SemanticModelAccess;

  class SemanticModel
  {
  public:
    SemanticModel(const ast::AstContext &AstContext, const ast::AstFile &AstFile, const core::StringInterner &Strings, type::TypeContext &Types);
    SemanticModel(const SemanticModel &) = delete;
    SemanticModel &operator=(const SemanticModel &) = delete;
    SemanticModel(SemanticModel &&) noexcept = default;
    SemanticModel &operator=(SemanticModel &&) noexcept = default;

    const ast::AstContext &astContext() const noexcept;
    const ast::AstFile &astFile() const noexcept;
    const core::StringInterner &strings() const noexcept;
    const type::TypeContext &typeContext() const noexcept;
    core::SourceFileId sourceFile() const noexcept;

    ScopeId globalScope() const noexcept;
    bool contains(ScopeId Id) const noexcept;
    bool contains(SymbolId Id) const noexcept;
    const Scope &scope(ScopeId Id) const;
    const Symbol &symbol(SymbolId Id) const;
    const std::vector<Scope> &scopes() const noexcept;
    const std::vector<Symbol> &symbols() const noexcept;

    std::optional<SymbolId> declarationSymbol(ast::AstDeclId Id) const;
    ScopeId declarationScope(ast::AstDeclId Id) const;
    std::optional<ScopeId> declarationOwnedScope(ast::AstDeclId Id) const;
    std::optional<SymbolId> patternSymbol(ast::AstPatternId Id) const;
    ScopeId expressionScope(ast::AstExprId Id) const;
    ScopeId statementScope(ast::AstStmtId Id) const;
    type::TypeId declaredType(ast::AstDeclId Id) const;
    type::TypeId expressionType(ast::AstExprId Id) const;
    ast::AstKind expressionKind(ast::AstExprId Id) const;
    ExpressionCategory expressionCategory(ast::AstExprId Id) const;
    bool expressionWritable(ast::AstExprId Id) const;
    ResolvedName resolvedName(ast::AstExprId Id) const;
    std::optional<ConstantValue> constantValue(ast::AstExprId Id) const;
    bool hasSemanticErrors() const noexcept;

  private:
    std::size_t declarationIndex(ast::AstDeclId Id) const;
    std::size_t expressionIndex(ast::AstExprId Id) const;
    std::size_t statementIndex(ast::AstStmtId Id) const;
    std::size_t patternIndex(ast::AstPatternId Id) const;

    const ast::AstContext *Ast = nullptr;
    ast::AstFile File;
    const core::StringInterner *Strings = nullptr;
    type::TypeContext *Types = nullptr;
    ScopeId GlobalScope;
    std::vector<Scope> Scopes;
    std::vector<Symbol> Symbols;
    std::vector<std::optional<SymbolId>> DeclSymbols;
    std::vector<ScopeId> DeclScopes;
    std::vector<std::optional<ScopeId>> DeclOwnedScopes;
    std::vector<type::TypeId> DeclaredTypes;
    std::vector<std::optional<SymbolId>> PatternSymbols;
    std::vector<ScopeId> PatternScopes;
    std::vector<ScopeId> ExprScopes;
    std::vector<type::TypeId> ExprTypes;
    std::vector<ast::AstKind> ExprKinds;
    std::vector<ExpressionCategory> ExprCategories;
    std::vector<bool> ExprWritable;
    std::vector<ResolvedName> ResolvedNames;
    std::vector<std::optional<ConstantValue>> ConstantValues;
    std::vector<bool> ExprChecked;
    std::vector<ScopeId> StmtScopes;
    std::vector<bool> ReportedUnsupportedDecls;
    std::vector<bool> ReportedUnsupportedExprs;
    std::vector<bool> ReportedUnsupportedStmts;
    std::vector<bool> ReportedUnsupportedPatterns;
    bool HasSemanticErrors = false;

    friend class DeclarationCollector;
    friend class SignatureResolver;
    friend class NameResolver;
    friend class TypeChecker;
    friend class ControlFlowChecker;
    friend class SemaVerifier;
    friend struct SemanticModelAccess;
  };
} // namespace ink::sema

#endif
