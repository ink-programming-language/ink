#pragma once

#include "ink/frontend/parser/ast/expr.h"
#include "ink/frontend/parser/ast/fwd.h"
#include <optional>
#include <utility>

namespace ink::frontend::parser
{
  class UnaryExpr final : public Expr
  {
    public:
      static constexpr ASTKind Kind = ASTKind::UnaryExpr;
      static bool classof(const ASTNodeBase *Node)
      {
        return Node && Node->getKind() == Kind;
      }
      UnaryExpr(SourceRange Range, TokenKind Op, SourceRange OperatorRange, Expr *Operand, std::optional<TypeQualifier> Qualifier = std::nullopt)
          : Expr(Kind, Range),
            Op(Op),
            OperatorRange(OperatorRange),
            Operand(Operand),
            Qualifier(Qualifier)
      {
      }
      TokenKind op() const noexcept
      {
        return Op;
      }
      SourceRange operatorRange() const noexcept
      {
        return OperatorRange;
      }
      // The qualifier belongs to this Star or Amp layer and restricts access to its pointee or referent.
      bool isConstQualified() const noexcept
      {
        return Qualifier && Qualifier->Kind == TypeQualifierKind::Const;
      }
      std::optional<TypeQualifier> qualifier() const noexcept
      {
        return Qualifier;
      }
      auto *operand(this auto &&Self) noexcept
      {
        return Self.Operand ? &std::forward_like<decltype(Self) &>(*Self.Operand) : nullptr;
      }

    private:
      TokenKind Op;
      SourceRange OperatorRange;
      Expr *Operand;
      std::optional<TypeQualifier> Qualifier;
  };
} // namespace ink::frontend::parser
