#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/context.h"

#include <algorithm>
#include <cstring>
#include <functional>
#include <limits>
#include <string_view>
#include <utility>

#include <utf8proc.h>

namespace ink::ir
{
  static_assert(sizeof(float) == sizeof(std::uint32_t) && std::numeric_limits<float>::is_iec559 && std::numeric_limits<float>::radix == 2 && std::numeric_limits<float>::digits == 24 && std::numeric_limits<float>::max_exponent == 128 && std::numeric_limits<float>::min_exponent == -125, "ConstantPool numeric float lookups require IEEE binary32 float");
  static_assert(sizeof(double) == sizeof(std::uint64_t) && std::numeric_limits<double>::is_iec559 && std::numeric_limits<double>::radix == 2 && std::numeric_limits<double>::digits == 53 && std::numeric_limits<double>::max_exponent == 1024 && std::numeric_limits<double>::min_exponent == -1021, "ConstantPool numeric float lookups require IEEE binary64 double");
  static_assert(UTF8PROC_VERSION_MAJOR == 2 && UTF8PROC_VERSION_MINOR == 9 && UTF8PROC_VERSION_PATCH == 0, "Ink requires utf8proc 2.9.0 with Unicode 15.1 data");

  namespace
  {
    bool validUtf8(std::string_view Value) noexcept
    {
      std::size_t Offset = 0;
      while (Offset < Value.size())
      {
        // A scalar uses at most four bytes; bounded lengths avoid narrowing large strings.
        const auto Available = static_cast<utf8proc_ssize_t>(std::min(Value.size() - Offset, std::size_t{4}));
        utf8proc_int32_t CodePoint = 0;
        const utf8proc_ssize_t Length = utf8proc_iterate(reinterpret_cast<const utf8proc_uint8_t *>(Value.data() + Offset), Available, &CodePoint);
        if (Length <= 0)
        {
          return false;
        }
        Offset += static_cast<std::size_t>(Length);
      }
      return true;
    }
  } // namespace

  std::size_t ConstantPool::IntegerBitsHash::operator()(const core::IntegerBits &Bits) const noexcept
  {
    return Bits.hash();
  }

  bool ConstantPool::ArrayKey::operator==(const ArrayKey &Other) const noexcept
  {
    return ConstantType == Other.ConstantType && Elements == Other.Elements;
  }

  std::size_t ConstantPool::ArrayKeyHash::operator()(const ArrayKey &Key) const noexcept
  {
    std::size_t Hash = std::hash<const ArrayType *>{}(Key.ConstantType);
    for (const Constant *Element : Key.Elements)
    {
      Hash ^= std::hash<const Constant *>{}(Element) + 0x9e3779b9U + (Hash << 6U) + (Hash >> 2U);
    }
    return Hash;
  }

  template <typename ConstantT>
  const ConstantT *ConstantPool::own(std::unique_ptr<ConstantT> ConstantValue)
  {
    const ConstantT *Result = ConstantValue.get();
    OwnedConstants.insert(Result);
    Storage.push_back(std::move(ConstantValue));
    return Result;
  }

  ConstantPool::ConstantPool(IRContext &Owner)
      : Owner(Owner),
        False(Owner.typePool().get<BoolType>(), false),
        True(Owner.typePool().get<BoolType>(), true),
        NullPointer(Owner.typePool().get<NullPointerType>())
  {
    OwnedConstants.insert({&False, &True, &NullPointer});
  }

  ConstantPool::~ConstantPool() noexcept = default;

  const IRContext &ConstantPool::irContext() const noexcept
  {
    return Owner;
  }

  bool ConstantPool::owns(const Constant *ConstantValue) const noexcept
  {
    return OwnedConstants.find(ConstantValue) != OwnedConstants.end();
  }

  const IntegerConstant *ConstantPool::internIntegerFromBits(const IntegerType *IntegerTypeValue, const core::IntegerBits &Bits)
  {
    if (!irContext().containsType(IntegerTypeValue) || IntegerTypeValue->width() != Bits.bitWidth())
    {
      return nullptr;
    }
    auto &Constants = IntegerConstants[IntegerTypeValue];
    const auto Found = Constants.find(Bits);
    if (Found != Constants.end())
    {
      return Found->second;
    }
    const IntegerConstant *Result = own(std::unique_ptr<IntegerConstant>(new IntegerConstant(IntegerTypeValue, Bits)));
    Constants.emplace(Bits, Result);
    return Result;
  }

