#pragma once

#include "ink/frontend/parser/ast/match_pattern.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class WildcardMatchPattern final : public MatchPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::WildcardMatchPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      WildcardMatchPattern(SourceRange Range)
          : MatchPattern(Kind, Range)
      {
      }

    private:
  };
} // namespace ink::frontend::parser
