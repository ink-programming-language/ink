#include "ink/ir/type/type_pool.h"
#include "ink/ir/context.h"
#include "ink/ir/type/nominal_type/class_type.h"
#include "ink/ir/type/nominal_type/interface_type.h"
#include "ink/ir/type/nominal_type/struct_type.h"
#include "type_layout.h"

#include <functional>
#include <limits>
#include <utility>

namespace ink::ir
{
  TypePool::TypePool(IRContext &Owner)
      : Owner(Owner),
        Meta(),
        Void(Meta),
        ConstVoid(Meta, false),
        Variadic(Meta),
        Bool(Meta),
        ConstBool(Meta, false),
        NullPointer(Meta, Owner),
        ConstNullPointer(Meta, Owner, false)
  {
    OwnedTypes.insert({&Meta, &Void, &ConstVoid, &Variadic, &Bool, &ConstBool, &NullPointer, &ConstNullPointer});
    ConstVoid.UnqualifiedType = &Void;
    ConstBool.UnqualifiedType = &Bool;
    ConstNullPointer.UnqualifiedType = &NullPointer;
  }

  TypePool::~TypePool() noexcept = default;

  const IRContext &TypePool::irContext() const noexcept
  {
    return Owner;
  }

  bool TypePool::owns(const Type *TypeValue) const noexcept
  {
    return OwnedTypes.find(TypeValue) != OwnedTypes.end();
  }

  bool TypePool::IntegerKey::operator==(const IntegerKey &Other) const noexcept
  {
    return Width == Other.Width && Signed == Other.Signed && Mutable == Other.Mutable;
  }

  std::size_t TypePool::IntegerKeyHash::operator()(const IntegerKey &Key) const noexcept
  {
    std::size_t Hash = std::hash<std::uint32_t>{}(Key.Width);
    Hash ^= std::hash<bool>{}(Key.Signed) + 0x9e3779b9U + (Hash << 6U) + (Hash >> 2U);
    Hash ^= std::hash<bool>{}(Key.Mutable) + 0x9e3779b9U + (Hash << 6U) + (Hash >> 2U);
    return Hash;
  }

  bool TypePool::FloatKey::operator==(const FloatKey &Other) const noexcept
  {
    return Width == Other.Width && Mutable == Other.Mutable;
  }

  std::size_t TypePool::FloatKeyHash::operator()(const FloatKey &Key) const noexcept
  {
    std::size_t Hash = std::hash<std::uint32_t>{}(Key.Width);
    Hash ^= std::hash<bool>{}(Key.Mutable) + 0x9e3779b9U + (Hash << 6U) + (Hash >> 2U);
    return Hash;
  }

  bool TypePool::PointerKey::operator==(const PointerKey &Other) const noexcept
  {
    return PointeeType == Other.PointeeType && Mutable == Other.Mutable;
  }

  std::size_t TypePool::PointerKeyHash::operator()(const PointerKey &Key) const noexcept
  {
    std::size_t Hash = std::hash<const Type *>{}(Key.PointeeType);
    Hash ^= std::hash<bool>{}(Key.Mutable) + 0x9e3779b9U + (Hash << 6U) + (Hash >> 2U);
    return Hash;
  }

  bool TypePool::ReferenceKey::operator==(const ReferenceKey &Other) const noexcept
  {
    return ReferencedType == Other.ReferencedType && Mutable == Other.Mutable;
  }

  std::size_t TypePool::ReferenceKeyHash::operator()(const ReferenceKey &Key) const noexcept
  {
    std::size_t Hash = std::hash<const Type *>{}(Key.ReferencedType);
    Hash ^= std::hash<bool>{}(Key.Mutable) + 0x9e3779b9U + (Hash << 6U) + (Hash >> 2U);
    return Hash;
  }

  bool TypePool::ArrayKey::operator==(const ArrayKey &Other) const noexcept
  {
    return ElementType == Other.ElementType && ElementCount == Other.ElementCount && Mutable == Other.Mutable;
  }

  std::size_t TypePool::ArrayKeyHash::operator()(const ArrayKey &Key) const noexcept
  {
    std::size_t Hash = std::hash<const Type *>{}(Key.ElementType);
    Hash ^= std::hash<std::uint64_t>{}(Key.ElementCount) + 0x9e3779b9U + (Hash << 6U) + (Hash >> 2U);
    Hash ^= std::hash<bool>{}(Key.Mutable) + 0x9e3779b9U + (Hash << 6U) + (Hash >> 2U);
    return Hash;
  }

  bool TypePool::FunctionKey::operator==(const FunctionKey &Other) const noexcept
  {
    return ReturnType == Other.ReturnType && Parameters == Other.Parameters && Mutable == Other.Mutable;
  }

