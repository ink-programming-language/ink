#pragma once

#include "ink/frontend/parser/ast/types.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/argument.h"

namespace ink::frontend::parser
{
  class Attribute
  {
    public:
      Attribute(ASTArray<NameToken> Path, AttributeSuffix Suffix, ASTArray<Argument> Arguments, Expr *Value, SourceRange Range)
          : Path(Path),
            Suffix(Suffix),
            Arguments(Arguments),
            Value(Value),
            Range(Range)
      {
      }
      ASTArray<NameToken> path() const noexcept
      {
        return Path;
      }
      AttributeSuffix suffix() const noexcept
      {
        return Suffix;
      }
      ASTArray<Argument> arguments() const noexcept
      {
        return Arguments;
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
      ASTArray<NameToken> Path;
      AttributeSuffix Suffix;
      ASTArray<Argument> Arguments;
      Expr *Value;
      SourceRange Range;
  };
} // namespace ink::frontend::parser
