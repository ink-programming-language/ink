#pragma once

#include "ink/frontend/parser/ast/decl.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/attribute.h"

namespace ink::frontend::parser
{
  class VarDecl final : public Decl
  {
    public:
      static constexpr ASTKind Kind = ASTKind::VarDecl;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      VarDecl(SourceRange Range, ASTArray<Attribute> Attributes, bool Constant, BindingPattern *Binding, TypeSyntax *Type, Expr *Initializer, VarDeclForm Form)
          : Decl(Kind, Range),
            Attributes(Attributes),
            Constant(Constant),
            Binding(Binding),
            Type(Type),
            Initializer(Initializer),
            Form(Form)
      {
      }
      ASTArray<Attribute> attributes() const noexcept
      {
        return Attributes;
      }
      bool constant() const noexcept
      {
        return Constant;
      }
      BindingPattern *binding() noexcept
      {
        return Binding;
      }
      const BindingPattern *binding() const noexcept
      {
        return Binding;
      }
      TypeSyntax *type() noexcept
      {
        return Type;
      }
      const TypeSyntax *type() const noexcept
      {
        return Type;
      }
      Expr *initializer() noexcept
      {
        return Initializer;
      }
      const Expr *initializer() const noexcept
      {
        return Initializer;
      }
      VarDeclForm form() const noexcept
      {
        return Form;
      }

    private:
      ASTArray<Attribute> Attributes;
      bool Constant;
      BindingPattern *Binding;
      TypeSyntax *Type;
      Expr *Initializer;
      VarDeclForm Form;
  };
} // namespace ink::frontend::parser
