#pragma once

#include "ink/frontend/parser/ast/binding_pattern.h"
#include "ink/frontend/parser/ast/fwd.h"

namespace ink::frontend::parser
{
  class ErrorBindingPattern final : public BindingPattern
  {
    public:
      static constexpr ASTKind Kind = ASTKind::ErrorBindingPattern;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      ErrorBindingPattern(SourceRange Range)
          : BindingPattern(Kind, Range)
      {
      }

    private:
  };
} // namespace ink::frontend::parser
