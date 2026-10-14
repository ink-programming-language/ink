#pragma once

#include "ink/frontend/parser/ast/match_pattern.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include <optional>

namespace ink::frontend::parser
{
  class ArrayMatchPattern final : public MatchPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ArrayMatchPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ArrayMatchPattern(SourceRange Range, ASTArray<MatchPattern *> Elements, std::optional<RestBinding> Rest)
          : MatchPattern(Kind, Range),
            Elements(Elements),
            Rest(Rest)
      {
      }
      auto elements(this auto &&Self) noexcept
      {
        return NodeArrayFor<decltype(Self), MatchPattern>(Self.Elements);
      }
      std::optional<RestBinding> rest() const noexcept
      {
        return Rest;
      }

    private:
      ASTArray<MatchPattern *> Elements;
      std::optional<RestBinding> Rest;
  };
} // namespace ink::frontend::parser
