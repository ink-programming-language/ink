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
    return combineHash(std::hash<const Value *>{}(ElementType), std::hash<std::uint64_t>{}(ElementCount));
  }

  bool ArrayType::operator==(const ArrayType &Other) const noexcept
  {
    return arrayKey(*this) == arrayKey(Other);
  }

  bool ArrayType::operator==(const Key &Other) const noexcept
  {
    return arrayKey(*this) == Other;
  }

  std::size_t PointerType::hash() const noexcept
  {
    return hash(&PointeeType);
  }

  std::size_t PointerType::hash(const Value *Value) noexcept
  {
    return std::hash<const ink::ir::Value *>{}(Value);
  }

  bool PointerType::operator==(const PointerType &Other) const noexcept
  {
    return &PointeeType == &Other.PointeeType;
  }

  bool PointerType::operator==(const Value *Other) const noexcept
  {
    return &PointeeType == Other;
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

  const MetaType &TypePool::getMetaType() const noexcept
  {
    return MetaType::instance();
  }

  StructType &TypePool::createStructType()
  {
    return Structs.emplace_back();
  }

  ObjectType &TypePool::createObjectType()
  {
    return Objects.emplace_back();
  }

  const ArrayType &TypePool::getArrayType(ValueRef ElementType, std::uint64_t ElementCount)
  {
    if (!ElementType.get().isTypeValue())
    {
      panic("TypePool requires an array element whose type is MetaType");
    }
    const auto Existing = Arrays.find(ArrayType::Key{&ElementType.get(), ElementCount});
    if (Existing != Arrays.end())
    {
      return *Existing;
    }
    return *Arrays.emplace(ElementType, ElementCount).first;
  }

  const PointerType &TypePool::getPointerType(ValueRef PointeeType)
  {
    if (!PointeeType.get().isTypeValue())
    {
      panic("TypePool requires a pointer target whose type is MetaType");
    }
    const auto Existing = Pointers.find(&PointeeType.get());
    if (Existing != Pointers.end())
    {
      return *Existing;
    }
    return *Pointers.emplace(PointeeType).first;
  }

  const FunctionType &TypePool::getFunctionType(ValueRef ReturnType, std::span<const Parameter> Parameters, CallABI ABI)
  {
    if (!ReturnType.get().isTypeValue())
    {
      panic("TypePool requires a function return type whose type is MetaType");
    }
    for (const auto &ParameterValue : Parameters)
    {
      if (!ParameterValue.type().isTypeValue())
      {
        panic("TypePool requires function parameter types whose type is MetaType");
      }
    }
    const auto Existing = Functions.find(FunctionType::Key{&ReturnType.get(), Parameters, ABI});
    if (Existing != Functions.end())
    {
      return *Existing;
    }
    return *Functions.emplace(ReturnType, Parameters, ABI).first;
  }
} // namespace ink::ir
