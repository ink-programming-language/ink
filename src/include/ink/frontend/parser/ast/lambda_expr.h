#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/parameter.h"
#include <utility>

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
      auto *returnType(this auto &&Self) noexcept
      {
        return Self.ReturnType ? &std::forward_like<decltype(Self) &>(*Self.ReturnType) : nullptr;
      }
      auto *body(this auto &&Self) noexcept
      {
        return Self.Body ? &std::forward_like<decltype(Self) &>(*Self.Body) : nullptr;
      }

    private:
      ASTArray<Parameter> GenericParameters;
      ASTArray<Parameter> Parameters;
      TypeSyntax *ReturnType;
      BlockStmt *Body;
  };
} // namespace ink::frontend::parser
