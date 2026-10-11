#pragma once

#include "ink/frontend/parser/ast/types.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/argument.h"

namespace ink::frontend::parser
{
  class PathSegment
  {
    public:
      PathSegment(NameToken Name, bool HasGenericArguments, ASTArray<Argument> Arguments, SourceRange Range)
          : Name(Name),
            HasGenericArguments(HasGenericArguments),
            Arguments(Arguments),
            Range(Range)
      {
      }
      NameToken name() const noexcept
      {
        return Name;
      }
      bool hasGenericArguments() const noexcept
      {
        return HasGenericArguments;
      }
      ASTArray<Argument> arguments() const noexcept
      {
        return Arguments;
      }
      SourceRange range() const noexcept
      {
        return Range;
      }

    private:
      NameToken Name;
      bool HasGenericArguments;
      ASTArray<Argument> Arguments;
      SourceRange Range;
  };
} // namespace ink::frontend::parser