  std::size_t TypePool::FunctionKeyHash::operator()(const FunctionKey &Key) const noexcept
  {
    std::size_t Hash = std::hash<const Type *>{}(Key.ReturnType);
    Hash ^= std::hash<bool>{}(Key.Mutable) + 0x9e3779b9U + (Hash << 6U) + (Hash >> 2U);
    for (const auto &[ParameterType, IsVariadic] : Key.Parameters)
    {
      Hash ^= std::hash<const Type *>{}(ParameterType) + 0x9e3779b9U + (Hash << 6U) + (Hash >> 2U);
      Hash ^= std::hash<bool>{}(IsVariadic) + 0x9e3779b9U + (Hash << 6U) + (Hash >> 2U);
    }
    return Hash;
  }

  template <typename TypeT>
  TypeT *TypePool::own(std::unique_ptr<TypeT> TypeValue, const Type *UnqualifiedType)
  {
    TypeT *Result = TypeValue.get();
    if (UnqualifiedType)
    {
      Result->UnqualifiedType = UnqualifiedType;
    }
    OwnedTypes.insert(Result);
    Storage.push_back(std::move(TypeValue));
    return Result;
  }

  bool TypePool::validParameters(std::span<const FunctionTypeParameter> Parameters) const noexcept
  {
    for (const FunctionTypeParameter &Descriptor : Parameters)
    {
      if (!Owner.containsType(Descriptor.ParameterType) || !Descriptor.ParameterType->hasLayout())
      {
        return false;
      }
    }
    return true;
  }

  void TypePool::unregisterNominalType(const Type *Declaration) noexcept
  {
    const auto Found = ConstNominalTypes.find(Declaration);
    if (Found != ConstNominalTypes.end())
    {
      OwnedTypes.erase(Found->second);
      ConstNominalTypes.erase(Found);
    }
  }

  const IntegerType *TypePool::internInteger(std::uint32_t Width, bool Signed, bool Mutable)
  {
    if (!core::IntegerBits::isValidWidth(Width))
    {
      return nullptr;
    }
    const IntegerKey Key{Width, Signed, Mutable};
    const auto Found = IntegerTypes.find(Key);
    if (Found != IntegerTypes.end())
    {
      return Found->second;
    }
    const IntegerType *Result = own(std::unique_ptr<IntegerType>(new IntegerType(Meta, Owner.targetContext(), Width, Signed, Mutable)), Mutable ? nullptr : internInteger(Width, Signed, true));
    IntegerTypes.emplace(Key, Result);
    return Result;
  }

  const FloatType *TypePool::internFloat(std::uint32_t Width, bool Mutable)
  {
    if (Width != 32 && Width != 64)
    {
      return nullptr;
    }
    const FloatKey Key{Width, Mutable};
    const auto Found = FloatTypes.find(Key);
    if (Found != FloatTypes.end())
    {
      return Found->second;
    }
    const FloatType *Result = own(std::unique_ptr<FloatType>(new FloatType(Meta, Owner.targetContext(), Width, Mutable)), Mutable ? nullptr : internFloat(Width, true));
    FloatTypes.emplace(Key, Result);
    return Result;
  }

  const Type *TypePool::get(const Type *TypeValue, bool Mutable)
  {
    if (!Owner.containsType(TypeValue))
    {
      return nullptr;
    }
    if (TypeValue->isMutable() == Mutable)
    {
      return TypeValue;
    }
    if (Mutable)
    {
      return TypeValue->unqualifiedType();
    }
    switch (TypeValue->kind())
    {
      case ValueKind::Void:
        return get<VoidType>(false);
      case ValueKind::Bool:
        return get<BoolType>(false);
      case ValueKind::NullPointer:
        return get<NullPointerType>(false);
      case ValueKind::Integer:
      {
        const auto *Integer = static_cast<const IntegerType *>(TypeValue);
        return internInteger(Integer->width(), Integer->isSigned(), false);
      }
      case ValueKind::Float:
        return internFloat(static_cast<const FloatType *>(TypeValue)->width(), false);
      case ValueKind::Pointer:
        return internPointer(static_cast<const PointerType *>(TypeValue)->pointeeType(), false);
      case ValueKind::Reference:
        return internReference(static_cast<const ReferenceType *>(TypeValue)->referencedType(), false);
      case ValueKind::Array:
      {
        const auto *Array = static_cast<const ArrayType *>(TypeValue);
        return internArray(Array->elementType(), Array->elementCount(), false);
      }
      case ValueKind::FunctionType:
      {
        const auto *Function = static_cast<const FunctionType *>(TypeValue);
        return internFunction(Function->returnType(), Function->parameters(), false);
      }
      case ValueKind::Class:
      case ValueKind::Struct:
      case ValueKind::Interface:
      {
        const auto *Declaration = static_cast<const NominalType *>(TypeValue)->declarationType();
        const auto Found = ConstNominalTypes.find(Declaration);
        if (Found != ConstNominalTypes.end())
        {
          return Found->second;
        }
        const Type *Result = nullptr;
        if (ClassType::classof(Declaration))
        {
          Result = own(std::unique_ptr<ClassType>(new ClassType(*static_cast<const ClassType *>(Declaration), false)), Declaration);
        }
        else if (StructType::classof(Declaration))
        {
          Result = own(std::unique_ptr<StructType>(new StructType(*static_cast<const StructType *>(Declaration), false)), Declaration);
        }
        else
        {
          Result = own(std::unique_ptr<InterfaceType>(new InterfaceType(*static_cast<const InterfaceType *>(Declaration), false)), Declaration);
        }
        ConstNominalTypes.emplace(Declaration, Result);
        return Result;
      }
      default:
        return nullptr;
    }
  }

