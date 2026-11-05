#include "ink/ir/type/type_pool.h"

#include "ink/core/support/core_define.h"

#include <cstddef>
#include <functional>

namespace ink::ir
{
  namespace
  {
    ArrayType::Key arrayKey(const ArrayType &Value) noexcept
    {
      return {&Value.elementType(), Value.elementCount()};
    }

    std::size_t combineHash(std::size_t First, std::size_t Second) noexcept
    {
      return First ^ (Second + std::size_t{0x9e3779b9} + (First << 6) + (First >> 2));
    }
  } // namespace

  std::size_t ArrayType::hash() const noexcept
  {
    return arrayKey(*this).hash();
  }

  std::size_t ArrayType::Key::hash() const noexcept
  {
    return combineHash(std::hash<const Type *>{}(ElementType), std::hash<std::uint64_t>{}(ElementCount));
  }

  bool ArrayType::operator==(const ArrayType &Other) const noexcept
  {
    return arrayKey(*this) == arrayKey(Other);
  }

  bool ArrayType::operator==(const Key &Other) const noexcept
  {
    return arrayKey(*this) == Other;
  }

  std::size_t ReferenceType::hash() const noexcept
  {
    return hash(&ReferencedType);
  }

  std::size_t ReferenceType::hash(const Type *Value) noexcept
  {
    return std::hash<const Type *>{}(Value);
  }

  bool ReferenceType::operator==(const ReferenceType &Other) const noexcept
  {
    return &ReferencedType == &Other.ReferencedType;
  }

  bool ReferenceType::operator==(const Type *Other) const noexcept
  {
    return &ReferencedType == Other;
  }

  const IntegerType &TypePool::getIntegerType(IntegerType::Width Width, IntegerType::Signedness Sign) const noexcept
  {
    std::size_t Index;
    switch (Width)
    {
      case IntegerType::Width::Bits8:
        Index = 0;
        break;
      case IntegerType::Width::Bits16:
        Index = 2;
        break;
      case IntegerType::Width::Bits32:
        Index = 4;
        break;
      case IntegerType::Width::Bits64:
        Index = 6;
        break;
      default:
        panic("TypePool requires a supported integer width");
    }
    switch (Sign)
    {
      case IntegerType::Signedness::Signed:
        return Integers[Index];
      case IntegerType::Signedness::Unsigned:
        return Integers[Index + 1];
      default:
        panic("TypePool requires a supported integer signedness");
    }
  }

  const FloatType &TypePool::getFloatType(FloatType::Width Width) const noexcept
  {
    switch (Width)
    {
      case FloatType::Width::Bits32:
        return Floats[0];
      case FloatType::Width::Bits64:
        return Floats[1];
      default:
        panic("TypePool requires a supported floating point width");
    }
  }

  const BoolType &TypePool::getBoolType() const noexcept
  {
    return Bool;
  }

  const VoidType &TypePool::getVoidType() const noexcept
  {
    return Void;
  }

  const ArrayType &TypePool::getArrayType(const Type &ElementType, std::uint64_t ElementCount)
  {
    const auto Existing = Arrays.find(ArrayType::Key{&ElementType, ElementCount});
    if (Existing != Arrays.end())
    {
      return *Existing;
    }
    return *Arrays.emplace(ElementType, ElementCount).first;
  }

  const ReferenceType &TypePool::getReferenceType(const Type &ReferencedType)
  {
    const auto Existing = References.find(&ReferencedType);
    if (Existing != References.end())
    {
      return *Existing;
    }
    return *References.emplace(ReferencedType).first;
  }

  const FunctionType &TypePool::getFunctionType(const Type &ReturnType, std::span<const Parameter> Parameters)
  {
    const auto Existing = Functions.find(FunctionType::Key{&ReturnType, Parameters});
    if (Existing != Functions.end())
    {
      return *Existing;
    }
    return *Functions.emplace(ReturnType, Parameters).first;
  }
} // namespace ink::ir
