#include "ink/semantic/model/type/type_pool.h"

#include "ink/semantic/context.h"

#include "../hash.h"

#include <algorithm>
#include <functional>
#include <utility>

namespace ink::semantic
{
  namespace
  {
    bool isValidAccess(AccessKind Access) noexcept
    {
      return Access == AccessKind::ReadOnly || Access == AccessKind::ReadWrite;
    }

    std::size_t functionTypeHash(const Type &ReturnType, std::span<const Type *const> ParameterTypes) noexcept
    {
      std::size_t Hash = combineHash(std::hash<const Type *>{}(&ReturnType), ParameterTypes.size());
      for (const Type *ParameterType : ParameterTypes)
      {
        Hash = combineHash(Hash, std::hash<const Type *>{}(ParameterType));
      }
      return Hash;
    }
  } // namespace

  std::size_t TypePool::ArrayTypeKeyHash::operator()(const ArrayTypeKey &Key) const noexcept
  {
    return combineHash(std::hash<const Type *>{}(Key.ElementType), std::hash<std::uint64_t>{}(Key.ElementCount));
  }

  std::size_t TypePool::AccessTypeKeyHash::operator()(const AccessTypeKey &Key) const noexcept
  {
    return combineHash(std::hash<const Type *>{}(Key.TargetType), static_cast<std::uint8_t>(Key.Access));
  }

  TypePool::TypePool(SemanticContext &Context)
      : Context(Context)
  {
    MetaType.reset(new BuiltinType(Context, ValueKind::BuiltinType, TypeKind::Meta, nullptr));
    VoidType.reset(new BuiltinType(Context, ValueKind::BuiltinType, TypeKind::Void, MetaType.get()));
    BoolType.reset(new BuiltinType(Context, ValueKind::BuiltinType, TypeKind::Bool, MetaType.get()));
    LabelType.reset(new BuiltinType(Context, ValueKind::BuiltinType, TypeKind::Label, MetaType.get()));
    ModuleType.reset(new BuiltinType(Context, ValueKind::BuiltinType, TypeKind::Module, MetaType.get()));
  }

  TypePool::~TypePool() = default;

  template <TypeKind Kind> requires SingletonTypeKind<Kind>
  const BuiltinType &TypePool::getType() const noexcept
  {
    if constexpr (Kind == TypeKind::Meta)
    {
      return *MetaType;
    }
    else if constexpr (Kind == TypeKind::Void)
    {
      return *VoidType;
    }
    else if constexpr (Kind == TypeKind::Bool)
    {
      return *BoolType;
    }
    else if constexpr (Kind == TypeKind::Label)
    {
      return *LabelType;
    }
    else
    {
      return *ModuleType;
    }
  }

  template <typename TypeMap, typename... Arguments>
  const typename TypeMap::mapped_type::element_type *TypePool::internType(TypeMap &Types, const typename TypeMap::key_type &Key, Arguments &&...Args)
  {
    using InternedType = typename TypeMap::mapped_type::element_type;
    auto [Entry, Inserted] = Types.try_emplace(Key);
    if (Inserted)
    {
      Entry->second.reset(new InternedType(Context, getType<TypeKind::Meta>(), std::forward<Arguments>(Args)...));
    }
    return Entry->second.get();
  }

  template <TypeKind Kind> requires (Kind == TypeKind::Integer)
  const IntegerType *TypePool::getType(std::uint32_t BitWidth, bool Signed)
  {
    if (BitWidth == 0)
    {
      return nullptr;
    }
    const std::uint64_t Key = (static_cast<std::uint64_t>(BitWidth) << 1) | static_cast<std::uint64_t>(Signed);
    return internType(IntegerTypes, Key, BitWidth, Signed);
  }

  template <TypeKind Kind> requires (Kind == TypeKind::Float)
  const FloatType *TypePool::getType(std::uint32_t BitWidth)
  {
    if (BitWidth != 16 && BitWidth != 32 && BitWidth != 64)
    {
      return nullptr;
    }
    return internType(FloatTypes, BitWidth, BitWidth);
  }

  template <TypeKind Kind> requires (Kind == TypeKind::Array)
  const ArrayType *TypePool::getType(const Type &ElementType, std::uint64_t ElementCount)
  {
    if (&ElementType.context() != &Context)
    {
      return nullptr;
    }
    return internType(ArrayTypes, ArrayTypeKey{&ElementType, ElementCount}, ElementType, ElementCount);
  }

