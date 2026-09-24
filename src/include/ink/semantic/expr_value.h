#ifndef INK_SEMANTIC_EXPR_VALUE_H
#define INK_SEMANTIC_EXPR_VALUE_H

#include "ink/core/source_id.h"
#include "ink/semantic/type.h"

namespace ink::parser
{
  class Expr;
} // namespace ink::parser

namespace ink::semantic
{
  // A checked expression's semantic result, without executing the expression.
  // Its AST is borrowed and must outlive the owning SemanticContext. Each
  // creation has independent identity, even when the same AST is reused.
  class ExprValue final : public Value
  {
    public:
      const Type &type() const noexcept final
      {
        return ValueType;
      }

      const SemanticContext &context() const noexcept
      {
        return ValueType.context();
      }

      const parser::Expr &expression() const noexcept
      {
        return Expression;
      }

      core::SourceId sourceId() const noexcept
      {
        return Source;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::Expr;
      }

    private:
      ExprValue(const Type &ValueType, const parser::Expr &Expression, core::SourceId Source) noexcept
          : Value(ValueKind::Expr),
            ValueType(ValueType),
            Expression(Expression),
            Source(Source)
      {
      }

      const Type &ValueType;
      const parser::Expr &Expression;
      core::SourceId Source;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
