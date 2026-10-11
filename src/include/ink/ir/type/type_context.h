#ifndef INK_IR_TYPE_TYPE_CONTEXT_H
#define INK_IR_TYPE_TYPE_CONTEXT_H

#include "ink/target/target_context.h"
#include "ink/ir/type/array_type.h"
#include "ink/ir/type/bool_type.h"
#include "ink/ir/type/float_type.h"
#include "ink/ir/type/function_type.h"
#include "ink/ir/type/integer_type.h"
#include "ink/ir/type/pointer_type.h"
#include "ink/ir/type/reference_type.h"
#include "ink/ir/type/string_type.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ink::ir
{
  // Owns immutable types for one fixed target. All returned pointers are borrowed
  // and remain valid until this context is destroyed; IR users must die first.
  // A context is not thread-safe and types from different contexts cannot be mixed.
  class TypeContext final
  {
    public:
      explicit TypeContext(target::TargetContext Target);
      ~TypeContext() noexcept;

      TypeContext(const TypeContext &) = delete;
      TypeContext &operator=(const TypeContext &) = delete;
      TypeContext(TypeContext &&) = delete;
      TypeContext &operator=(TypeContext &&) = delete;

      const target::TargetContext &targetContext() const noexcept;
      bool owns(const Type *TypeValue) const noexcept;

      const BoolType *getBoolType() const noexcept;
      const IntegerType *getInt32Type() const noexcept;
      const IntegerType *getUInt32Type() const noexcept;
      const IntegerType *getInt64Type() const noexcept;
      const IntegerType *getUInt64Type() const noexcept;
      const FloatType *getFloat32Type() const noexcept;
      const FloatType *getFloat64Type() const noexcept;
      const StringType *getStringType() const noexcept;

      // Width is in bits; unsupported widths return nullptr.
      const IntegerType *getIntegerType(std::uint32_t Width, bool Signed) const noexcept;
      const FloatType *getFloatType(std::uint32_t Width) const noexcept;

      // Structural types are reused. Null or foreign-context operands return nullptr.
      const PointerType *getPointerType(const Type *PointeeType);
      const ReferenceType *getReferenceType(const Type *ReferencedType);
      // Missing/invalid element layout or size overflow also returns nullptr.
      const ArrayType *getArrayType(const Type *ElementType, std::uint64_t ElementCount);
      // All types must have runtime layouts. Names and variadic flags are part of
      // the cache key because the current FunctionType preserves this metadata.
      const FunctionType *getFunctionType(const Type *ReturnType, std::vector<Parameter> Parameters = {});

    private:
      struct FunctionKey
      {
          const Type *ReturnType;
          std::vector<Parameter> Parameters;

          bool operator==(const FunctionKey &Other) const noexcept;
      };

      struct FunctionKeyHash
      {
          std::size_t operator()(const FunctionKey &Key) const noexcept;
      };

      template <typename TypeT>
      const TypeT *own(std::unique_ptr<TypeT> TypeValue);

      bool validParameters(const std::vector<Parameter> &Parameters) const noexcept;

      const target::TargetContext Target;
      BoolType Bool;
      IntegerType Int32;
      IntegerType UInt32;
      IntegerType Int64;
      IntegerType UInt64;
      FloatType Float32;
      FloatType Float64;
      StringType String;
      std::vector<std::unique_ptr<Type>> Storage;
      std::unordered_set<const Type *> OwnedTypes;
      std::unordered_map<const Type *, const PointerType *> PointerTypes;
      std::unordered_map<const Type *, const ReferenceType *> ReferenceTypes;
      std::unordered_map<const Type *, std::unordered_map<std::uint64_t, const ArrayType *>> ArrayTypes;
      std::unordered_map<FunctionKey, const FunctionType *, FunctionKeyHash> FunctionTypes;
  };
} // namespace ink::ir

#endif