  const PointerType *TypePool::internPointer(const Type *PointeeType, bool Mutable)
  {
    if (!Owner.containsType(PointeeType) || MetaType::classof(PointeeType))
    {
      return nullptr;
    }
    const PointerKey Key{PointeeType, Mutable};
    const auto Found = PointerTypes.find(Key);
    if (Found != PointerTypes.end())
    {
      return Found->second;
    }
    const PointerType *Result = own(std::unique_ptr<PointerType>(new PointerType(Meta, Owner.targetContext(), PointeeType, Mutable)), Mutable ? nullptr : internPointer(PointeeType, true));
    PointerTypes.emplace(Key, Result);
    return Result;
  }

  const ReferenceType *TypePool::internReference(const Type *ReferencedType, bool Mutable)
  {
    if (!Owner.containsType(ReferencedType) || MetaType::classof(ReferencedType))
    {
      return nullptr;
    }
    const ReferenceKey Key{ReferencedType, Mutable};
    const auto Found = ReferenceTypes.find(Key);
    if (Found != ReferenceTypes.end())
    {
      return Found->second;
    }
    const ReferenceType *Result = own(std::unique_ptr<ReferenceType>(new ReferenceType(Meta, Owner.targetContext(), ReferencedType, Mutable)), Mutable ? nullptr : internReference(ReferencedType, true));
    ReferenceTypes.emplace(Key, Result);
    return Result;
  }

  const ArrayType *TypePool::internArray(const Type *ElementType, std::uint64_t ElementCount, bool Mutable)
  {
    if (!Owner.containsType(ElementType) || !ElementType->hasLayout())
    {
      return nullptr;
    }
    const ArrayKey Key{ElementType, ElementCount, Mutable};
    const auto Found = ArrayTypes.find(Key);
    if (Found != ArrayTypes.end())
    {
      return Found->second;
    }
    const std::uint64_t Alignment = ElementType->alignment();
    if (!detail::validAlignment(Alignment))
    {
      return nullptr;
    }
    std::uint64_t Size = 0;
    if (ElementCount != 0)
    {
      const std::uint64_t ElementSize = ElementType->size();
      const std::uint64_t MaximumSize = std::numeric_limits<std::uint64_t>::max();
      const std::uint64_t Padding = (Alignment - ElementSize % Alignment) % Alignment;
      if (ElementSize > MaximumSize - Padding)
      {
        return nullptr;
      }
      const std::uint64_t Stride = ElementSize + Padding;
      if (Stride != 0 && ElementCount > MaximumSize / Stride)
      {
        return nullptr;
      }
      Size = Stride * ElementCount;
    }
    const ArrayType *Result = own(std::unique_ptr<ArrayType>(new ArrayType(Meta, ElementType, ElementCount, Size, Alignment, Mutable)), Mutable ? nullptr : internArray(ElementType, ElementCount, true));
    ArrayTypes.emplace(Key, Result);
    return Result;
  }

  const FunctionType *TypePool::internFunction(const Type *ReturnType, std::span<const FunctionTypeParameter> Parameters, bool Mutable)
  {
    if (!Owner.containsType(ReturnType) || !ReturnType->hasLayout() || !validParameters(Parameters))
    {
      return nullptr;
    }
    FunctionKey Key{ReturnType, {}, Mutable};
    Key.Parameters.reserve(Parameters.size());
    for (const FunctionTypeParameter &Descriptor : Parameters)
    {
      Key.Parameters.emplace_back(Descriptor.ParameterType, Descriptor.Variadic);
    }
    const auto Found = FunctionTypes.find(Key);
    if (Found != FunctionTypes.end())
    {
      return Found->second;
    }
    const FunctionType *Result = own(std::unique_ptr<FunctionType>(new FunctionType(Meta, Owner.targetContext(), ReturnType, Parameters, Mutable)), Mutable ? nullptr : internFunction(ReturnType, Parameters, true));
    FunctionTypes.emplace(std::move(Key), Result);
    return Result;
  }
} // namespace ink::ir
