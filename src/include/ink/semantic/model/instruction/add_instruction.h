#ifndef INK_SEMANTIC_MODEL_ADD_INSTRUCTION_H
#define INK_SEMANTIC_MODEL_ADD_INSTRUCTION_H

#include "ink/semantic/model/type/integer_type.h"

namespace ink::semantic
{
  // Integer addition modulo 2^bitWidth, for both signed and unsigned types.
  class AddInstruction final : public Value
  {
    public:
      const IntegerType &type() const noexcept override
      {
        return static_cast<const IntegerType &>(Left.type());
      }

      const Value &left() const noexcept
      {
        return Left;
      }

      const Value &right() const noexcept
      {
        return Right;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::AddInstruction;
      }

    private:
      AddInstruction(const Value &Left, const Value &Right) noexcept
          : Value(Left.context(), ValueKind::AddInstruction),
            Left(Left),
            Right(Right)
      {
      }

      const Value &Left;
      const Value &Right;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
