#pragma once

#include "ink/frontend/parser/ast/types.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/argument.h"
#include <utility>

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
      auto *value(this auto &&Self) noexcept
      {
        return Self.Value ? &std::forward_like<decltype(Self) &>(*Self.Value) : nullptr;
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
