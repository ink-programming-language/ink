#ifndef INK_SEMANTIC_MODEL_CONSTANT_H
#define INK_SEMANTIC_MODEL_CONSTANT_H

#include "ink/semantic/model/float_bits.h"
#include "ink/semantic/model/integer_bits.h"
#include "ink/semantic/model/type/float_type.h"
#include "ink/semantic/model/type/integer_type.h"
#include "ink/semantic/model/type/slice_type.h"

#include <string>
#include <string_view>
#include <utility>

namespace ink::semantic
{
  class ConstantPool;

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
        if (!ValueObject)
        {
          return false;
        }
        switch (ValueObject->kind())
        {
#define INK_SEMANTIC_CONSTANT_VALUE(Name) case ValueKind::Name:
#include "ink/semantic/model/Values.def"
          return true;
        default:
          return false;
        }
      }

    protected:
      Constant(ValueKind Kind, const Type &ValueType) noexcept
          : Value(ValueType.context(), Kind),
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
      const IntegerBits &value() const noexcept
      {
        return Payload;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::IntegerConstant;
      }

    private:
      IntegerConstant(const IntegerType &ValueType, IntegerBits Payload)
          : Constant(ValueKind::IntegerConstant, ValueType),
            Payload(std::move(Payload))
      {
      }

      IntegerBits Payload;

      friend class ConstantPool;
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

      friend class ConstantPool;
  };

  class StringConst final : public Constant
  {
    public:
      // Owned decoded UTF-8 bytes, including embedded NULs; no terminator is added to the value.
      std::string_view value() const noexcept
      {
        return Payload;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::StringConst;
      }

    private:
      StringConst(const SliceType &ValueType, std::string_view Payload)
          : Constant(ValueKind::StringConst, ValueType),
            Payload(Payload)
      {
      }

      std::string Payload;

      friend class ConstantPool;
  };

  class FloatConst final : public Constant
  {
    public:
      // Exact IEEE binary16/32/64 payload, including signed zero and NaN payload bits.
      const FloatBits &value() const noexcept
      {
        return Payload;
      }

      static bool classof(const Value *ValueObject) noexcept
      {
        return ValueObject && ValueObject->kind() == ValueKind::FloatConst;
      }

    private:
      FloatConst(const FloatType &ValueType, FloatBits Payload) noexcept
          : Constant(ValueKind::FloatConst, ValueType),
            Payload(Payload)
      {
      }

      FloatBits Payload;

      friend class ConstantPool;
  };
} // namespace ink::semantic

#endif
