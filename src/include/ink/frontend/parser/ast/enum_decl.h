#pragma once

#include "ink/frontend/parser/ast/decl.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/attribute.h"
#include "ink/frontend/parser/ast/base_spec.h"
#include "ink/frontend/parser/ast/parameter.h"
#include <utility>

namespace ink::frontend::parser
{
  class EnumDecl final : public Decl
  {
    public:
      static constexpr ASTKind Kind = ASTKind::EnumDecl;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      EnumDecl(SourceRange Range, ASTArray<Attribute> Attributes, NameToken Name, ASTArray<Parameter> GenericParameters, ASTArray<BaseSpec> Bases, AggregateForm Form, BlockStmt *Body, SourceRange SemicolonRange)
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
      auto *body(this auto &&Self) noexcept
      {
        return Self.Body ? &std::forward_like<decltype(Self) &>(*Self.Body) : nullptr;
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
