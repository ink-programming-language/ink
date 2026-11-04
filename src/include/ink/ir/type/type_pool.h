#pragma once

#include "ink/core/support/hash.h"
#include "ink/ir/type/array_type.h"
#include "ink/ir/type/bool_type.h"
#include "ink/ir/type/float_type.h"
#include "ink/ir/type/integer_type.h"
#include "ink/ir/type/reference_type.h"
#include "ink/ir/type/void_type.h"

#include <cstdint>
#include <functional>
#include <unordered_set>

namespace ink::ir
{
  // Owns immutable types with stable addresses until pool destruction.
  // Array elements and reference targets may be external, but must outlive the pool.
  // Concurrent pool operations require external synchronization.
  class TypePool final
  {
    public:
      TypePool() = default;
      TypePool(const TypePool &) = delete;
      TypePool &operator=(const TypePool &) = delete;
      TypePool(TypePool &&) = delete;
      TypePool &operator=(TypePool &&) = delete;

      // Width and Sign must be declared enumerators; invalid values are programming errors.
      const IntegerType &getIntegerType(IntegerType::Width Width, IntegerType::Signedness Sign) const noexcept;
      const FloatType &getFloatType(FloatType::Width Width) const noexcept;
      const BoolType &getBoolType() const noexcept;
      const VoidType &getVoidType() const noexcept;

      // Composite types are interned by the identity of the borrowed type and their parameters.
      const ArrayType &getArrayType(const Type &ElementType, std::uint64_t ElementCount);
      const ArrayType &getArrayType(const Type &&, std::uint64_t) = delete;
      const ReferenceType &getReferenceType(const Type &ReferencedType);
      const ReferenceType &getReferenceType(const Type &&) = delete;

    private:
      IntegerType Integers[8] = {
          IntegerType(IntegerType::Width::Bits8, IntegerType::Signedness::Signed),
          IntegerType(IntegerType::Width::Bits8, IntegerType::Signedness::Unsigned),
          IntegerType(IntegerType::Width::Bits16, IntegerType::Signedness::Signed),
          IntegerType(IntegerType::Width::Bits16, IntegerType::Signedness::Unsigned),
          IntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Signed),
          IntegerType(IntegerType::Width::Bits32, IntegerType::Signedness::Unsigned),
          IntegerType(IntegerType::Width::Bits64, IntegerType::Signedness::Signed),
          IntegerType(IntegerType::Width::Bits64, IntegerType::Signedness::Unsigned),
      };
      FloatType Floats[2] = {
          FloatType(FloatType::Width::Bits32),
          FloatType(FloatType::Width::Bits64),
      };
      BoolType Bool;
      VoidType Void;
      std::unordered_set<ArrayType, core::Hash<ArrayType>, std::equal_to<>> Arrays;
      std::unordered_set<ReferenceType, core::Hash<ReferenceType>, std::equal_to<>> References;
  };
} // namespace ink::ir
