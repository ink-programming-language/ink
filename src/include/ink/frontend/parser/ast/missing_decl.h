#pragma once

#include "ink/frontend/parser/ast/decl.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/attribute.h"

namespace ink::frontend::parser
{
  class MissingDecl final : public Decl
  {
    public:
      static constexpr ASTKind Kind = ASTKind::MissingDecl;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      MissingDecl(SourceRange Range, ASTArray<Attribute> Attributes)
          : Decl(Kind, Range),
            Attributes(Attributes)
      {
      }
      ASTArray<Attribute> attributes() const noexcept
      {
        return Attributes;
      }

    private:
      ASTArray<Attribute> Attributes;
  };
} // namespace ink::frontend::parser
