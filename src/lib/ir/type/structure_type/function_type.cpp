#include "ink/ir/type/structure_type/function_type.h"

#include <utility>

namespace ink::ir
{
  FunctionType::FunctionType(const target::TargetContext &Target, const Type *ReturnType, std::span<const Parameter *const> Parameters)
      : Type(ValueKind::FunctionType),
        ReturnType(ReturnType),
        PointerByteWidth(Target.pointerByteWidth())
  {
    this->Parameters.reserve(Parameters.size());
    for (const Parameter *Descriptor : Parameters)
    {
      auto ParameterValue = std::make_unique<Parameter>(Descriptor->type(), Descriptor->isVariadic());
      ParameterValue->Index = this->Parameters.size();
      this->Parameters.push_back(std::move(ParameterValue));
    }
  }

  const Type *FunctionType::returnType() const noexcept
  {
    return ReturnType;
  }

  const std::vector<std::unique_ptr<const Parameter>> &FunctionType::parameters() const noexcept
  {
    return Parameters;
  }

  std::size_t FunctionType::parameterCount() const noexcept
  {
    return Parameters.size();
  }

  std::uint64_t FunctionType::size() const noexcept
  {
    return PointerByteWidth;
  }

  std::uint64_t FunctionType::alignment() const noexcept
  {
    return PointerByteWidth;
  }

  bool FunctionType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::FunctionType;
  }
} // namespace ink::ir
