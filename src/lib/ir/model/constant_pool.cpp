#include "ink/ir/model/constant_pool.h"

#include <cassert>
#include <functional>
#include <unordered_map>

namespace ink::ir
{
  namespace
  {
    void combineHash(std::size_t &Seed, std::size_t Value) noexcept
    {
      Seed ^= Value + static_cast<std::size_t>(0x9E3779B9U) + (Seed << 6U) + (Seed >> 2U);
    }

    std::size_t constantHash(const Constant &ConstantValue) noexcept
    {
      std::size_t Result = std::hash<const Type *>{}(&ConstantValue.type());
      combineHash(Result, std::hash<unsigned int>{}(static_cast<unsigned int>(ConstantValue.kind())));
      switch (ConstantValue.kind())
      {
      case ValueKind::IntegerConstant:
      {
        const IntegerConstant &Integer = static_cast<const IntegerConstant &>(ConstantValue);
        combineHash(Result, std::hash<std::uint64_t>{}(Integer.unsignedValue()));
        combineHash(Result, std::hash<bool>{}(Integer.isNegative()));
        return Result;
      }
      case ValueKind::FloatConstant:
      {
        const FloatConstant &Float = static_cast<const FloatConstant &>(ConstantValue);
        combineHash(Result, std::hash<unsigned int>{}(static_cast<unsigned int>(Float.format())));
        combineHash(Result, std::hash<std::uint64_t>{}(Float.bitPattern()));
        return Result;
      }
      case ValueKind::StringConstant:
        combineHash(Result, std::hash<std::string>{}(static_cast<const StringConstant &>(ConstantValue).data()));
        return Result;
      case ValueKind::NullConstant:
      case ValueKind::ZeroInitializer:
        return Result;
      case ValueKind::AggregateConstant:
      {
        const AggregateConstant &Aggregate = static_cast<const AggregateConstant &>(ConstantValue);
        combineHash(Result, Aggregate.elements().size());
        for (const std::reference_wrapper<const Constant> Element : Aggregate.elements())
        {
          combineHash(Result, constantHash(Element.get()));
        }
        return Result;
      }
      case ValueKind::ValueOperand:
      case ValueKind::GlobalAddressOperand:
      case ValueKind::GlobalVariableAddressOperand:
        return Result;
      }
      return Result;
    }
  } // namespace

  class ConstantPool::Impl
  {
    public:
      const Constant &intern(std::unique_ptr<Constant> Candidate)
      {
        const std::size_t Hash = constantHash(*Candidate);
        std::vector<std::unique_ptr<Constant>> &Bucket = Constants[Hash];
        for (const std::unique_ptr<Constant> &Existing : Bucket)
        {
          if (constantsEqual(*Existing, *Candidate))
          {
            return *Existing;
          }
        }
        const Constant &Result = *Candidate;
        Bucket.push_back(std::move(Candidate));
        ++Size;
        return Result;
      }

      std::size_t size() const noexcept
      {
        return Size;
      }

      bool owns(const Constant &ConstantValue) const noexcept
      {
        const auto Bucket = Constants.find(constantHash(ConstantValue));
        if (Bucket == Constants.end())
        {
          return false;
        }
        for (const std::unique_ptr<Constant> &Existing : Bucket->second)
        {
          if (Existing.get() == &ConstantValue)
          {
            return true;
          }
        }
        return false;
      }

    private:
      std::unordered_map<std::size_t, std::vector<std::unique_ptr<Constant>>> Constants;
      std::size_t Size = 0;
  };

  ConstantPool::ConstantPool()
      : Implementation(std::make_unique<Impl>())
  {
  }

  ConstantPool::~ConstantPool() = default;

  const FloatConstant &ConstantPool::getFloatConstant(const Type &ValueType, FloatFormat Format, std::uint64_t BitPattern)
  {
    return static_cast<const FloatConstant &>(intern(std::unique_ptr<FloatConstant>(new FloatConstant(ValueType, Format, BitPattern))));
  }

  const StringConstant &ConstantPool::getStringConstant(const Type &ValueType, std::string Data)
  {
    return static_cast<const StringConstant &>(intern(std::unique_ptr<StringConstant>(new StringConstant(ValueType, std::move(Data)))));
  }

  const NullConstant &ConstantPool::getNullConstant(const Type &ValueType)
  {
    return static_cast<const NullConstant &>(intern(std::unique_ptr<NullConstant>(new NullConstant(ValueType))));
  }

  const ZeroInitializer &ConstantPool::getZeroInitializer(const Type &ValueType)
  {
    return static_cast<const ZeroInitializer &>(intern(std::unique_ptr<ZeroInitializer>(new ZeroInitializer(ValueType))));
  }

  const AggregateConstant &ConstantPool::getAggregateConstant(const Type &ValueType, const std::vector<std::reference_wrapper<const Constant>> &Elements)
  {
    std::vector<std::reference_wrapper<const Constant>> CanonicalElements;
    CanonicalElements.reserve(Elements.size());
    for (const std::reference_wrapper<const Constant> Element : Elements)
    {
      CanonicalElements.emplace_back(owns(Element.get()) ? Element.get() : internCopy(Element.get()));
    }
    return static_cast<const AggregateConstant &>(intern(std::unique_ptr<AggregateConstant>(new AggregateConstant(ValueType, std::move(CanonicalElements)))));
  }

  const AggregateConstant &ConstantPool::getAggregateConstant(const Type &ValueType, std::initializer_list<std::reference_wrapper<const Constant>> Elements)
  {
    return getAggregateConstant(ValueType, std::vector<std::reference_wrapper<const Constant>>(Elements));
  }

  std::size_t ConstantPool::size() const noexcept
  {
    return Implementation->size();
  }

  bool ConstantPool::owns(const Constant &ConstantValue) const noexcept
  {
    return Implementation->owns(ConstantValue);
  }

  const Constant &ConstantPool::intern(std::unique_ptr<Constant> Candidate)
  {
    assert(Candidate != nullptr);
    return Implementation->intern(std::move(Candidate));
  }

  const Constant &ConstantPool::internCopy(const Constant &Source)
  {
    switch (Source.kind())
    {
    case ValueKind::IntegerConstant:
    {
      const IntegerConstant &Integer = static_cast<const IntegerConstant &>(Source);
      return Integer.isNegative() ? static_cast<const Constant &>(getIntegerConstant(Integer.type(), Integer.signedValue())) : static_cast<const Constant &>(getIntegerConstant(Integer.type(), Integer.unsignedValue()));
    }
    case ValueKind::FloatConstant:
    {
      const FloatConstant &Float = static_cast<const FloatConstant &>(Source);
      return getFloatConstant(Float.type(), Float.format(), Float.bitPattern());
    }
    case ValueKind::StringConstant:
    {
      const StringConstant &String = static_cast<const StringConstant &>(Source);
      return getStringConstant(String.type(), String.data());
    }
    case ValueKind::NullConstant:
      return getNullConstant(Source.type());
    case ValueKind::ZeroInitializer:
      return getZeroInitializer(Source.type());
    case ValueKind::AggregateConstant:
    {
      const AggregateConstant &Aggregate = static_cast<const AggregateConstant &>(Source);
      return getAggregateConstant(Aggregate.type(), Aggregate.elements());
    }
    case ValueKind::ValueOperand:
    case ValueKind::GlobalAddressOperand:
    case ValueKind::GlobalVariableAddressOperand:
      break;
    }
    assert(false);
    return Source;
  }
} // namespace ink::ir
