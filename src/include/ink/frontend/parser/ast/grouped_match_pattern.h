#pragma once

#include "ink/frontend/parser/ast/match_pattern.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class GroupedMatchPattern final : public MatchPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::GroupedMatchPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      GroupedMatchPattern(SourceRange Range, MatchPattern *Pattern)
          : MatchPattern(Kind, Range),
            Pattern(Pattern)
      {
      }
      MatchPattern *pattern() noexcept
      {
        return Pattern;
      }
      const MatchPattern *pattern() const noexcept
      {
        return Pattern;
      }

    private:
      MatchPattern *Pattern;
  };
} // namespace ink::frontend::parser
