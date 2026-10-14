#pragma once

#include "ink/frontend/parser/ast/match_pattern.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"

namespace ink::frontend::parser
{
  class OrMatchPattern final : public MatchPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::OrMatchPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      OrMatchPattern(SourceRange Range, ASTArray<MatchPattern *> Alternatives)
          : MatchPattern(Kind, Range),
            Alternatives(Alternatives)
      {
      }
      auto alternatives(this auto &&Self) noexcept
      {
        return NodeArrayFor<decltype(Self), MatchPattern>(Self.Alternatives);
      }

    private:
      ASTArray<MatchPattern *> Alternatives;
  };
} // namespace ink::frontend::parser
