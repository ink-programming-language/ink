#ifndef INK_SEMANTIC_MODEL_TYPE_TYPE_POOL_H
#define INK_SEMANTIC_MODEL_TYPE_TYPE_POOL_H

#include "ink/semantic/model/function/function_type.h"
#include "ink/semantic/model/type/array_type.h"
#include "ink/semantic/model/type/builtin_type.h"
#include "ink/semantic/model/type/class_type.h"
#include "ink/semantic/model/type/enum_type.h"
#include "ink/semantic/model/type/float_type.h"
#include "ink/semantic/model/type/integer_type.h"
#include "ink/semantic/model/type/interface_type.h"
#include "ink/semantic/model/type/pointer_type.h"
#include "ink/semantic/model/type/reference_type.h"
#include "ink/semantic/model/type/slice_type.h"
#include "ink/semantic/model/type/user_defined_type.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace ink::semantic
{
  class SemanticContext;

  template <TypeKind Kind>
  concept SingletonTypeKind = Kind == TypeKind::Meta || Kind == TypeKind::Void || Kind == TypeKind::Bool || Kind == TypeKind::Label || Kind == TypeKind::Module;

  template <TypeKind Kind>
  concept AccessTypeKind = Kind == TypeKind::Slice || Kind == TypeKind::Pointer || Kind == TypeKind::Reference;

  template <TypeKind Kind> requires AccessTypeKind<Kind>
  using AccessTypeForKind = std::conditional_t<Kind == TypeKind::Slice, SliceType, std::conditional_t<Kind == TypeKind::Pointer, PointerType, ReferenceType>>;

  // Each SemanticContext owns one pool of immutable types with stable addresses.
  // Structural types are canonical; each nominal creation has its own identity.
  class TypePool final
  {
    public:
      ~TypePool();
      TypePool(const TypePool &) = delete;
      TypePool &operator=(const TypePool &) = delete;
      TypePool(TypePool &&) = delete;
      TypePool &operator=(TypePool &&) = delete;

      const SemanticContext &context() const noexcept
      {
        return Context;
      }

      // Label and Module are internal types for basic block and module identities.
      template <TypeKind Kind> requires SingletonTypeKind<Kind>
      const BuiltinType &getType() const noexcept;

      // Zero width is invalid. Signedness participates in canonical identity.
      template <TypeKind Kind> requires (Kind == TypeKind::Integer)
      const IntegerType *getType(std::uint32_t BitWidth, bool Signed);

      // IEEE binary16/32/64. Other widths return null.
      template <TypeKind Kind> requires (Kind == TypeKind::Float)
      const FloatType *getType(std::uint32_t BitWidth);

      // Components must belong to this context; invalid access modes return null.
      // Source-language legality and target layout are checked by later analysis.
      template <TypeKind Kind> requires (Kind == TypeKind::Array)
      const ArrayType *getType(const Type &ElementType, std::uint64_t ElementCount);

      template <TypeKind Kind> requires AccessTypeKind<Kind>
      const AccessTypeForKind<Kind> *getType(const Type &TargetType, AccessKind Access);

      // Fixed-arity signatures are canonical by return type and ordered parameters.
      // All components must be local; null parameter types are rejected.
      template <TypeKind Kind> requires (Kind == TypeKind::Function)
      const FunctionType *getType(const Type &ReturnType, std::span<const Type *const> ParameterTypes = {});

      // Requires a valid name from this context's NamePool. Same-name types stay distinct.
      // Callers reuse the returned object for the same type or instantiated type.
      const ClassType *createClassType(Name TypeName);
      const EnumType *createEnumType(Name TypeName);
      const InterfaceType *createInterfaceType(Name TypeName);

    private:
      explicit TypePool(SemanticContext &Context);

      template <typename TypeMap, typename... Arguments>
      const typename TypeMap::mapped_type::element_type *internType(TypeMap &Types, const typename TypeMap::key_type &Key, Arguments &&...Args);

      struct ArrayTypeKey
      {
          const Type *ElementType;
          std::uint64_t ElementCount;

          bool operator==(const ArrayTypeKey &) const noexcept = default;
      };

      struct ArrayTypeKeyHash
      {
          std::size_t operator()(const ArrayTypeKey &Key) const noexcept;
      };

      struct AccessTypeKey
      {
          const Type *TargetType;
          AccessKind Access;

          bool operator==(const AccessTypeKey &) const noexcept = default;
      };

      struct AccessTypeKeyHash
      {
          std::size_t operator()(const AccessTypeKey &Key) const noexcept;
      };

      SemanticContext &Context;
      std::unique_ptr<BuiltinType> MetaType;
      std::unique_ptr<BuiltinType> VoidType;
      std::unique_ptr<BuiltinType> BoolType;
      std::unique_ptr<BuiltinType> LabelType;
      std::unique_ptr<BuiltinType> ModuleType;
      // Type destructors never traverse borrowed component types.
      std::unordered_map<std::uint64_t, std::unique_ptr<IntegerType>> IntegerTypes;
      std::unordered_map<std::uint32_t, std::unique_ptr<FloatType>> FloatTypes;
      std::vector<std::unique_ptr<UserDefinedType>> UserDefinedTypes;
      std::unordered_map<ArrayTypeKey, std::unique_ptr<ArrayType>, ArrayTypeKeyHash> ArrayTypes;
      std::unordered_map<AccessTypeKey, std::unique_ptr<SliceType>, AccessTypeKeyHash> SliceTypes;
      std::unordered_map<AccessTypeKey, std::unique_ptr<PointerType>, AccessTypeKeyHash> PointerTypes;
      std::unordered_map<AccessTypeKey, std::unique_ptr<ReferenceType>, AccessTypeKeyHash> ReferenceTypes;
      std::unordered_multimap<std::size_t, std::unique_ptr<FunctionType>> FunctionTypes;

      friend class SemanticContext;
  };
} // namespace ink::semantic

#endif
