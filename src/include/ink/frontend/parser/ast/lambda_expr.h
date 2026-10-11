#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/parameter.h"

namespace ink::frontend::parser
{
  class LambdaExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::LambdaExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      LambdaExpr(SourceRange Range, ASTArray<Parameter> GenericParameters, ASTArray<Parameter> Parameters, TypeSyntax *ReturnType, BlockStmt *Body)
          : Expr(Kind, Range),
            GenericParameters(GenericParameters),
            Parameters(Parameters),
            ReturnType(ReturnType),
            Body(Body)
      {
      }
      ASTArray<Parameter> genericParameters() const noexcept
      {
        return GenericParameters;
      }
      ASTArray<Parameter> parameters() const noexcept
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
      BlockStmt *body() noexcept
      {
        return Body;
      }
      const BlockStmt *body() const noexcept
      {
        return Body;
      }

    private:
      ASTArray<Parameter> GenericParameters;
      ASTArray<Parameter> Parameters;
      TypeSyntax *ReturnType;
      BlockStmt *Body;
  };
} // namespace ink::frontend::parser
