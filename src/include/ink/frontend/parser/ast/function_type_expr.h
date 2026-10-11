#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/function_type_parameter.h"

namespace ink::frontend::parser
{
  class FunctionTypeExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::FunctionTypeExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      FunctionTypeExpr(SourceRange Range, ASTArray<FunctionTypeParameter> Parameters, TypeSyntax *ReturnType)
          : Expr(Kind, Range),
            Parameters(Parameters),
            ReturnType(ReturnType)
      {
      }
      ASTArray<FunctionTypeParameter> parameters() const noexcept
      {
        return Parameters;
      }
      TypeSyntax *returnType() noexcept
      {
        return ReturnType;
      }
      const TypeSyntax *returnType() const noexcept
      {
        return ReturnType;
      }

    private:
      ASTArray<FunctionTypeParameter> Parameters;
      TypeSyntax *ReturnType;
  };
} // namespace ink::frontend::parser
