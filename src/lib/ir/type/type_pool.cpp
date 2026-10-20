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
        ConstNullPointer(Meta, Owner, false),
        Int8(Meta, Owner.targetContext(), 8, true),
        ConstInt8(Meta, Owner.targetContext(), 8, true, false),
        UInt8(Meta, Owner.targetContext(), 8, false),
        ConstUInt8(Meta, Owner.targetContext(), 8, false, false),
        Int16(Meta, Owner.targetContext(), 16, true),
        ConstInt16(Meta, Owner.targetContext(), 16, true, false),
        UInt16(Meta, Owner.targetContext(), 16, false),
        ConstUInt16(Meta, Owner.targetContext(), 16, false, false),
        Int32(Meta, Owner.targetContext(), 32, true),
        ConstInt32(Meta, Owner.targetContext(), 32, true, false),
        UInt32(Meta, Owner.targetContext(), 32, false),
        ConstUInt32(Meta, Owner.targetContext(), 32, false, false),
        Int64(Meta, Owner.targetContext(), 64, true),
        ConstInt64(Meta, Owner.targetContext(), 64, true, false),
        UInt64(Meta, Owner.targetContext(), 64, false),
        ConstUInt64(Meta, Owner.targetContext(), 64, false, false),
        Float32(Meta, Owner.targetContext(), 32),
        ConstFloat32(Meta, Owner.targetContext(), 32, false),
        Float64(Meta, Owner.targetContext(), 64),
        ConstFloat64(Meta, Owner.targetContext(), 64, false)
  {
    OwnedTypes.insert({&Meta, &Void, &ConstVoid, &Variadic, &Bool, &ConstBool, &NullPointer, &ConstNullPointer, &Int8, &ConstInt8, &UInt8, &ConstUInt8, &Int16, &ConstInt16, &UInt16, &ConstUInt16, &Int32, &ConstInt32, &UInt32, &ConstUInt32, &Int64, &ConstInt64, &UInt64, &ConstUInt64, &Float32, &ConstFloat32, &Float64, &ConstFloat64});
    ConstVoid.UnqualifiedType = &Void;
    ConstBool.UnqualifiedType = &Bool;
    ConstNullPointer.UnqualifiedType = &NullPointer;
    ConstInt8.UnqualifiedType = &Int8;
    ConstUInt8.UnqualifiedType = &UInt8;
    ConstInt16.UnqualifiedType = &Int16;
    ConstUInt16.UnqualifiedType = &UInt16;
    ConstInt32.UnqualifiedType = &Int32;
    ConstUInt32.UnqualifiedType = &UInt32;
    ConstInt64.UnqualifiedType = &Int64;
    ConstUInt64.UnqualifiedType = &UInt64;
    ConstFloat32.UnqualifiedType = &Float32;
    ConstFloat64.UnqualifiedType = &Float64;
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

  const MetaType *TypePool::getMetaType() const noexcept
  {
    return &Meta;
  }

  const VoidType *TypePool::getVoidType(bool Mutable) const noexcept
  {
    return Mutable ? &Void : &ConstVoid;
  }

  const VariadicType *TypePool::getVariadicType() const noexcept
  {
    return &Variadic;
  }

  const BoolType *TypePool::getBoolType(bool Mutable) const noexcept
  {
    return Mutable ? &Bool : &ConstBool;
  }

  const NullPointerType *TypePool::getNullPointerType(bool Mutable) const noexcept
  {
    return Mutable ? &NullPointer : &ConstNullPointer;
  }

  const IntegerType *TypePool::getInt8Type(bool Mutable) const noexcept
  {
    return Mutable ? &Int8 : &ConstInt8;
  }

  const IntegerType *TypePool::getUInt8Type(bool Mutable) const noexcept
  {
    return Mutable ? &UInt8 : &ConstUInt8;
  }

  const IntegerType *TypePool::getInt16Type(bool Mutable) const noexcept
  {
    return Mutable ? &Int16 : &ConstInt16;
  }

  const IntegerType *TypePool::getUInt16Type(bool Mutable) const noexcept
  {
    return Mutable ? &UInt16 : &ConstUInt16;
  }

  const IntegerType *TypePool::getInt32Type(bool Mutable) const noexcept
  {
    return Mutable ? &Int32 : &ConstInt32;
  }

  const IntegerType *TypePool::getUInt32Type(bool Mutable) const noexcept
  {
    return Mutable ? &UInt32 : &ConstUInt32;
  }

  const IntegerType *TypePool::getInt64Type(bool Mutable) const noexcept
  {
    return Mutable ? &Int64 : &ConstInt64;
  }

  const IntegerType *TypePool::getUInt64Type(bool Mutable) const noexcept
  {
    return Mutable ? &UInt64 : &ConstUInt64;
  }

  const FloatType *TypePool::getFloat32Type(bool Mutable) const noexcept
  {
    return Mutable ? &Float32 : &ConstFloat32;
  }

  const FloatType *TypePool::getFloat64Type(bool Mutable) const noexcept
  {
    return Mutable ? &Float64 : &ConstFloat64;
  }

  const IntegerType *TypePool::getIntegerType(std::uint32_t Width, bool Signed, bool Mutable) const noexcept
  {
    if (Width == 8)
    {
      return Signed ? getInt8Type(Mutable) : getUInt8Type(Mutable);
    }
    if (Width == 16)
    {
      return Signed ? getInt16Type(Mutable) : getUInt16Type(Mutable);
    }
    if (Width == 32)
    {
      return Signed ? getInt32Type(Mutable) : getUInt32Type(Mutable);
    }
    if (Width == 64)
    {
      return Signed ? getInt64Type(Mutable) : getUInt64Type(Mutable);
    }
    return nullptr;
  }

  const FloatType *TypePool::getFloatType(std::uint32_t Width, bool Mutable) const noexcept
  {
    return Width == 32 ? getFloat32Type(Mutable) : Width == 64 ? getFloat64Type(Mutable) : nullptr;
  }

  const Type *TypePool::getQualifiedType(const Type *TypeValue, bool Mutable)
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
        return getVoidType(false);
      case ValueKind::Bool:
        return getBoolType(false);
      case ValueKind::NullPointer:
        return getNullPointerType(false);
      case ValueKind::Integer:
      {
        const auto *Integer = static_cast<const IntegerType *>(TypeValue);
        return getIntegerType(Integer->width(), Integer->isSigned(), false);
      }
      case ValueKind::Float:
        return getFloatType(static_cast<const FloatType *>(TypeValue)->width(), false);
      case ValueKind::Pointer:
        return getPointerType(static_cast<const PointerType *>(TypeValue)->pointeeType(), false);
      case ValueKind::Reference:
        return getReferenceType(static_cast<const ReferenceType *>(TypeValue)->referencedType(), false);
      case ValueKind::Array:
      {
        const auto *Array = static_cast<const ArrayType *>(TypeValue);
        return getArrayType(Array->elementType(), Array->elementCount(), false);
      }
      case ValueKind::FunctionType:
      {
        const auto *Function = static_cast<const FunctionType *>(TypeValue);
        return getFunctionType(Function->returnType(), Function->parameters(), false);
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

  const PointerType *TypePool::getPointerType(const Type *PointeeType, bool Mutable)
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
    const PointerType *Result = own(std::unique_ptr<PointerType>(new PointerType(Meta, Owner.targetContext(), PointeeType, Mutable)), Mutable ? nullptr : getPointerType(PointeeType));
    PointerTypes.emplace(Key, Result);
    return Result;
  }

  const ReferenceType *TypePool::getReferenceType(const Type *ReferencedType, bool Mutable)
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
    const ReferenceType *Result = own(std::unique_ptr<ReferenceType>(new ReferenceType(Meta, Owner.targetContext(), ReferencedType, Mutable)), Mutable ? nullptr : getReferenceType(ReferencedType));
    ReferenceTypes.emplace(Key, Result);
    return Result;
  }

  const ArrayType *TypePool::getArrayType(const Type *ElementType, std::uint64_t ElementCount, bool Mutable)
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
    const ArrayType *Result = own(std::unique_ptr<ArrayType>(new ArrayType(Meta, ElementType, ElementCount, Size, Alignment, Mutable)), Mutable ? nullptr : getArrayType(ElementType, ElementCount));
    ArrayTypes.emplace(Key, Result);
    return Result;
  }

  const FunctionType *TypePool::getFunctionType(const Type *ReturnType, std::initializer_list<FunctionTypeParameter> Parameters, bool Mutable)
  {
    return getFunctionType(ReturnType, std::span<const FunctionTypeParameter>(Parameters.begin(), Parameters.size()), Mutable);
  }

  const FunctionType *TypePool::getFunctionType(const Type *ReturnType, std::span<const FunctionTypeParameter> Parameters, bool Mutable)
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
    const FunctionType *Result = own(std::unique_ptr<FunctionType>(new FunctionType(Meta, Owner.targetContext(), ReturnType, Parameters, Mutable)), Mutable ? nullptr : getFunctionType(ReturnType, Parameters));
    FunctionTypes.emplace(std::move(Key), Result);
    return Result;
  }
} // namespace ink::ir
