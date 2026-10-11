#pragma once

#include "ink/frontend/parser/ast/binding_pattern.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class MissingBindingPattern final : public BindingPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::MissingBindingPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      MissingBindingPattern(SourceRange Range)
          : BindingPattern(Kind, Range)
      {
      }

    private:
  };
} // namespace ink::frontend::parser
