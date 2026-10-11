#pragma once

#include "ink/frontend/parser/ast/ast_node_base.h"

namespace ink::frontend::parser
{
  class BindingPattern : public ASTNodeBase
  {
    public:
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && categoryOf(Node->getKind()) == ASTCategory::BindingPattern;
      }

    protected:
      using ASTNodeBase::ASTNodeBase;
  };
} // namespace ink::frontend::parser
