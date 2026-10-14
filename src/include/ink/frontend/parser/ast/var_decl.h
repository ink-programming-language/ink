#pragma once

#include "ink/frontend/parser/ast/decl.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/attribute.h"
#include <utility>

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
      auto *binding(this auto &&Self) noexcept
      {
        return Self.Binding ? &std::forward_like<decltype(Self) &>(*Self.Binding) : nullptr;
      }
      auto *type(this auto &&Self) noexcept
      {
        return Self.Type ? &std::forward_like<decltype(Self) &>(*Self.Type) : nullptr;
      }
      auto *initializer(this auto &&Self) noexcept
      {
        return Self.Initializer ? &std::forward_like<decltype(Self) &>(*Self.Initializer) : nullptr;
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
