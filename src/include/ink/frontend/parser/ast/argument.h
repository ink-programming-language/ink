#pragma once

#include "ink/frontend/parser/ast/types.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <optional>

namespace ink::frontend::parser
{
  class Argument
  {
    public:
      Argument(ArgumentKind Form, std::optional<NameToken> Name, Expr *Value, SourceRange Range)
          : Form(Form),
            Name(Name),
            Value(Value),
            Range(Range)
      {
      }
      ArgumentKind form() const noexcept
      {
        return Form;
      }
      std::optional<NameToken> name() const noexcept
      {
        return Name;
      }
      Expr *value() noexcept
      {
        return Value;
      }
      const Expr *value() const noexcept
      {
        return Value;
      }
      SourceRange range() const noexcept
      {
        return Range;
      }

    private:
      ArgumentKind Form;
      std::optional<NameToken> Name;
      Expr *Value;
      SourceRange Range;
  };
} // namespace ink::frontend::parser
