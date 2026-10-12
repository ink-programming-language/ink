#ifndef INK_IR_TYPE_TYPE_CONTEXT_H
#define INK_IR_TYPE_TYPE_CONTEXT_H

#include "ink/core/name.h"
#include "ink/ir/constant/constant_pool.h"
#include "ink/target/target_context.h"
#include "ink/ir/type/structure_type/array_type.h"
#include "ink/ir/type/builtin_type/bool_type.h"
#include "ink/ir/type/nominal_type/class_type.h"
#include "ink/ir/type/builtin_type/float_type.h"
#include "ink/ir/type/structure_type/function_type.h"
#include "ink/ir/type/builtin_type/integer_type.h"
#include "ink/ir/type/nominal_type/interface_type.h"
#include "ink/ir/type/builtin_type/null_pointer_type.h"
#include "ink/ir/type/structure_type/pointer_type.h"
#include "ink/ir/type/structure_type/reference_type.h"
#include "ink/ir/type/builtin_type/string_type.h"
#include "ink/ir/type/nominal_type/struct_type.h"
#include "ink/ir/type/builtin_type/variadic_type.h"
#include "ink/ir/type/builtin_type/void_type.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ink::ir
{
  // Owns types with fixed layouts and their unique ConstantPool for one shared lifetime.
  // Nominal metadata can be built and then frozen.
  // Returned pointers are borrowed and remain valid until this context is destroyed; external IR users must die first.
  // Teardown releases nominal methods, field metadata, constants and finally types, in that order.
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
      ConstantPool &constantPool() noexcept;
      const ConstantPool &constantPool() const noexcept;
      bool owns(const Type *TypeValue) const noexcept;

      const VoidType *getVoidType() const noexcept;
      const VariadicType *getVariadicType() const noexcept;
      const BoolType *getBoolType() const noexcept;
      const NullPointerType *getNullPointerType() const noexcept;
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

      // Each creation has a distinct nominal identity, even for equal layouts.
      // Size is preserved in bytes; zero or non-power-of-two alignment returns nullptr.
      // Type names, fields and owned methods remain editable until freeze(); function names/bodies are not frozen and layouts never change.
      ClassType *createClassType(std::uint64_t Size, std::uint64_t Alignment, core::Name Name = core::NAME_None);
      InterfaceType *createInterfaceType(std::uint64_t Size, std::uint64_t Alignment, core::Name Name = core::NAME_None);
      StructType *createStructType(std::uint64_t Size, std::uint64_t Alignment, core::Name Name = core::NAME_None);

      // Structural types are reused. Null or foreign-context operands return nullptr.
      const PointerType *getPointerType(const Type *PointeeType);
      const ReferenceType *getReferenceType(const Type *ReferencedType);
      // Missing/invalid element layout or size overflow also returns nullptr.
      const ArrayType *getArrayType(const Type *ElementType, std::uint64_t ElementCount);
      // All types must have runtime layouts. Cache identity uses the return type
      // and the ordered parameter types.
      const FunctionType *getFunctionType(const Type *ReturnType, std::vector<const Type *> Parameters = {});

    private:
      struct FunctionKey
      {
          const Type *ReturnType;
          std::vector<const Type *> Parameters;

          bool operator==(const FunctionKey &Other) const noexcept;
      };

      struct FunctionKeyHash
      {
          std::size_t operator()(const FunctionKey &Key) const noexcept;
      };

      template <typename TypeT>
      TypeT *own(std::unique_ptr<TypeT> TypeValue);

      bool validParameters(const std::vector<const Type *> &Parameters) const noexcept;

      const target::TargetContext Target;
      VoidType Void;
      VariadicType Variadic;
      BoolType Bool;
      NullPointerType NullPointer;
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
      // Declared last so constants are destroyed before any of their referenced types.
      ConstantPool Constants;
  };
} // namespace ink::ir

#endif
