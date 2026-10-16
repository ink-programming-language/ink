#include "ink/ir/type/structure_type/function_type.h"

#include "../../value_string_builder.h"

namespace ink::ir
{
  FunctionType::FunctionType(const target::TargetContext &Target, const Type *ReturnType, std::span<const FunctionTypeParameter> Parameters)
      : Type(ValueKind::FunctionType),
        ReturnType(ReturnType),
        Parameters(Parameters.begin(), Parameters.end()),
        PointerByteWidth(Target.pointerByteWidth())
  {
  }

  const Type *FunctionType::returnType() const noexcept
  {
    return ReturnType;
  }

  std::span<const FunctionTypeParameter> FunctionType::parameters() const noexcept
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

  std::string FunctionType::toString() const
  {
    return ValueStringBuilder::format(*this);
  }

  void FunctionType::appendToString(ValueStringBuilder &Builder) const
  {
    Builder.append("func(");
    Builder.pushValue(*ReturnType);
    Builder.pushText("): ");
    for (std::size_t Index = Parameters.size(); Index != 0; --Index)
    {
      if (Index < Parameters.size())
      {
        Builder.pushText(", ");
      }
      const auto &Parameter = Parameters[Index - 1];
      if (Parameter.Variadic)
      {
        Builder.pushText("...");
      }
      Builder.pushValue(*Parameter.ParameterType);
    }
  }

  bool FunctionType::classof(const Value *Value) noexcept
  {
    return Value && Value->kind() == ValueKind::FunctionType;
  }
} // namespace ink::ir
