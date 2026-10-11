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
      ASTArray<BindingPattern *> elements() noexcept
      {
        return Elements;
      }
      ConstNodeArray<BindingPattern> elements() const noexcept
      {
        return ConstNodeArray<BindingPattern>(Elements);
      }

    private:
      ASTArray<BindingPattern *> Elements;
  };
} // namespace ink::frontend::parser
