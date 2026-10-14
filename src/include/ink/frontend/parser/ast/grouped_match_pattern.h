#pragma once

#include "ink/frontend/parser/ast/match_pattern.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <utility>

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
      auto *pattern(this auto &&Self) noexcept
      {
        return Self.Pattern ? &std::forward_like<decltype(Self) &>(*Self.Pattern) : nullptr;
      }

    private:
      MatchPattern *Pattern;
  };
} // namespace ink::frontend::parser