  const IntegerConstant *ConstantPool::internSignedInteger(const IntegerType *IntegerTypeValue, std::int64_t Value)
  {
    if (!irContext().containsType(IntegerTypeValue) || !IntegerTypeValue->isSigned())
    {
      return nullptr;
    }
    const auto Bits = core::IntegerBits::fromSigned(IntegerTypeValue->width(), Value);
    return Bits ? internIntegerFromBits(IntegerTypeValue, *Bits) : nullptr;
  }

  const IntegerConstant *ConstantPool::internUnsignedInteger(const IntegerType *IntegerTypeValue, std::uint64_t Value)
  {
    if (!irContext().containsType(IntegerTypeValue) || !IntegerTypeValue->isUnsigned())
    {
      return nullptr;
    }
    const auto Bits = core::IntegerBits::fromUnsigned(IntegerTypeValue->width(), Value);
    return Bits ? internIntegerFromBits(IntegerTypeValue, *Bits) : nullptr;
  }

  const FloatConstant *ConstantPool::internFloatFromBits(const FloatType *FloatTypeValue, const core::IntegerBits &Bits)
  {
    if (!irContext().containsType(FloatTypeValue) || FloatTypeValue->width() != Bits.bitWidth())
    {
      return nullptr;
    }
    const auto Value = Bits.tryGetUInt64();
    return Value ? internFloatFromBits(FloatTypeValue, *Value) : nullptr;
  }

  const FloatConstant *ConstantPool::internFloatFromBits(const FloatType *FloatTypeValue, std::uint64_t Bits)
  {
    if (!irContext().containsType(FloatTypeValue) || (FloatTypeValue->width() == 32 && Bits > std::numeric_limits<std::uint32_t>::max()))
    {
      return nullptr;
    }
    auto &Constants = FloatConstants[FloatTypeValue];
    const auto Found = Constants.find(Bits);
    if (Found != Constants.end())
    {
      return Found->second;
    }
    const FloatConstant *Result = own(std::unique_ptr<FloatConstant>(new FloatConstant(FloatTypeValue, Bits)));
    Constants.emplace(Bits, Result);
    return Result;
  }

  const FloatConstant *ConstantPool::internFloat(float Value)
  {
    std::uint32_t Bits = 0;
    std::memcpy(&Bits, &Value, sizeof(Bits));
    return internFloatFromBits(Owner.typePool().get<FloatType>(32), Bits);
  }

  const FloatConstant *ConstantPool::internFloat(double Value)
  {
    std::uint64_t Bits = 0;
    std::memcpy(&Bits, &Value, sizeof(Bits));
    return internFloatFromBits(Owner.typePool().get<FloatType>(64), Bits);
  }

  const ArrayConstant *ConstantPool::internArray(const ArrayType *ConstantType, std::vector<const Constant *> Elements)
  {
    if (!irContext().containsType(ConstantType) || Elements.size() != ConstantType->elementCount())
    {
      return nullptr;
    }
    for (const Constant *Element : Elements)
    {
      if (!owns(Element) || !Element->canInitialize(ConstantType->elementType()))
      {
        return nullptr;
      }
    }
    ArrayKey Key{ConstantType, std::move(Elements)};
    const auto Found = ArrayConstants.find(Key);
    if (Found != ArrayConstants.end())
    {
      return Found->second;
    }
    const ArrayConstant *Result = own(std::unique_ptr<ArrayConstant>(new ArrayConstant(ConstantType, Key.Elements)));
    ArrayConstants.emplace(std::move(Key), Result);
    return Result;
  }

  const StringConstant *ConstantPool::internString(std::string Value)
  {
    if (Value.size() > Owner.targetContext().maximumPointerSizeValue())
    {
      return nullptr;
    }
    auto &Constants = StringConstants;
    const auto Found = Constants.find(std::string_view(Value));
    if (Found != Constants.end())
    {
      return Found->second;
    }
    if (!validUtf8(Value))
    {
      return nullptr;
    }
    const ArrayType *ArrayTypeValue = Owner.typePool().get<ArrayType>(Owner.typePool().get<IntegerType>(8, false), Value.size());
    if (!ArrayTypeValue)
    {
      return nullptr;
    }
    const StringConstant *Result = own(std::unique_ptr<StringConstant>(new StringConstant(ArrayTypeValue, std::move(Value))));
    Constants.emplace(Result->value(), Result);
    return Result;
  }
} // namespace ink::ir
