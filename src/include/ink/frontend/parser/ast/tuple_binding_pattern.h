#pragma once

#include "ink/frontend/parser/ast/binding_pattern.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"

namespace ink::frontend::parser
{
  class TupleBindingPattern final : public BindingPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::TupleBindingPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      TupleBindingPattern(SourceRange Range, ASTArray<BindingPattern *> Elements)
          : BindingPattern(Kind, Range),
            Elements(Elements)
      {
      }
      auto elements(this auto &&Self) noexcept
      {
        return NodeArrayFor<decltype(Self), BindingPattern>(Self.Elements);
      }

    private:
      ASTArray<BindingPattern *> Elements;
  };
} // namespace ink::frontend::parser
