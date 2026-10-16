#ifndef INK_IR_TYPE_TYPE_POOL_H
#define INK_IR_TYPE_TYPE_POOL_H

#include "ink/ir/type/builtin_type/bool_type.h"
#include "ink/ir/type/builtin_type/float_type.h"
#include "ink/ir/type/builtin_type/integer_type.h"
#include "ink/ir/type/builtin_type/null_pointer_type.h"
#include "ink/ir/type/builtin_type/variadic_type.h"
#include "ink/ir/type/builtin_type/void_type.h"
#include "ink/ir/type/structure_type/array_type.h"
#include "ink/ir/type/structure_type/function_type.h"
#include "ink/ir/type/structure_type/pointer_type.h"
#include "ink/ir/type/structure_type/reference_type.h"

#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <memory>
#include <span>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace ink::ir
{
  class IRContext;

  // Owns and canonicalizes builtin and structural types for its containing IRContext.
  // Nominal operands are borrowed from modules using that context and must outlive all uses of derived types.
  // Operations require external synchronization.
  class TypePool final
  {
    public:
      TypePool(const TypePool &) = delete;
      TypePool &operator=(const TypePool &) = delete;
      TypePool(TypePool &&) = delete;
      TypePool &operator=(TypePool &&) = delete;

      const IRContext &irContext() const noexcept;
      // Includes only pooled builtin and structural types, excluding module-owned nominal types.
      bool owns(const Type *TypeValue) const noexcept;

      const VoidType *getVoidType() const noexcept;
      const VariadicType *getVariadicType() const noexcept;
      const BoolType *getBoolType() const noexcept;
      const NullPointerType *getNullPointerType() const noexcept;
      const IntegerType *getInt8Type() const noexcept;
      const IntegerType *getUInt8Type() const noexcept;
      const IntegerType *getInt16Type() const noexcept;
      const IntegerType *getUInt16Type() const noexcept;
      const IntegerType *getInt32Type() const noexcept;
      const IntegerType *getUInt32Type() const noexcept;
      const IntegerType *getInt64Type() const noexcept;
      const IntegerType *getUInt64Type() const noexcept;
      const FloatType *getFloat32Type() const noexcept;
      const FloatType *getFloat64Type() const noexcept;

      // Width is in bits; integers support signed/unsigned 8/16/32/64-bit types.
      // Unsupported widths return nullptr.
      const IntegerType *getIntegerType(std::uint32_t Width, bool Signed) const noexcept;
      const FloatType *getFloatType(std::uint32_t Width) const noexcept;

      // Structural types are reused. Null or foreign-context operands return nullptr.
      const PointerType *getPointerType(const Type *PointeeType);
      const ReferenceType *getReferenceType(const Type *ReferencedType);
      // Missing/invalid element layout or size overflow also returns nullptr.
      const ArrayType *getArrayType(const Type *ElementType, std::uint64_t ElementCount);
      // All types must have runtime layouts. Signature descriptors are copied by value.
      // Cache identity uses the return type and ordered parameter types and variadic flags.
      const FunctionType *getFunctionType(const Type *ReturnType, std::initializer_list<FunctionTypeParameter> Parameters = {});
      const FunctionType *getFunctionType(const Type *ReturnType, std::span<const FunctionTypeParameter> Parameters);

    private:
      friend class IRContext;

      explicit TypePool(IRContext &Owner);
      ~TypePool() noexcept;

      struct FunctionKey
      {
          const Type *ReturnType;
          std::vector<std::pair<const Type *, bool>> Parameters;

          bool operator==(const FunctionKey &Other) const noexcept;
      };

      struct FunctionKeyHash
      {
          std::size_t operator()(const FunctionKey &Key) const noexcept;
      };

      template <typename TypeT>
      TypeT *own(std::unique_ptr<TypeT> TypeValue);

      bool validParameters(std::span<const FunctionTypeParameter> Parameters) const noexcept;

      const IRContext &Owner;
      VoidType Void;
      VariadicType Variadic;
      BoolType Bool;
      NullPointerType NullPointer;
      IntegerType Int8;
      IntegerType UInt8;
      IntegerType Int16;
      IntegerType UInt16;
      IntegerType Int32;
      IntegerType UInt32;
      IntegerType Int64;
      IntegerType UInt64;
      FloatType Float32;
      FloatType Float64;
      std::vector<std::unique_ptr<Type>> Storage;
      std::unordered_set<const Type *> OwnedTypes;
      std::unordered_map<const Type *, const PointerType *> PointerTypes;
      std::unordered_map<const Type *, const ReferenceType *> ReferenceTypes;
      std::unordered_map<const Type *, std::unordered_map<std::uint64_t, const ArrayType *>> ArrayTypes;
      std::unordered_map<FunctionKey, const FunctionType *, FunctionKeyHash> FunctionTypes;
  };
} // namespace ink::ir

#endif
