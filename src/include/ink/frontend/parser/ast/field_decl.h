#pragma once

#include "ink/frontend/parser/ast/decl.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/attribute.h"
#include "ink/frontend/parser/ast/parameter.h"

namespace ink::frontend::parser
{
  class FieldDecl final : public Decl
  {
    public:
      static constexpr ASTKind Kind = ASTKind::FieldDecl;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      FieldDecl(SourceRange Range, ASTArray<Attribute> Attributes, NameToken Name, FieldTailKind Tail, TypeSyntax *Type, ASTArray<Parameter> Payload, Expr *Initializer)
          : Decl(Kind, Range),
            Attributes(Attributes),
            Name(Name),
            Tail(Tail),
            Type(Type),
            Payload(Payload),
            Initializer(Initializer)
      {
      }
      ASTArray<Attribute> attributes() const noexcept
      {
        return Attributes;
      }
      NameToken name() const noexcept
      {
        return Name;
      }
      FieldTailKind tail() const noexcept
      {
        return Tail;
      }
      TypeSyntax *type() noexcept
      {
        return Type;
      }
      const TypeSyntax *type() const noexcept
      {
        return Type;
      }
      ASTArray<Parameter> payload() const noexcept
      {
        return Payload;
      }
      Expr *initializer() noexcept
      {
        return Initializer;
      }
      const Expr *initializer() const noexcept
      {
        return Initializer;
      }

    private:
      ASTArray<Attribute> Attributes;
      NameToken Name;
      FieldTailKind Tail;
      TypeSyntax *Type;
      ASTArray<Parameter> Payload;
      Expr *Initializer;
  };
} // namespace ink::frontend::parser
