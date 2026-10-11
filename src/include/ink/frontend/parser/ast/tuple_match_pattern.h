#pragma once

#include "ink/frontend/parser/ast/match_pattern.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"

namespace ink::frontend::parser
{
  class TupleMatchPattern final : public MatchPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::TupleMatchPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      TupleMatchPattern(SourceRange Range, ASTArray<MatchPattern *> Elements)
          : MatchPattern(Kind, Range),
            Elements(Elements)
      {
      }
      ASTArray<MatchPattern *> elements() noexcept
      {
        return Elements;
      }
      ConstNodeArray<MatchPattern> elements() const noexcept
      {
        return ConstNodeArray<MatchPattern>(Elements);
      }

    private:
      ASTArray<MatchPattern *> Elements;
  };
} // namespace ink::frontend::parser
