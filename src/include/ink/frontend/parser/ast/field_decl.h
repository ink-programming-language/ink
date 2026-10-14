#pragma once

#include "ink/frontend/parser/ast/decl.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/attribute.h"
#include "ink/frontend/parser/ast/parameter.h"
#include <utility>

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
      auto *type(this auto &&Self) noexcept
      {
        return Self.Type ? &std::forward_like<decltype(Self) &>(*Self.Type) : nullptr;
      }
      ASTArray<Parameter> payload() const noexcept
      {
        return Payload;
      }
      auto *initializer(this auto &&Self) noexcept
      {
        return Self.Initializer ? &std::forward_like<decltype(Self) &>(*Self.Initializer) : nullptr;
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
