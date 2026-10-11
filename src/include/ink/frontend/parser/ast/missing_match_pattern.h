#pragma once

#include "ink/frontend/parser/ast/match_pattern.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class MissingMatchPattern final : public MatchPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::MissingMatchPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      MissingMatchPattern(SourceRange Range)
          : MatchPattern(Kind, Range)
      {
      }

    private:
  };
} // namespace ink::frontend::parser
