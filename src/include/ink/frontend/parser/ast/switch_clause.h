#pragma once

#include "ink/frontend/parser/ast/types.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include <utility>

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
      auto *value(this auto &&Self) noexcept
      {
        return Self.Value ? &std::forward_like<decltype(Self) &>(*Self.Value) : nullptr;
      }
      auto statements(this auto &&Self) noexcept
      {
        return NodeArrayFor<decltype(Self), Stmt>(Self.Statements);
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
