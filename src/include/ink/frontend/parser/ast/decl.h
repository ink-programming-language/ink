#pragma once

#include "ink/frontend/parser/ast/ast_node_base.h"

namespace ink::frontend::parser
{
  class Decl : public ASTNodeBase
  {
    public:
      DeclarationVisibility visibility() const noexcept
      {
        return Visibility;
      }
      void setVisibility(DeclarationVisibility Value) noexcept
      {
        Visibility = Value;
      }
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && categoryOf(Node->getKind()) == ASTCategory::Decl;
      }

    protected:
      using ASTNodeBase::ASTNodeBase;

    private:
      DeclarationVisibility Visibility = DeclarationVisibility::Default;
  };
} // namespace ink::frontend::parser
