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

#include <concepts>
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

      // Compile-time-only markers are immutable, eagerly owned singletons.
      template <typename TypeT> requires (std::same_as<TypeT, MetaType> || std::same_as<TypeT, VariadicType>)
      const TypeT *get() const noexcept
      {
        if constexpr (std::same_as<TypeT, MetaType>)
        {
          return &Meta;
        }
        else
        {
          return &Variadic;
        }
      }

      // Fixed nonnumeric builtins have eagerly owned mutable and const variants.
      template <typename TypeT> requires (std::same_as<TypeT, VoidType> || std::same_as<TypeT, BoolType> || std::same_as<TypeT, NullPointerType>)
      const TypeT *get(bool Mutable = true) const noexcept
      {
        if constexpr (std::same_as<TypeT, VoidType>)
        {
          return Mutable ? &Void : &ConstVoid;
        }
        else if constexpr (std::same_as<TypeT, BoolType>)
        {
          return Mutable ? &Bool : &ConstBool;
        }
        else
        {
          return Mutable ? &NullPointer : &ConstNullPointer;
        }
      }

      // Width and signedness are runtime attributes; every valid IntegerBits width is interned on demand.
      template <typename TypeT> requires std::same_as<TypeT, IntegerType>
      const TypeT *get(std::uint32_t Width, bool Signed, bool Mutable = true)
      {
        return internInteger(Width, Signed, Mutable);
      }

      // Floating-point support remains restricted to 32 and 64 bits.
      template <typename TypeT> requires std::same_as<TypeT, FloatType>
      const TypeT *get(std::uint32_t Width, bool Mutable = true)
      {
        return internFloat(Width, Mutable);
      }

      // Changes only the top-level qualifier and preserves canonical identity.
      // Null, foreign-context types and readonly compile-time-only markers return nullptr.
      const Type *get(const Type *TypeValue, bool Mutable);

      // Structural types are reused. Null or foreign-context operands return nullptr.
      // Pointer and reference operands cannot be the compile-time-only metatype.
      template <typename TypeT> requires (std::same_as<TypeT, PointerType> || std::same_as<TypeT, ReferenceType>)
      const TypeT *get(const Type *ElementType, bool Mutable = true)
      {
        if constexpr (std::same_as<TypeT, PointerType>)
        {
          return internPointer(ElementType, Mutable);
        }
        else
        {
          return internReference(ElementType, Mutable);
        }
      }

      // Missing/invalid element layout or size overflow also returns nullptr.
      template <typename TypeT> requires std::same_as<TypeT, ArrayType>
      const TypeT *get(const Type *ElementType, std::uint64_t ElementCount, bool Mutable = true)
      {
        return internArray(ElementType, ElementCount, Mutable);
      }

      // All types must have runtime layouts. Signature descriptors are copied by value.
      // Cache identity uses the return type and ordered parameter types and variadic flags.
      template <typename TypeT> requires std::same_as<TypeT, FunctionType>
      const TypeT *get(const Type *ReturnType, std::initializer_list<FunctionTypeParameter> Parameters = {}, bool Mutable = true)
      {
        return internFunction(ReturnType, std::span<const FunctionTypeParameter>(Parameters.begin(), Parameters.size()), Mutable);
      }

      template <typename TypeT> requires std::same_as<TypeT, FunctionType>
      const TypeT *get(const Type *ReturnType, std::span<const FunctionTypeParameter> Parameters, bool Mutable = true)
      {
        return internFunction(ReturnType, Parameters, Mutable);
      }

    private:
      friend class IRContext;

      explicit TypePool(IRContext &Owner);
      ~TypePool() noexcept;

      const IntegerType *internInteger(std::uint32_t Width, bool Signed, bool Mutable);
      const FloatType *internFloat(std::uint32_t Width, bool Mutable);
      const PointerType *internPointer(const Type *PointeeType, bool Mutable);
      const ReferenceType *internReference(const Type *ReferencedType, bool Mutable);
      const ArrayType *internArray(const Type *ElementType, std::uint64_t ElementCount, bool Mutable);
      const FunctionType *internFunction(const Type *ReturnType, std::span<const FunctionTypeParameter> Parameters, bool Mutable);

      struct IntegerKey
      {
          std::uint32_t Width;
          bool Signed;
          bool Mutable;

          bool operator==(const IntegerKey &Other) const noexcept;
      };

      struct IntegerKeyHash
      {
          std::size_t operator()(const IntegerKey &Key) const noexcept;
      };

      struct FloatKey
      {
          std::uint32_t Width;
          bool Mutable;

          bool operator==(const FloatKey &Other) const noexcept;
      };

      struct FloatKeyHash
      {
          std::size_t operator()(const FloatKey &Key) const noexcept;
      };

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
      std::vector<std::unique_ptr<Type>> Storage;
      std::unordered_set<const Type *> OwnedTypes;
      std::unordered_map<IntegerKey, const IntegerType *, IntegerKeyHash> IntegerTypes;
      std::unordered_map<FloatKey, const FloatType *, FloatKeyHash> FloatTypes;
      std::unordered_map<PointerKey, const PointerType *, PointerKeyHash> PointerTypes;
      std::unordered_map<ReferenceKey, const ReferenceType *, ReferenceKeyHash> ReferenceTypes;
      std::unordered_map<ArrayKey, const ArrayType *, ArrayKeyHash> ArrayTypes;
      std::unordered_map<FunctionKey, const FunctionType *, FunctionKeyHash> FunctionTypes;
      std::unordered_map<const Type *, const Type *> ConstNominalTypes;
  };
} // namespace ink::ir

#endif
