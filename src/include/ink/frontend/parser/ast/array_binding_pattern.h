#pragma once

#include "ink/frontend/parser/ast/binding_pattern.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include <optional>

namespace ink::frontend::parser
{
  class ArrayBindingPattern final : public BindingPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ArrayBindingPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ArrayBindingPattern(SourceRange Range, ASTArray<BindingPattern *> Elements, std::optional<RestBinding> Rest)
          : BindingPattern(Kind, Range),
            Elements(Elements),
            Rest(Rest)
      {
      }
      auto elements(this auto &&Self) noexcept
      {
        return NodeArrayFor<decltype(Self), BindingPattern>(Self.Elements);
      }
      std::optional<RestBinding> rest() const noexcept
      {
        return Rest;
      }

    private:
      ASTArray<BindingPattern *> Elements;
      std::optional<RestBinding> Rest;
  };
} // namespace ink::frontend::parser
