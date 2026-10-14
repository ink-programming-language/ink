#pragma once

#include "ink/frontend/parser/ast/match_pattern.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/path_segment.h"

namespace ink::frontend::parser
{
  class NameMatchPattern final : public MatchPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::NameMatchPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      NameMatchPattern(SourceRange Range, ASTArray<PathSegment> Path, bool HasArguments, ASTArray<MatchPattern *> Arguments)
          : MatchPattern(Kind, Range),
            Path(Path),
            HasArguments(HasArguments),
            Arguments(Arguments)
      {
      }
      ASTArray<PathSegment> path() const noexcept
      {
        return Path;
      }
      bool hasArguments() const noexcept
      {
        return HasArguments;
      }
      auto arguments(this auto &&Self) noexcept
      {
        return NodeArrayFor<decltype(Self), MatchPattern>(Self.Arguments);
      }

    private:
      ASTArray<PathSegment> Path;
      bool HasArguments;
      ASTArray<MatchPattern *> Arguments;
  };
} // namespace ink::frontend::parser
