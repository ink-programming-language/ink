#ifndef INK_SEMANTIC_CONSTANT_H
#define INK_SEMANTIC_CONSTANT_H

#include "ink/semantic/type.h"

#include <llvm/ADT/APInt.h>

#include <utility>

namespace ink::semantic
{
  // Concrete immutable data constants. A Type is also a compile-time value,
  // so Constant membership alone does not describe evaluation availability.
  class Constant : public Value
  {
    public:
      const Type &type() const noexcept final
      {
        return ValueType;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && (ValueObject->kind() == ValueKind::IntegerConstant || ValueObject->kind() == ValueKind::BoolConstant);
      }

    protected:
      Constant(ValueKind Kind, const Type &ValueType) noexcept
          : Value(Kind),
            ValueType(ValueType)
      {
      }

    private:
      const Type &ValueType;
  };

  class IntegerConstant final : public Constant
  {
    public:
      // Bits have exactly the declared integer width; signedness is in the type.
      const llvm::APInt &value() const noexcept
      {
        return Payload;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::IntegerConstant;
      }

    private:
      IntegerConstant(const IntegerType &ValueType, llvm::APInt Payload)
          : Constant(ValueKind::IntegerConstant, ValueType),
            Payload(std::move(Payload))
      {
      }

      llvm::APInt Payload;

      friend class SemanticContext;
  };

  class BoolConstant final : public Constant
  {
    public:
      bool value() const noexcept
      {
        return Payload;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::BoolConstant;
      }

    private:
      BoolConstant(const Type &ValueType, bool Payload) noexcept
          : Constant(ValueKind::BoolConstant, ValueType),
            Payload(Payload)
      {
      }

      bool Payload;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
