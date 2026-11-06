#pragma once

#include "ink/core/support/hash.h"
#include "ink/ir/type/array_type.h"
#include "ink/ir/type/bool_type.h"
#include "ink/ir/type/float_type.h"
#include "ink/ir/type/function_type.h"
#include "ink/ir/type/integer_type.h"
#include "ink/ir/type/meta_type.h"
#include "ink/ir/type/object_type.h"
#include "ink/ir/type/pointer_type.h"
#include "ink/ir/type/struct_type.h"
#include "ink/ir/type/void_type.h"

#include <cstdint>
#include <functional>
#include <list>
#include <span>
#include <unordered_set>

namespace ink::ir
{
  // Owns types with stable addresses until pool destruction: interned immutable types and unique mutable record definitions.
  // Borrowed array elements, pointer targets, and signature types may be external and must remain alive while the pool is used.
  // Stop using the pool when any borrowed owner begins destruction; pool destruction does not access borrowed values.
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
      const MetaType &getMetaType() const noexcept;

      // Each call creates a distinct record definition; filling its body does not change its identity.
      StructType &createStructType();
      ObjectType &createObjectType();

      // Composite types are interned by borrowed type identities, parameter metadata, and function call ABI.
      // Passing a value whose type is not MetaType is a programming error.
      const ArrayType &getArrayType(ValueRef ElementType, std::uint64_t ElementCount);
      const PointerType &getPointerType(ValueRef PointeeType);
      const FunctionType &getFunctionType(ValueRef ReturnType, std::span<const Parameter> Parameters = {}, CallABI ABI = {});

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
      BoolType Bool = {};
      VoidType Void = {};
      std::unordered_set<ArrayType, core::Hash<ArrayType>, std::equal_to<>> Arrays = {};
      std::unordered_set<PointerType, core::Hash<PointerType>, std::equal_to<>> Pointers = {};
      std::unordered_set<FunctionType, core::Hash<FunctionType>, std::equal_to<>> Functions = {};
      // Destroy record-owned method bodies before their borrowed interned signature types.
      std::list<StructType> Structs = {};
      std::list<ObjectType> Objects = {};
  };
} // namespace ink::ir
