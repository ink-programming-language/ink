#include "ink/ir/type/function_type.h"

#include <utility>

namespace ink::ir
{
  FunctionType::FunctionType(const target::TargetContext &Target, const Type *ReturnType, std::vector<Parameter> Parameters) noexcept
      : Type(ValueKind::Function),
        ReturnType(ReturnType),
        Parameters(std::move(Parameters)),
        PointerByteWidth(Target.pointerByteWidth())
  {
  }

  const Type *FunctionType::returnType() const noexcept
  {
    return ReturnType;
  }

  const std::vector<Parameter> &FunctionType::parameters() const noexcept
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
    return Value && Value->kind() == ValueKind::Function;
  }
} // namespace ink::ir
