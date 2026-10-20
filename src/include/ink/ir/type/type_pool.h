#ifndef INK_IR_TYPE_TYPE_POOL_H
#define INK_IR_TYPE_TYPE_POOL_H

#include "ink/ir/type/builtin_type/bool_type.h"
#include "ink/ir/type/builtin_type/float_type.h"
#include "ink/ir/type/builtin_type/integer_type.h"
#include "ink/ir/type/builtin_type/meta_type.h"
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
      // Includes builtin and structural types and borrowed nominal qualifier views, excluding module-owned declarations.
      bool owns(const Type *TypeValue) const noexcept;

      const MetaType *getMetaType() const noexcept;
      const VoidType *getVoidType(bool Mutable = true) const noexcept;
      const VariadicType *getVariadicType() const noexcept;
      const BoolType *getBoolType(bool Mutable = true) const noexcept;
      const NullPointerType *getNullPointerType(bool Mutable = true) const noexcept;
      const IntegerType *getInt8Type(bool Mutable = true) const noexcept;
      const IntegerType *getUInt8Type(bool Mutable = true) const noexcept;
      const IntegerType *getInt16Type(bool Mutable = true) const noexcept;
      const IntegerType *getUInt16Type(bool Mutable = true) const noexcept;
      const IntegerType *getInt32Type(bool Mutable = true) const noexcept;
      const IntegerType *getUInt32Type(bool Mutable = true) const noexcept;
      const IntegerType *getInt64Type(bool Mutable = true) const noexcept;
      const IntegerType *getUInt64Type(bool Mutable = true) const noexcept;
      const FloatType *getFloat32Type(bool Mutable = true) const noexcept;
      const FloatType *getFloat64Type(bool Mutable = true) const noexcept;

      // Width is in bits; integers support signed/unsigned 8/16/32/64-bit types.
      // Unsupported widths return nullptr.
      const IntegerType *getIntegerType(std::uint32_t Width, bool Signed, bool Mutable = true) const noexcept;
      const FloatType *getFloatType(std::uint32_t Width, bool Mutable = true) const noexcept;

      // Changes only the top-level qualifier and preserves canonical identity.
      // Null, foreign-context types and readonly compile-time-only markers return nullptr.
      const Type *getQualifiedType(const Type *TypeValue, bool Mutable);

      // Structural types are reused. Null or foreign-context operands return nullptr.
      // Pointer and reference operands cannot be the compile-time-only metatype.
      const PointerType *getPointerType(const Type *PointeeType, bool Mutable = true);
      const ReferenceType *getReferenceType(const Type *ReferencedType, bool Mutable = true);
      // Missing/invalid element layout or size overflow also returns nullptr.
      const ArrayType *getArrayType(const Type *ElementType, std::uint64_t ElementCount, bool Mutable = true);
      // All types must have runtime layouts. Signature descriptors are copied by value.
      // Cache identity uses the return type and ordered parameter types and variadic flags.
      const FunctionType *getFunctionType(const Type *ReturnType, std::initializer_list<FunctionTypeParameter> Parameters = {}, bool Mutable = true);
      const FunctionType *getFunctionType(const Type *ReturnType, std::span<const FunctionTypeParameter> Parameters, bool Mutable = true);

    private:
      friend class IRContext;

      explicit TypePool(IRContext &Owner);
      ~TypePool() noexcept;

      struct PointerKey
      {
          const Type *PointeeType;
          bool Mutable;

          bool operator==(const PointerKey &Other) const noexcept;
      };

      struct PointerKeyHash
      {
          std::size_t operator()(const PointerKey &Key) const noexcept;
      };

      struct ReferenceKey
      {
          const Type *ReferencedType;
          bool Mutable;

          bool operator==(const ReferenceKey &Other) const noexcept;
      };

      struct ReferenceKeyHash
      {
          std::size_t operator()(const ReferenceKey &Key) const noexcept;
      };

      struct ArrayKey
      {
          const Type *ElementType;
          std::uint64_t ElementCount;
          bool Mutable;

          bool operator==(const ArrayKey &Other) const noexcept;
      };

      struct ArrayKeyHash
      {
          std::size_t operator()(const ArrayKey &Key) const noexcept;
      };

      struct FunctionKey
      {
          const Type *ReturnType;
          std::vector<std::pair<const Type *, bool>> Parameters;
          bool Mutable;

          bool operator==(const FunctionKey &Other) const noexcept;
      };

      struct FunctionKeyHash
      {
          std::size_t operator()(const FunctionKey &Key) const noexcept;
      };

      template <typename TypeT>
      TypeT *own(std::unique_ptr<TypeT> TypeValue, const Type *UnqualifiedType = nullptr);

      bool validParameters(std::span<const FunctionTypeParameter> Parameters) const noexcept;
      void unregisterNominalType(const Type *Declaration) noexcept;

      const IRContext &Owner;
      MetaType Meta;
      VoidType Void;
      VoidType ConstVoid;
      VariadicType Variadic;
      BoolType Bool;
      BoolType ConstBool;
      NullPointerType NullPointer;
      NullPointerType ConstNullPointer;
      IntegerType Int8;
      IntegerType ConstInt8;
      IntegerType UInt8;
      IntegerType ConstUInt8;
      IntegerType Int16;
      IntegerType ConstInt16;
      IntegerType UInt16;
      IntegerType ConstUInt16;
      IntegerType Int32;
      IntegerType ConstInt32;
      IntegerType UInt32;
      IntegerType ConstUInt32;
      IntegerType Int64;
      IntegerType ConstInt64;
      IntegerType UInt64;
      IntegerType ConstUInt64;
      FloatType Float32;
      FloatType ConstFloat32;
      FloatType Float64;
      FloatType ConstFloat64;
      std::vector<std::unique_ptr<Type>> Storage;
      std::unordered_set<const Type *> OwnedTypes;
      std::unordered_map<PointerKey, const PointerType *, PointerKeyHash> PointerTypes;
      std::unordered_map<ReferenceKey, const ReferenceType *, ReferenceKeyHash> ReferenceTypes;
      std::unordered_map<ArrayKey, const ArrayType *, ArrayKeyHash> ArrayTypes;
      std::unordered_map<FunctionKey, const FunctionType *, FunctionKeyHash> FunctionTypes;
      std::unordered_map<const Type *, const Type *> ConstNominalTypes;
  };
} // namespace ink::ir

#endif
