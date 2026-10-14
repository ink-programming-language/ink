#include "ink/ir/type/type_pool.h"
#include "ink/ir/context.h"
#include "type_layout.h"

#include <functional>
#include <limits>
#include <utility>

namespace ink::ir
{
  TypePool::TypePool(IRContext &Owner)
      : Owner(Owner),
        NullPointer(Owner),
        UInt8(Owner.targetContext(), 8, false),
        Int32(Owner.targetContext(), 32, true),
        UInt32(Owner.targetContext(), 32, false),
        Int64(Owner.targetContext(), 64, true),
        UInt64(Owner.targetContext(), 64, false),
        Float32(Owner.targetContext(), 32),
        Float64(Owner.targetContext(), 64)
  {
    OwnedTypes.insert({&Void, &Variadic, &Bool, &NullPointer, &UInt8, &Int32, &UInt32, &Int64, &UInt64, &Float32, &Float64});
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

  bool TypePool::FunctionKey::operator==(const FunctionKey &Other) const noexcept
  {
    return ReturnType == Other.ReturnType && Parameters == Other.Parameters;
  }

  std::size_t TypePool::FunctionKeyHash::operator()(const FunctionKey &Key) const noexcept
  {
    std::size_t Hash = std::hash<const Type *>{}(Key.ReturnType);
    for (const Type *ParameterType : Key.Parameters)
    {
      Hash ^= std::hash<const Type *>{}(ParameterType) + 0x9e3779b9U + (Hash << 6U) + (Hash >> 2U);
    }
    return Hash;
  }

  template <typename TypeT>
  TypeT *TypePool::own(std::unique_ptr<TypeT> TypeValue)
  {
    TypeT *Result = TypeValue.get();
    OwnedTypes.insert(Result);
    Storage.push_back(std::move(TypeValue));
    return Result;
  }

  bool TypePool::validParameters(const std::vector<const Type *> &Parameters) const noexcept
  {
    for (const Type *ParameterType : Parameters)
    {
      if (!Owner.containsType(ParameterType) || !ParameterType->hasLayout())
      {
        return false;
      }
    }
    return true;
  }

  const VoidType *TypePool::getVoidType() const noexcept
  {
    return &Void;
  }

  const VariadicType *TypePool::getVariadicType() const noexcept
  {
    return &Variadic;
  }

  const BoolType *TypePool::getBoolType() const noexcept
  {
    return &Bool;
  }

  const NullPointerType *TypePool::getNullPointerType() const noexcept
  {
    return &NullPointer;
  }

  const IntegerType *TypePool::getUInt8Type() const noexcept
  {
    return &UInt8;
  }

  const IntegerType *TypePool::getInt32Type() const noexcept
  {
    return &Int32;
  }

  const IntegerType *TypePool::getUInt32Type() const noexcept
  {
    return &UInt32;
  }

  const IntegerType *TypePool::getInt64Type() const noexcept
  {
    return &Int64;
  }

  const IntegerType *TypePool::getUInt64Type() const noexcept
  {
    return &UInt64;
  }

  const FloatType *TypePool::getFloat32Type() const noexcept
  {
    return &Float32;
  }

  const FloatType *TypePool::getFloat64Type() const noexcept
  {
    return &Float64;
  }

  const IntegerType *TypePool::getIntegerType(std::uint32_t Width, bool Signed) const noexcept
  {
    if (Width == 8 && !Signed)
    {
      return getUInt8Type();
    }
    if (Width == 32)
    {
      return Signed ? getInt32Type() : getUInt32Type();
    }
    if (Width == 64)
    {
      return Signed ? getInt64Type() : getUInt64Type();
    }
    return nullptr;
  }

  const FloatType *TypePool::getFloatType(std::uint32_t Width) const noexcept
  {
    return Width == 32 ? getFloat32Type() : Width == 64 ? getFloat64Type() : nullptr;
  }

  const PointerType *TypePool::getPointerType(const Type *PointeeType)
  {
    if (!Owner.containsType(PointeeType))
    {
      return nullptr;
    }
    const auto Found = PointerTypes.find(PointeeType);
    if (Found != PointerTypes.end())
    {
      return Found->second;
    }
    const PointerType *Result = own(std::unique_ptr<PointerType>(new PointerType(Owner.targetContext(), PointeeType)));
    PointerTypes.emplace(PointeeType, Result);
    return Result;
  }

  const ReferenceType *TypePool::getReferenceType(const Type *ReferencedType)
  {
    if (!Owner.containsType(ReferencedType))
    {
      return nullptr;
    }
    const auto Found = ReferenceTypes.find(ReferencedType);
    if (Found != ReferenceTypes.end())
    {
      return Found->second;
    }
    const ReferenceType *Result = own(std::unique_ptr<ReferenceType>(new ReferenceType(Owner.targetContext(), ReferencedType)));
    ReferenceTypes.emplace(ReferencedType, Result);
    return Result;
  }

  const ArrayType *TypePool::getArrayType(const Type *ElementType, std::uint64_t ElementCount)
  {
    if (!Owner.containsType(ElementType) || !ElementType->hasLayout())
    {
      return nullptr;
    }
    const auto FoundElement = ArrayTypes.find(ElementType);
    if (FoundElement != ArrayTypes.end())
    {
      const auto FoundCount = FoundElement->second.find(ElementCount);
      if (FoundCount != FoundElement->second.end())
      {
        return FoundCount->second;
      }
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
    const ArrayType *Result = own(std::unique_ptr<ArrayType>(new ArrayType(ElementType, ElementCount, Size, Alignment)));
    ArrayTypes[ElementType].emplace(ElementCount, Result);
    return Result;
  }

  const FunctionType *TypePool::getFunctionType(const Type *ReturnType, std::vector<const Type *> Parameters)
  {
    if (!Owner.containsType(ReturnType) || !ReturnType->hasLayout() || !validParameters(Parameters))
    {
      return nullptr;
    }
    FunctionKey Key{ReturnType, std::move(Parameters)};
    const auto Found = FunctionTypes.find(Key);
    if (Found != FunctionTypes.end())
    {
      return Found->second;
    }
    const FunctionType *Result = own(std::unique_ptr<FunctionType>(new FunctionType(Owner.targetContext(), ReturnType, Key.Parameters)));
    FunctionTypes.emplace(std::move(Key), Result);
    return Result;
  }
} // namespace ink::ir
