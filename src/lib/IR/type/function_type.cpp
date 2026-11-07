#include "ink/ir/type/function_type.h"

#include <functional>

namespace ink::ir
{
  namespace
  {
    FunctionType::Key functionKey(const FunctionType &Value) noexcept
    {
      return {&Value.returnType(), Value.parameters(), Value.callABI(), Value.isVariadic()};
    }

    std::size_t combineHash(std::size_t First, std::size_t Second) noexcept
    {
      return First ^ (Second + std::size_t{0x9e3779b9} + (First << 6) + (First >> 2));
    }
  } // namespace

  FunctionType::FunctionType(ValueRef ReturnType, std::span<const Parameter> Parameters, CallABI ABI, bool IsVariadic)
      : Type(Kind),
        ReturnType(ReturnType.get()),
        ABI(ABI),
        IsVariadic(IsVariadic)
  {
    if (!Parameters.empty())
    {
      this->Parameters.assign(Parameters.begin(), Parameters.end());
    }
  }

  std::size_t FunctionType::hash() const noexcept
  {
    return functionKey(*this).hash();
  }

  std::size_t FunctionType::Key::hash() const noexcept
  {
    std::size_t Hash = combineHash(std::hash<const Value *>{}(ReturnType), std::hash<std::size_t>{}(Parameters.size()));
    Hash = combineHash(Hash, std::hash<ABIKind>{}(ABI.ABI));
    Hash = combineHash(Hash, std::hash<CallingConvention>{}(ABI.Convention));
    Hash = combineHash(Hash, std::hash<bool>{}(IsVariadic));
    for (const auto &ParameterValue : Parameters)
    {
      Hash = combineHash(Hash, std::hash<const Value *>{}(&ParameterValue.type()));
      Hash = combineHash(Hash, std::hash<Parameter::Kind>{}(ParameterValue.kind()));
    }
    return Hash;
  }

  bool FunctionType::operator==(const FunctionType &Other) const noexcept
  {
    return functionKey(*this) == functionKey(Other);
  }

  bool FunctionType::operator==(const Key &Other) const noexcept
  {
    return functionKey(*this) == Other;
  }

  bool FunctionType::Key::operator==(const Key &Other) const noexcept
  {
    if (ReturnType != Other.ReturnType || Parameters.size() != Other.Parameters.size() || ABI != Other.ABI || IsVariadic != Other.IsVariadic)
    {
      return false;
    }
    for (std::size_t Index = 0; Index < Parameters.size(); ++Index)
    {
      if (&Parameters[Index].type() != &Other.Parameters[Index].type() || Parameters[Index].kind() != Other.Parameters[Index].kind())
      {
        return false;
      }
    }
    return true;
  }
} // namespace ink::ir