  template <TypeKind Kind> requires AccessTypeKind<Kind>
  const AccessTypeForKind<Kind> *TypePool::getType(const Type &TargetType, AccessKind Access)
  {
    if (&TargetType.context() != &Context || !isValidAccess(Access))
    {
      return nullptr;
    }
    auto &Types = [this]() -> auto &
    {
      if constexpr (Kind == TypeKind::Slice)
      {
        return SliceTypes;
      }
      else if constexpr (Kind == TypeKind::Pointer)
      {
        return PointerTypes;
      }
      else
      {
        return ReferenceTypes;
      }
    }();
    return internType(Types, AccessTypeKey{&TargetType, Access}, TargetType, Access);
  }

  template <TypeKind Kind> requires (Kind == TypeKind::Function)
  const FunctionType *TypePool::getType(const Type &ReturnType, std::span<const Type *const> ParameterTypes)
  {
    if (&ReturnType.context() != &Context)
    {
      return nullptr;
    }
    for (const Type *ParameterType : ParameterTypes)
    {
      if (!ParameterType || &ParameterType->context() != &Context)
      {
        return nullptr;
      }
    }
    const std::size_t Hash = functionTypeHash(ReturnType, ParameterTypes);
    const auto Candidates = FunctionTypes.equal_range(Hash);
    for (auto Entry = Candidates.first; Entry != Candidates.second; ++Entry)
    {
      const FunctionType &Candidate = *Entry->second;
      if (&Candidate.returnType() == &ReturnType && std::equal(Candidate.parameterTypes().begin(), Candidate.parameterTypes().end(), ParameterTypes.begin(), ParameterTypes.end()))
      {
        return &Candidate;
      }
    }
    auto Result = std::unique_ptr<FunctionType>(new FunctionType(Context, getType<TypeKind::Meta>(), ReturnType, ParameterTypes));
    const FunctionType *Pointer = Result.get();
    FunctionTypes.emplace(Hash, std::move(Result));
    return Pointer;
  }

  const ClassType *TypePool::createClassType(Name TypeName)
  {
    if (!Context.namePool().contains(TypeName))
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<ClassType>(new ClassType(Context, getType<TypeKind::Meta>(), TypeName));
    const ClassType *Pointer = Result.get();
    UserDefinedTypes.push_back(std::move(Result));
    return Pointer;
  }

  const EnumType *TypePool::createEnumType(Name TypeName)
  {
    if (!Context.namePool().contains(TypeName))
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<EnumType>(new EnumType(Context, getType<TypeKind::Meta>(), TypeName));
    const EnumType *Pointer = Result.get();
    UserDefinedTypes.push_back(std::move(Result));
    return Pointer;
  }

  const InterfaceType *TypePool::createInterfaceType(Name TypeName)
  {
    if (!Context.namePool().contains(TypeName))
    {
      return nullptr;
    }
    auto Result = std::unique_ptr<InterfaceType>(new InterfaceType(Context, getType<TypeKind::Meta>(), TypeName));
    const InterfaceType *Pointer = Result.get();
    UserDefinedTypes.push_back(std::move(Result));
    return Pointer;
  }

  // The constrained API is closed over these kinds; keep implementation and cache details in this file.
  template const BuiltinType &TypePool::getType<TypeKind::Meta>() const noexcept;
  template const BuiltinType &TypePool::getType<TypeKind::Void>() const noexcept;
  template const BuiltinType &TypePool::getType<TypeKind::Bool>() const noexcept;
  template const BuiltinType &TypePool::getType<TypeKind::Label>() const noexcept;
  template const BuiltinType &TypePool::getType<TypeKind::Module>() const noexcept;
  template const IntegerType *TypePool::getType<TypeKind::Integer>(std::uint32_t, bool);
  template const FloatType *TypePool::getType<TypeKind::Float>(std::uint32_t);
  template const ArrayType *TypePool::getType<TypeKind::Array>(const Type &, std::uint64_t);
  template const SliceType *TypePool::getType<TypeKind::Slice>(const Type &, AccessKind);
  template const PointerType *TypePool::getType<TypeKind::Pointer>(const Type &, AccessKind);
  template const ReferenceType *TypePool::getType<TypeKind::Reference>(const Type &, AccessKind);
  template const FunctionType *TypePool::getType<TypeKind::Function>(const Type &, std::span<const Type *const>);
} // namespace ink::semantic
