#include "ink/semantic/model/constant_pool.h"
#include "ink/semantic/model/context.h"

#include "hash.h"

#include <functional>
#include <unordered_map>

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

    std::size_t stringConstHash(const Type &ValueType, std::string_view Payload) noexcept
    {
      return combineHash(std::hash<const Type *>{}(&ValueType), std::hash<std::string_view>{}(Payload));
    }

    std::size_t floatConstHash(const Type &ValueType, const FloatBits &Payload) noexcept
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

  class ConstantPool::Impl
  {
    public:
      std::unordered_multimap<std::size_t, std::unique_ptr<IntegerConstant>> IntegerConstants;
      std::unordered_multimap<std::size_t, std::unique_ptr<StringConst>> StringConstants;
      std::unordered_multimap<std::size_t, std::unique_ptr<FloatConst>> FloatConstants;
      std::unique_ptr<BoolConstant> FalseValue;
      std::unique_ptr<BoolConstant> TrueValue;
  };

  ConstantPool::ConstantPool(const SemanticContext &Context)
      : Context(Context),
        Storage(std::make_unique<Impl>())
  {
    Storage->FalseValue.reset(new BoolConstant(Context.getBoolType(), false));
    Storage->TrueValue.reset(new BoolConstant(Context.getBoolType(), true));
  }

  ConstantPool::~ConstantPool() = default;

  const IntegerConstant *ConstantPool::getIntegerConstant(const IntegerType &ValueType, const IntegerBits &Payload)
  {
    if (&ValueType.context() != &Context || !Payload.valid() || ValueType.bitWidth() != Payload.bitWidth())
    {
      return nullptr;
    }
    const std::size_t Hash = integerConstantHash(ValueType, Payload);
    const auto Candidates = Storage->IntegerConstants.equal_range(Hash);
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
    Storage->IntegerConstants.emplace(Hash, std::move(Result));
    return Pointer;
  }

  const BoolConstant &ConstantPool::getBoolConstant(bool Payload) const noexcept
  {
    return Payload ? *Storage->TrueValue : *Storage->FalseValue;
  }

  const StringConst *ConstantPool::getStringConst(const SliceType &ValueType, std::string_view Payload)
  {
    if (&ValueType.context() != &Context || !isStringType(ValueType))
    {
      return nullptr;
    }
    const std::size_t Hash = stringConstHash(ValueType, Payload);
    const auto Candidates = Storage->StringConstants.equal_range(Hash);
    for (auto Entry = Candidates.first; Entry != Candidates.second; ++Entry)
    {
      const StringConst &Candidate = *Entry->second;
      if (&Candidate.type() == &ValueType && Candidate.value() == Payload)
      {
        return &Candidate;
      }
    }
    auto Result = std::unique_ptr<StringConst>(new StringConst(ValueType, Payload));
    const StringConst *Pointer = Result.get();
    Storage->StringConstants.emplace(Hash, std::move(Result));
    return Pointer;
  }

  const FloatConst *ConstantPool::getFloatConst(const FloatType &ValueType, const FloatBits &Payload)
  {
    if (&ValueType.context() != &Context || !Payload.valid() || ValueType.bitWidth() != Payload.bitWidth())
    {
      return nullptr;
    }
    const std::size_t Hash = floatConstHash(ValueType, Payload);
    const auto Candidates = Storage->FloatConstants.equal_range(Hash);
    for (auto Entry = Candidates.first; Entry != Candidates.second; ++Entry)
    {
      const FloatConst &Candidate = *Entry->second;
      if (&Candidate.type() == &ValueType && Candidate.value() == Payload)
      {
        return &Candidate;
      }
    }
    auto Result = std::unique_ptr<FloatConst>(new FloatConst(ValueType, Payload));
    const FloatConst *Pointer = Result.get();
    Storage->FloatConstants.emplace(Hash, std::move(Result));
    return Pointer;
  }

  std::size_t ConstantPool::size() const noexcept
  {
    return Storage->IntegerConstants.size() + Storage->StringConstants.size() + Storage->FloatConstants.size() + 2;
  }

  bool ConstantPool::owns(const Constant &ConstantValue) const noexcept
  {
    if (&ConstantValue == Storage->FalseValue.get() || &ConstantValue == Storage->TrueValue.get())
    {
      return true;
    }
    if (&ConstantValue.type().context() != &Context)
    {
      return false;
    }
    switch (ConstantValue.kind())
    {
    case ValueKind::IntegerConstant:
      return containsConstant(Storage->IntegerConstants, integerConstantHash(ConstantValue.type(), static_cast<const IntegerConstant &>(ConstantValue).value()), ConstantValue);
    case ValueKind::StringConst:
      return containsConstant(Storage->StringConstants, stringConstHash(ConstantValue.type(), static_cast<const StringConst &>(ConstantValue).value()), ConstantValue);
    case ValueKind::FloatConst:
      return containsConstant(Storage->FloatConstants, floatConstHash(ConstantValue.type(), static_cast<const FloatConst &>(ConstantValue).value()), ConstantValue);
    default:
      return false;
    }
  }
} // namespace ink::semantic
