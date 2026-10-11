#pragma once

#include "ink/frontend/parser/ast/binding_pattern.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class NameBindingPattern final : public BindingPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::NameBindingPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      NameBindingPattern(SourceRange Range, NameToken Name)
          : BindingPattern(Kind, Range),
            Name(Name)
      {
      }
      NameToken name() const noexcept
      {
        return Name;
      }

    private:
      NameToken Name;
  };
} // namespace ink::frontend::parser
