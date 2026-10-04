#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/context.h"
#include "ink/ir/analysis/type_layout.h"

#include "../hash.h"

#include <functional>
#include <algorithm>

namespace ink::ir
{
  namespace
  {
    std::size_t integerConstantHash(const Type &ValueType, const IntegerBits &Payload) noexcept
    {
      std::size_t Hash = combineHash(std::hash<const Type *>{}(&ValueType), Payload.bitWidth());
      for (std::uint64_t Word : Payload.words())
      {
        Hash = combineHash(Hash, std::hash<std::uint64_t>{}(Word));
      }
      return Hash;
    }

    std::size_t stringConstantHash(const Type &ValueType, std::string_view Payload) noexcept
    {
      return combineHash(std::hash<const Type *>{}(&ValueType), std::hash<std::string_view>{}(Payload));
    }

    std::size_t floatConstantHash(const Type &ValueType, const FloatBits &Payload) noexcept
    {
      return combineHash(combineHash(std::hash<const Type *>{}(&ValueType), Payload.bitWidth()), std::hash<std::uint64_t>{}(Payload.bits()));
    }

    bool isStringType(const SliceType &ValueType) noexcept
    {
      if (ValueType.access() != AccessKind::ReadOnly || !IntegerType::classof(&ValueType.elementType()))
      {
        return false;
      }
      const auto &Element = static_cast<const IntegerType &>(ValueType.elementType());
      return Element.bitWidth() == 8 && !Element.isSigned();
    }

    std::size_t arrayConstantHash(const Type &ValueType, std::span<const Constant *const> Elements) noexcept
    {
      std::size_t Hash = std::hash<const Type *>{}(&ValueType);
      for (const Constant *Element : Elements)
      {
        Hash = combineHash(Hash, std::hash<const Constant *>{}(Element));
      }
      return Hash;
    }

    template <typename ConstantType>
    bool containsConstant(const std::unordered_multimap<std::size_t, std::unique_ptr<ConstantType>> &Constants, std::size_t Hash, const Constant &ConstantValue) noexcept
    {
      const auto Candidates = Constants.equal_range(Hash);
      for (auto Entry = Candidates.first; Entry != Candidates.second; ++Entry)
      {
        if (Entry->second.get() == &ConstantValue)
        {
          return true;
        }
      }
      return false;
    }
  } // namespace

  ConstantPool::ConstantPool(const IRContext &Context)
      : Context(Context)
  {
    FalseValue.reset(new BoolConstant(Context.typePool().getType<TypeKind::Bool>(), false));
    TrueValue.reset(new BoolConstant(Context.typePool().getType<TypeKind::Bool>(), true));
  }

  ConstantPool::~ConstantPool() = default;

  const IntegerConstant *ConstantPool::getIntegerConstant(const IntegerType &ValueType, const IntegerBits &Payload)
  {
    if (&ValueType.context() != &Context || !Payload.valid() || ValueType.bitWidth() != Payload.bitWidth())
    {
      return nullptr;
    }
    const std::size_t Hash = integerConstantHash(ValueType, Payload);
    const auto Candidates = IntegerConstants.equal_range(Hash);
    for (auto Entry = Candidates.first; Entry != Candidates.second; ++Entry)
    {
      const IntegerConstant &Candidate = *Entry->second;
      if (&Candidate.type() == &ValueType && Candidate.value() == Payload)
      {
        return &Candidate;
      }
    }
    auto Result = std::unique_ptr<IntegerConstant>(new IntegerConstant(ValueType, Payload));
    const IntegerConstant *Pointer = Result.get();
    IntegerConstants.emplace(Hash, std::move(Result));
    return Pointer;
  }

  const BoolConstant &ConstantPool::getBoolConstant(bool Payload) const noexcept
  {
    return Payload ? *TrueValue : *FalseValue;
  }

  const StringConstant *ConstantPool::getStringConstant(const SliceType &ValueType, std::string_view Payload)
  {
    if (&ValueType.context() != &Context || !isStringType(ValueType))
    {
      return nullptr;
    }
    const std::size_t Hash = stringConstantHash(ValueType, Payload);
    const auto Candidates = StringConstants.equal_range(Hash);
    for (auto Entry = Candidates.first; Entry != Candidates.second; ++Entry)
    {
      const StringConstant &Candidate = *Entry->second;
      if (&Candidate.type() == &ValueType && Candidate.value() == Payload)
      {
        return &Candidate;
      }
    }
    auto Result = std::unique_ptr<StringConstant>(new StringConstant(ValueType, Payload));
    const StringConstant *Pointer = Result.get();
    StringConstants.emplace(Hash, std::move(Result));
    return Pointer;
  }

