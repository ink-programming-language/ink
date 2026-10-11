#include "ink/ir/constant/array_constant.h"

#include <utility>

namespace ink::ir
{
  ArrayConstant::ArrayConstant(const ArrayType *ConstantType, std::vector<const Constant *> Elements) noexcept
      : Constant(ValueKind::ArrayConstant, ConstantType),
        Elements(std::move(Elements))
  {
  }

  const std::vector<const Constant *> &ArrayConstant::elements() const noexcept
  {
    return Elements;
  }

  std::size_t ArrayConstant::elementCount() const noexcept
  {
    return Elements.size();
  }

  bool ArrayConstant::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::ArrayConstant;
  }
} // namespace ink::ir
