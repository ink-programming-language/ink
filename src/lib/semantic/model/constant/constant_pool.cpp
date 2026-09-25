#include "ink/semantic/model/constant/constant_pool.h"
#include "ink/semantic/context.h"

#include "../hash.h"

#include <functional>

namespace ink::semantic
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

  ConstantPool::ConstantPool(const SemanticContext &Context)
      : Context(Context)
  {
    FalseValue.reset(new BoolConstant(Context.getBoolType(), false));
    TrueValue.reset(new BoolConstant(Context.getBoolType(), true));
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
    return IntegerConstants.size() + StringConstants.size() + FloatConstants.size() + 2;
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
    default:
      return false;
    }
  }
} // namespace ink::semantic