  const FloatConstant *ConstantPool::getFloatConstant(const FloatType &ValueType, const FloatBits &Payload)
  {
    if (&ValueType.context() != &Context || !Payload.valid() || ValueType.bitWidth() != Payload.bitWidth())
    {
      return nullptr;
    }
    const std::size_t Hash = floatConstantHash(ValueType, Payload);
    const auto Candidates = FloatConstants.equal_range(Hash);
    for (auto Entry = Candidates.first; Entry != Candidates.second; ++Entry)
    {
      const FloatConstant &Candidate = *Entry->second;
      if (&Candidate.type() == &ValueType && Candidate.value() == Payload)
      {
        return &Candidate;
      }
    }
    auto Result = std::unique_ptr<FloatConstant>(new FloatConstant(ValueType, Payload));
    const FloatConstant *Pointer = Result.get();
    FloatConstants.emplace(Hash, std::move(Result));
    return Pointer;
  }

  std::size_t ConstantPool::size() const noexcept
  {
    return IntegerConstants.size() + StringConstants.size() + FloatConstants.size() + ArrayConstants.size() + ClassConstants.size() + 2;
  }

  const ArrayConstant *ConstantPool::getArrayConstant(const ArrayType &ValueType, std::span<const Constant *const> Elements)
  {
    if (&ValueType.context() != &Context || ValueType.elementCount() != Elements.size())
    {
      return nullptr;
    }
    for (const Constant *Element : Elements)
    {
      if (!Element || &Element->type() != &ValueType.elementType() || !owns(*Element))
      {
        return nullptr;
      }
    }
    const std::size_t Hash = arrayConstantHash(ValueType, Elements);
    const auto Candidates = ArrayConstants.equal_range(Hash);
    for (auto Entry = Candidates.first; Entry != Candidates.second; ++Entry)
    {
      const ArrayConstant &Candidate = *Entry->second;
      if (&Candidate.type() == &ValueType && std::equal(Candidate.elements().begin(), Candidate.elements().end(), Elements.begin(), Elements.end()))
      {
        return &Candidate;
      }
    }
    auto Result = std::unique_ptr<ArrayConstant>(new ArrayConstant(ValueType, Elements));
    const ArrayConstant *Pointer = Result.get();
    ArrayConstants.emplace(Hash, std::move(Result));
    return Pointer;
  }

  const ClassConstant *ConstantPool::getClassConstant(const ClassType &ValueType, std::span<const Constant *const> Fields)
  {
    if (&ValueType.context() != &Context || !computeTypeLayout(ValueType, Context.compilationContext().targetContext()) || ValueType.fields().size() != Fields.size())
    {
      return nullptr;
    }
    for (std::size_t Index = 0; Index < Fields.size(); ++Index)
    {
      if (!Fields[Index] || &Fields[Index]->type() != ValueType.fields()[Index].FieldType || !owns(*Fields[Index]))
      {
        return nullptr;
      }
    }
    const std::size_t Hash = arrayConstantHash(ValueType, Fields);
    const auto Candidates = ClassConstants.equal_range(Hash);
    for (auto Entry = Candidates.first; Entry != Candidates.second; ++Entry)
    {
      const ClassConstant &Candidate = *Entry->second;
      if (&Candidate.type() == &ValueType && std::equal(Candidate.fields().begin(), Candidate.fields().end(), Fields.begin(), Fields.end()))
      {
        return &Candidate;
      }
    }
    auto Result = std::unique_ptr<ClassConstant>(new ClassConstant(ValueType, Fields));
    const ClassConstant *Pointer = Result.get();
    ClassConstants.emplace(Hash, std::move(Result));
    return Pointer;
  }

  bool ConstantPool::owns(const Constant &ConstantValue) const noexcept
  {
    if (&ConstantValue == FalseValue.get() || &ConstantValue == TrueValue.get())
    {
      return true;
    }
    if (&ConstantValue.context() != &Context)
    {
      return false;
    }
    switch (ConstantValue.kind())
    {
    case ValueKind::IntegerConstant:
      return containsConstant(IntegerConstants, integerConstantHash(ConstantValue.type(), static_cast<const IntegerConstant &>(ConstantValue).value()), ConstantValue);
    case ValueKind::StringConstant:
      return containsConstant(StringConstants, stringConstantHash(ConstantValue.type(), static_cast<const StringConstant &>(ConstantValue).value()), ConstantValue);
    case ValueKind::FloatConstant:
      return containsConstant(FloatConstants, floatConstantHash(ConstantValue.type(), static_cast<const FloatConstant &>(ConstantValue).value()), ConstantValue);
    case ValueKind::ArrayConstant:
      return containsConstant(ArrayConstants, arrayConstantHash(ConstantValue.type(), static_cast<const ArrayConstant &>(ConstantValue).elements()), ConstantValue);
    case ValueKind::ClassConstant:
      return containsConstant(ClassConstants, arrayConstantHash(ConstantValue.type(), static_cast<const ClassConstant &>(ConstantValue).fields()), ConstantValue);
    default:
      return false;
    }
  }
} // namespace ink::ir
