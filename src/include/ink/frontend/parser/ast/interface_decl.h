#pragma once

#include "ink/frontend/parser/ast/decl.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/attribute.h"
#include "ink/frontend/parser/ast/base_spec.h"
#include "ink/frontend/parser/ast/parameter.h"

namespace ink::frontend::parser
{
  class InterfaceDecl final : public Decl
  {
    public:
      static constexpr ASTKind Kind = ASTKind::InterfaceDecl;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      InterfaceDecl(SourceRange Range, ASTArray<Attribute> Attributes, NameToken Name, ASTArray<Parameter> GenericParameters, ASTArray<BaseSpec> Bases, AggregateForm Form, BlockStmt *Body, SourceRange SemicolonRange)
          : Decl(Kind, Range),
            Attributes(Attributes),
            Name(Name),
            GenericParameters(GenericParameters),
            Bases(Bases),
            Form(Form),
            Body(Body),
            SemicolonRange(SemicolonRange)
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
      ASTArray<Parameter> genericParameters() const noexcept
      {
        return GenericParameters;
      }
      ASTArray<BaseSpec> bases() const noexcept
      {
        return Bases;
      }
      AggregateForm form() const noexcept
      {
        return Form;
      }
      BlockStmt *body() noexcept
      {
        return Body;
      }
      const BlockStmt *body() const noexcept
      {
        return Body;
      }
      SourceRange semicolonRange() const noexcept
      {
        return SemicolonRange;
      }

    private:
      ASTArray<Attribute> Attributes;
      NameToken Name;
      ASTArray<Parameter> GenericParameters;
      ASTArray<BaseSpec> Bases;
      AggregateForm Form;
      BlockStmt *Body;
      SourceRange SemicolonRange;
  };
} // namespace ink::frontend::parser
