#ifndef INK_LIB_SEMA_SEMA_INTERNAL_H
#define INK_LIB_SEMA_SEMA_INTERNAL_H

#include "ink/core/diagnostic.h"
#include "ink/sema/semantic_model.h"

#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ink::sema
{
  struct SemanticModelAccess
  {
    static type::TypeContext &types(SemanticModel &Model) noexcept
    {
      return *Model.Types;
    }

    static ScopeId addScope(SemanticModel &Model, std::optional<ScopeId> Parent, ast::AstNodeRef Owner)
    {
      if (Model.Scopes.size() >= ScopeId::InvalidValue)
      {
        throw std::length_error("SemanticModel cannot represent another scope ID");
      }
      const ScopeId Id = ScopeId::fromValue(static_cast<ScopeId::ValueType>(Model.Scopes.size()));
      Model.Scopes.push_back({Id, Parent, Owner, {}});
      return Id;
    }

    static SymbolId addSymbol(SemanticModel &Model, SymbolKind Kind, core::InternedStringId Name, ast::AstDeclId Declaration, ScopeId Scope, bool Mutable)
    {
      if (Model.Symbols.size() >= SymbolId::InvalidValue)
      {
        throw std::length_error("SemanticModel cannot represent another symbol ID");
      }
      const SymbolId Id = SymbolId::fromValue(static_cast<SymbolId::ValueType>(Model.Symbols.size()));
      Model.Symbols.push_back({Id, Kind, Name, Declaration, Scope, Model.Types->errorType(), Mutable});
      Model.Scopes.at(Scope.value()).Symbols.push_back(Id);
      Model.DeclSymbols[Model.declarationIndex(Declaration)] = Id;
      return Id;
    }

    static Scope &scope(SemanticModel &Model, ScopeId Id)
    {
      return Model.Scopes.at(Id.value());
    }

    static Symbol &symbol(SemanticModel &Model, SymbolId Id)
    {
      return Model.Symbols.at(Id.value());
    }

    static void setGlobalScope(SemanticModel &Model, ScopeId Id) noexcept
    {
      Model.GlobalScope = Id;
    }

    static void setDeclarationScope(SemanticModel &Model, ast::AstDeclId Id, ScopeId Scope)
    {
      Model.DeclScopes[Model.declarationIndex(Id)] = Scope;
    }

    static void setDeclarationOwnedScope(SemanticModel &Model, ast::AstDeclId Id, ScopeId Scope)
    {
      Model.DeclOwnedScopes[Model.declarationIndex(Id)] = Scope;
    }

    static void setDeclaredType(SemanticModel &Model, ast::AstDeclId Id, type::TypeId Type)
    {
      Model.DeclaredTypes[Model.declarationIndex(Id)] = Type;
    }

    static void setPatternScope(SemanticModel &Model, ast::AstPatternId Id, ScopeId Scope)
    {
      Model.PatternScopes[Model.patternIndex(Id)] = Scope;
    }

    static void setPatternSymbol(SemanticModel &Model, ast::AstPatternId Id, SymbolId Symbol)
    {
      Model.PatternSymbols[Model.patternIndex(Id)] = Symbol;
    }

    static void setExpressionScope(SemanticModel &Model, ast::AstExprId Id, ScopeId Scope)
    {
      Model.ExprScopes[Model.expressionIndex(Id)] = Scope;
    }

    static void setStatementScope(SemanticModel &Model, ast::AstStmtId Id, ScopeId Scope)
    {
      Model.StmtScopes[Model.statementIndex(Id)] = Scope;
    }

    static void setExpressionType(SemanticModel &Model, ast::AstExprId Id, type::TypeId Type, ExpressionCategory Category, bool Writable = false)
    {
      const std::size_t Index = Model.expressionIndex(Id);
      Model.ExprTypes[Index] = Type;
      Model.ExprCategories[Index] = Category;
      Model.ExprWritable[Index] = Writable;
      Model.ExprChecked[Index] = true;
    }

    static void setTypeSyntax(SemanticModel &Model, ast::AstExprId Id, type::TypeId Type)
    {
      Model.ExprTypes[Model.expressionIndex(Id)] = Type;
    }

    static bool expressionChecked(const SemanticModel &Model, ast::AstExprId Id)
    {
      return Model.ExprChecked[Model.expressionIndex(Id)];
    }

    static void setResolvedName(SemanticModel &Model, ast::AstExprId Id, ResolvedName Value)
    {
      Model.ResolvedNames[Model.expressionIndex(Id)] = Value;
    }

    static void setConstantValue(SemanticModel &Model, ast::AstExprId Id, std::optional<ConstantValue> Value)
    {
      Model.ConstantValues[Model.expressionIndex(Id)] = std::move(Value);
    }

    static void markSemanticError(SemanticModel &Model) noexcept
    {
      Model.HasSemanticErrors = true;
    }

    static bool markUnsupported(SemanticModel &Model, ast::AstNodeRef Ref)
    {
      switch (Ref.Category)
      {
      case ast::AstNodeCategory::Declaration:
      {
        std::vector<bool>::reference Reported = Model.ReportedUnsupportedDecls[Model.declarationIndex(ast::AstDeclId::fromValue(Ref.Index))];
        if (Reported)
        {
          return false;
        }
        Reported = true;
        return true;
      }
      case ast::AstNodeCategory::Expression:
      {
        std::vector<bool>::reference Reported = Model.ReportedUnsupportedExprs[Model.expressionIndex(ast::AstExprId::fromValue(Ref.Index))];
        if (Reported)
        {
          return false;
        }
        Reported = true;
        return true;
      }
      case ast::AstNodeCategory::Statement:
      {
        std::vector<bool>::reference Reported = Model.ReportedUnsupportedStmts[Model.statementIndex(ast::AstStmtId::fromValue(Ref.Index))];
        if (Reported)
        {
          return false;
        }
        Reported = true;
        return true;
      }
      case ast::AstNodeCategory::Pattern:
      {
        std::vector<bool>::reference Reported = Model.ReportedUnsupportedPatterns[Model.patternIndex(ast::AstPatternId::fromValue(Ref.Index))];
        if (Reported)
        {
          return false;
        }
        Reported = true;
        return true;
      }
      case ast::AstNodeCategory::Unknown:
        break;
      }
      return false;
    }
  };

  inline core::SourceRange nodeRange(const SemanticModel &Model, ast::AstNodeRef Ref)
  {
    return Model.astContext().node(Ref).Header->Range;
  }

  inline void emitDiagnostic(SemanticModel &Model, std::vector<core::Diagnostic> &Diagnostics, core::DiagnosticKind Kind, core::SourceRange Range, std::optional<std::string> Expected = std::nullopt, std::optional<std::string> Actual = std::nullopt, std::optional<ast::AstDeclId> Previous = std::nullopt)
  {
    core::DiagnosticBuilder Builder(Kind, Model.sourceFile(), Range);
    if (Expected)
    {
      Builder.argument(core::DiagnosticArgumentName::Expected, std::move(*Expected));
    }
    if (Actual)
    {
      Builder.argument(core::DiagnosticArgumentName::Actual, std::move(*Actual));
    }
    if (Previous)
    {
      Builder.related(core::DiagnosticRelatedKind::PreviousDefinition, Model.sourceFile(), Model.astContext().declaration(*Previous).Header.Range);
    }
    Diagnostics.push_back(std::move(Builder).build());
    SemanticModelAccess::markSemanticError(Model);
  }

  inline std::string typeName(const SemanticModel &Model, type::TypeId Id)
  {
    if (!Model.typeContext().contains(Id))
    {
      return "<invalid>";
    }
    const type::Type &Value = Model.typeContext().type(Id);
    if (Value.Kind != type::TypeKind::Function)
    {
      return type::typeKindName(Value.Kind);
    }
    const type::FunctionType &Function = Model.typeContext().function(Id);
    std::string Result = "func(";
    for (std::size_t Index = 0; Index < Function.Parameters.size(); ++Index)
    {
      if (Index != 0)
      {
        Result += ", ";
      }
      Result += typeName(Model, Function.Parameters[Index]);
    }
    Result += ") -> ";
    Result += typeName(Model, Function.Result);
    return Result;
  }

  inline std::vector<SymbolId> lookupLocal(const SemanticModel &Model, ScopeId Scope, core::InternedStringId Name)
  {
    std::vector<SymbolId> Matches;
    for (const SymbolId Id : Model.scope(Scope).Symbols)
    {
      if (Model.symbol(Id).Name == Name)
      {
        Matches.push_back(Id);
      }
    }
    return Matches;
  }

  inline std::vector<SymbolId> lookupLexical(const SemanticModel &Model, ScopeId Scope, core::InternedStringId Name)
  {
    while (Model.contains(Scope))
    {
      std::vector<SymbolId> Matches = lookupLocal(Model, Scope, Name);
      if (!Matches.empty())
      {
        return Matches;
      }
      const std::optional<ScopeId> Parent = Model.scope(Scope).Parent;
      if (!Parent)
      {
        break;
      }
      Scope = *Parent;
    }
    return {};
  }

  inline void appendNodeList(const ast::AstContext &Context, ast::AstNodeList List, std::vector<ast::AstNodeRef> &Result)
  {
    for (const ast::AstNodeRef Ref : Context.list(List))
    {
      Result.push_back(Ref);
    }
  }

  inline std::vector<ast::AstNodeRef> directChildren(const SemanticModel &Model, ast::AstNodeRef Ref)
  {
    const ast::AstContext &Context = Model.astContext();
    std::vector<ast::AstNodeRef> Result;
    const ast::AstNodeView View = Context.node(Ref);
    appendNodeList(Context, View.Header->Supplemental, Result);
    if (Ref.Category == ast::AstNodeCategory::Declaration)
    {
      const ast::Declaration &Node = Context.declaration(ast::AstDeclId::fromValue(Ref.Index));
      if (const ast::SourceFilePayload *SourcePayload = std::get_if<ast::SourceFilePayload>(&Node.Payload))
      {
        appendNodeList(Context, SourcePayload->Items, Result);
      }
      else if (const ast::ErrorPayload *ErrorPayload = std::get_if<ast::ErrorPayload>(&Node.Payload))
      {
        appendNodeList(Context, ErrorPayload->Recovered, Result);
      }
      else if (const ast::BindingPayload *BindingPayload = std::get_if<ast::BindingPayload>(&Node.Payload))
      {
        Result.push_back(ast::AstNodeRef::pattern(BindingPayload->Pattern));
        if (BindingPayload->Type)
        {
          Result.push_back(ast::AstNodeRef::expression(*BindingPayload->Type));
        }
        if (BindingPayload->Initializer)
        {
          Result.push_back(ast::AstNodeRef::expression(*BindingPayload->Initializer));
        }
      }
      else if (const ast::ParameterPayload *ParameterPayload = std::get_if<ast::ParameterPayload>(&Node.Payload))
      {
        Result.push_back(ast::AstNodeRef::expression(ParameterPayload->Type));
        if (ParameterPayload->DefaultValue)
        {
          Result.push_back(ast::AstNodeRef::expression(*ParameterPayload->DefaultValue));
        }
      }
      else if (const ast::FunctionPayload *FunctionPayload = std::get_if<ast::FunctionPayload>(&Node.Payload))
      {
        appendNodeList(Context, FunctionPayload->Parameters, Result);
        if (FunctionPayload->ResultType)
        {
          Result.push_back(ast::AstNodeRef::expression(*FunctionPayload->ResultType));
        }
        if (FunctionPayload->Body)
        {
          Result.push_back(ast::AstNodeRef::statement(*FunctionPayload->Body));
        }
      }
      else if (const ast::UnsupportedPayload *UnsupportedPayload = std::get_if<ast::UnsupportedPayload>(&Node.Payload))
      {
        appendNodeList(Context, UnsupportedPayload->Children, Result);
      }
      return Result;
    }
    if (Ref.Category == ast::AstNodeCategory::Expression)
    {
      const ast::Expression &Node = Context.expression(ast::AstExprId::fromValue(Ref.Index));
      if (const ast::ErrorPayload *ErrorPayload = std::get_if<ast::ErrorPayload>(&Node.Payload))
      {
        appendNodeList(Context, ErrorPayload->Recovered, Result);
      }
      else if (const ast::GroupPayload *GroupPayload = std::get_if<ast::GroupPayload>(&Node.Payload))
      {
        Result.push_back(ast::AstNodeRef::expression(GroupPayload->Value));
      }
      else if (const ast::UnaryPayload *UnaryPayload = std::get_if<ast::UnaryPayload>(&Node.Payload))
      {
        Result.push_back(ast::AstNodeRef::expression(UnaryPayload->Operand));
      }
      else if (const ast::BinaryPayload *BinaryPayload = std::get_if<ast::BinaryPayload>(&Node.Payload))
      {
        Result.push_back(ast::AstNodeRef::expression(BinaryPayload->Left));
        Result.push_back(ast::AstNodeRef::expression(BinaryPayload->Right));
      }
      else if (const ast::CallPayload *CallPayload = std::get_if<ast::CallPayload>(&Node.Payload))
      {
        Result.push_back(ast::AstNodeRef::expression(CallPayload->Callee));
        appendNodeList(Context, CallPayload->Arguments, Result);
      }
      else if (const ast::IfExpressionPayload *IfPayload = std::get_if<ast::IfExpressionPayload>(&Node.Payload))
      {
        Result.push_back(ast::AstNodeRef::expression(IfPayload->Condition));
        Result.push_back(ast::AstNodeRef::expression(IfPayload->ThenValue));
        Result.push_back(ast::AstNodeRef::expression(IfPayload->ElseValue));
      }
      else if (const ast::FunctionTypePayload *FunctionTypePayload = std::get_if<ast::FunctionTypePayload>(&Node.Payload))
      {
        appendNodeList(Context, FunctionTypePayload->Parameters, Result);
        if (FunctionTypePayload->Result)
        {
          Result.push_back(ast::AstNodeRef::expression(*FunctionTypePayload->Result));
        }
      }
      else if (const ast::UnsupportedPayload *UnsupportedPayload = std::get_if<ast::UnsupportedPayload>(&Node.Payload))
      {
        appendNodeList(Context, UnsupportedPayload->Children, Result);
      }
      return Result;
    }
    if (Ref.Category == ast::AstNodeCategory::Statement)
    {
      const ast::Statement &Node = Context.statement(ast::AstStmtId::fromValue(Ref.Index));
      if (const ast::ErrorPayload *ErrorPayload = std::get_if<ast::ErrorPayload>(&Node.Payload))
      {
        appendNodeList(Context, ErrorPayload->Recovered, Result);
      }
      else if (const ast::BlockPayload *BlockPayload = std::get_if<ast::BlockPayload>(&Node.Payload))
      {
        appendNodeList(Context, BlockPayload->Items, Result);
      }
      else if (const ast::AssignmentPayload *AssignmentPayload = std::get_if<ast::AssignmentPayload>(&Node.Payload))
      {
        Result.push_back(ast::AstNodeRef::expression(AssignmentPayload->Left));
        Result.push_back(ast::AstNodeRef::expression(AssignmentPayload->Right));
      }
      else if (const ast::ExpressionStatementPayload *ExpressionPayload = std::get_if<ast::ExpressionStatementPayload>(&Node.Payload))
      {
        Result.push_back(ast::AstNodeRef::expression(ExpressionPayload->Value));
      }
      else if (const ast::IfStatementPayload *IfPayload = std::get_if<ast::IfStatementPayload>(&Node.Payload))
      {
        Result.push_back(IfPayload->Condition);
        Result.push_back(ast::AstNodeRef::statement(IfPayload->ThenBlock));
        if (IfPayload->ElseBlock)
        {
          Result.push_back(ast::AstNodeRef::statement(*IfPayload->ElseBlock));
        }
      }
      else if (const ast::WhileStatementPayload *WhilePayload = std::get_if<ast::WhileStatementPayload>(&Node.Payload))
      {
        Result.push_back(WhilePayload->Condition);
        Result.push_back(ast::AstNodeRef::statement(WhilePayload->Body));
      }
      else if (const ast::ReturnPayload *ReturnPayload = std::get_if<ast::ReturnPayload>(&Node.Payload))
      {
        if (ReturnPayload->Value)
        {
          Result.push_back(ast::AstNodeRef::expression(*ReturnPayload->Value));
        }
      }
      else if (const ast::UnsupportedPayload *UnsupportedPayload = std::get_if<ast::UnsupportedPayload>(&Node.Payload))
      {
        appendNodeList(Context, UnsupportedPayload->Children, Result);
      }
      return Result;
    }
    if (Ref.Category == ast::AstNodeCategory::Pattern)
    {
      const ast::Pattern &Node = Context.pattern(ast::AstPatternId::fromValue(Ref.Index));
      if (const ast::ErrorPayload *ErrorPayload = std::get_if<ast::ErrorPayload>(&Node.Payload))
      {
        appendNodeList(Context, ErrorPayload->Recovered, Result);
      }
      else if (const ast::TuplePatternPayload *TuplePayload = std::get_if<ast::TuplePatternPayload>(&Node.Payload))
      {
        appendNodeList(Context, TuplePayload->Elements, Result);
      }
      else if (const ast::VariantPatternPayload *VariantPayload = std::get_if<ast::VariantPatternPayload>(&Node.Payload))
      {
        appendNodeList(Context, VariantPayload->Elements, Result);
      }
      else if (const ast::UnsupportedPayload *UnsupportedPayload = std::get_if<ast::UnsupportedPayload>(&Node.Payload))
      {
        appendNodeList(Context, UnsupportedPayload->Children, Result);
      }
    }
    return Result;
  }
} // namespace ink::sema

#endif
