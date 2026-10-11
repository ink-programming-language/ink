#pragma once

#include "ink/frontend/parser/ast/types.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"

namespace ink::frontend::parser
{
  class SwitchClause
  {
    public:
      SwitchClause(bool Default, Expr *Value, ASTArray<Stmt *> Statements, SourceRange Range)
          : Default(Default),
            Value(Value),
            Statements(Statements),
            Range(Range)
      {
      }
      bool isDefault() const noexcept
      {
        return Default;
      }
      Expr *value() noexcept
      {
        return Value;
      }
      const Expr *value() const noexcept
      {
        return Value;
      }
      ASTArray<Stmt *> statements() noexcept
      {
        return Statements;
      }
      ConstNodeArray<Stmt> statements() const noexcept
      {
        return ConstNodeArray<Stmt>(Statements);
      }
      SourceRange range() const noexcept
      {
        return Range;
      }

    private:
      bool Default;
      Expr *Value;
      ASTArray<Stmt *> Statements;
      SourceRange Range;
  };
} // namespace ink::frontend::parser
