#include "ink/ir/constant/string_constant.h"

#include <utility>

namespace ink::ir
{
  StringConstant::StringConstant(const ArrayType *ArrayTypeValue, std::string Value) noexcept
      : Constant(ValueKind::StringConstant, ArrayTypeValue),
        StoredValue(std::move(Value))
  {
  }

  std::string_view StringConstant::value() const noexcept
  {
    return StoredValue;
  }

  std::size_t StringConstant::byteLength() const noexcept
  {
    return StoredValue.size();
  }

  bool StringConstant::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::StringConstant;
  }
} // namespace ink::ir
