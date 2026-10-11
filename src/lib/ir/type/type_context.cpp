#include "ink/ir/type/type_context.h"
#include "type_layout.h"

#include <functional>
#include <limits>
#include <utility>

namespace ink::ir
{
  bool TypeContext::FunctionKey::operator==(const FunctionKey &Other) const noexcept
  {
    if (ReturnType != Other.ReturnType || Parameters.size() != Other.Parameters.size())
    {
      return false;
    }
    for (std::size_t Index = 0; Index < Parameters.size(); ++Index)
    {
      const Parameter &Left = Parameters[Index];
      const Parameter &Right = Other.Parameters[Index];
      if (Left.type() != Right.type() || Left.name() != Right.name() || Left.isVariadic() != Right.isVariadic())
      {
        return false;
      }
    }
    return true;
  }

  std::size_t TypeContext::FunctionKeyHash::operator()(const FunctionKey &Key) const noexcept
  {
    std::size_t Hash = std::hash<const Type *>{}(Key.ReturnType);
    for (const Parameter &ParameterValue : Key.Parameters)
    {
      Hash ^= std::hash<const Type *>{}(ParameterValue.type()) + 0x9e3779b9U + (Hash << 6U) + (Hash >> 2U);
      Hash ^= std::hash<core::Name>{}(ParameterValue.name()) + 0x9e3779b9U + (Hash << 6U) + (Hash >> 2U);
      Hash ^= std::hash<bool>{}(ParameterValue.isVariadic()) + 0x9e3779b9U + (Hash << 6U) + (Hash >> 2U);
    }
    return Hash;
  }

  template <typename TypeT>
  const TypeT *TypeContext::own(std::unique_ptr<TypeT> TypeValue)
  {
    const TypeT *Result = TypeValue.get();
    OwnedTypes.insert(Result);
    Storage.push_back(std::move(TypeValue));
    return Result;
  }

  bool TypeContext::validParameters(const std::vector<Parameter> &Parameters) const noexcept
  {
    for (const Parameter &ParameterValue : Parameters)
    {
      const Type *ParameterType = ParameterValue.type();
      if (OwnedTypes.find(ParameterType) == OwnedTypes.end() || !ParameterType->hasLayout())
      {
        return false;
      }
    }
    return true;
  }

  TypeContext::TypeContext(target::TargetContext Target)
      : Target(Target),
        Int32(Target, 32, true),
        UInt32(Target, 32, false),
        Int64(Target, 64, true),
        UInt64(Target, 64, false),
        Float32(Target, 32),
        Float64(Target, 64),
        String(Target)
  {
    OwnedTypes.insert({&Void, &Bool, &Int32, &UInt32, &Int64, &UInt64, &Float32, &Float64, &String});
  }

  TypeContext::~TypeContext() noexcept = default;

  const target::TargetContext &TypeContext::targetContext() const noexcept
  {
    return Target;
  }

  bool TypeContext::owns(const Type *TypeValue) const noexcept
  {
    return OwnedTypes.find(TypeValue) != OwnedTypes.end();
  }

  const VoidType *TypeContext::getVoidType() const noexcept
  {
    return &Void;
  }

  const BoolType *TypeContext::getBoolType() const noexcept
  {
    return &Bool;
  }

  const IntegerType *TypeContext::getInt32Type() const noexcept
  {
    return &Int32;
  }

  const IntegerType *TypeContext::getUInt32Type() const noexcept
  {
    return &UInt32;
  }

  const IntegerType *TypeContext::getInt64Type() const noexcept
  {
    return &Int64;
  }

  const IntegerType *TypeContext::getUInt64Type() const noexcept
  {
    return &UInt64;
  }

  const FloatType *TypeContext::getFloat32Type() const noexcept
  {
    return &Float32;
  }

  const FloatType *TypeContext::getFloat64Type() const noexcept
  {
    return &Float64;
  }

  const StringType *TypeContext::getStringType() const noexcept
  {
    return &String;
  }

  const IntegerType *TypeContext::getIntegerType(std::uint32_t Width, bool Signed) const noexcept
  {
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

  const FloatType *TypeContext::getFloatType(std::uint32_t Width) const noexcept
  {
    return Width == 32 ? getFloat32Type() : Width == 64 ? getFloat64Type() : nullptr;
  }

  const PointerType *TypeContext::getPointerType(const Type *PointeeType)
  {
    if (!owns(PointeeType))
    {
      return nullptr;
    }
    const auto Found = PointerTypes.find(PointeeType);
    if (Found != PointerTypes.end())
    {
      return Found->second;
    }
    const PointerType *Result = own(std::unique_ptr<PointerType>(new PointerType(targetContext(), PointeeType)));
    PointerTypes.emplace(PointeeType, Result);
    return Result;
  }

  const ReferenceType *TypeContext::getReferenceType(const Type *ReferencedType)
  {
    if (!owns(ReferencedType))
    {
      return nullptr;
    }
    const auto Found = ReferenceTypes.find(ReferencedType);
    if (Found != ReferenceTypes.end())
    {
      return Found->second;
    }
    const ReferenceType *Result = own(std::unique_ptr<ReferenceType>(new ReferenceType(targetContext(), ReferencedType)));
    ReferenceTypes.emplace(ReferencedType, Result);
    return Result;
  }

  const ArrayType *TypeContext::getArrayType(const Type *ElementType, std::uint64_t ElementCount)
  {
    if (!owns(ElementType) || !ElementType->hasLayout())
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

  const FunctionType *TypeContext::getFunctionType(const Type *ReturnType, std::vector<Parameter> Parameters)
  {
    if (!owns(ReturnType) || !ReturnType->hasLayout() || !validParameters(Parameters))
    {
      return nullptr;
    }
    FunctionKey Key{ReturnType, std::move(Parameters)};
    const auto Found = FunctionTypes.find(Key);
    if (Found != FunctionTypes.end())
    {
      return Found->second;
    }
    const FunctionType *Result = own(std::unique_ptr<FunctionType>(new FunctionType(targetContext(), ReturnType, Key.Parameters)));
    FunctionTypes.emplace(std::move(Key), Result);
    return Result;
  }
} // namespace ink::ir
