#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include "ink/frontend/parser/ast_context.h"
#include "ink/frontend/parser/ast/function_type_parameter.h"
#include <utility>

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
      auto *returnType(this auto &&Self) noexcept
      {
        return Self.ReturnType ? &std::forward_like<decltype(Self) &>(*Self.ReturnType) : nullptr;
      }

    private:
      ASTArray<FunctionTypeParameter> Parameters;
      TypeSyntax *ReturnType;
  };
} // namespace ink::frontend::parser
