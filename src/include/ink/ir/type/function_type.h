#pragma once

#include "ink/ir/type/call_abi.h"
#include "ink/ir/function/parameter.h"
#include "ink/ir/type/type.h"

#include <cstddef>
#include <span>
#include <vector>

namespace ink::ir
{
  // Owns copied fixed-parameter metadata; borrowed return and parameter types must remain alive during use.
  // Destruction does not access borrowed type values.
  // Signature identity includes parameter kinds, their order, borrowed type identities, the call ABI and the C ellipsis flag.
  class FunctionType final : public Type
  {
    public:
      struct Key
      {
          const Value *ReturnType = nullptr;
          std::span<const Parameter> Parameters = {};
          CallABI ABI = {};
          bool IsVariadic = false;

          std::size_t hash() const noexcept;
          bool operator==(const Key &Other) const noexcept;
      };

      std::size_t hash() const noexcept;
      bool operator==(const FunctionType &Other) const noexcept;
      bool operator==(const Key &Other) const noexcept;

      static constexpr TypeKind Kind = TypeKind::Function;

      explicit FunctionType(ValueRef ReturnType, std::span<const Parameter> Parameters = {}, CallABI ABI = {}, bool IsVariadic = false);

      static constexpr bool classof(const Value *Value) noexcept
      {
        return Type::classof(Value) && static_cast<const Type *>(Value)->getKind() == Kind;
      }

      const Value &returnType() const noexcept
      {
        return ReturnType;
      }

      // Contains only typed fixed parameters; the C ellipsis is recorded separately.
      std::span<const Parameter> parameters() const noexcept
      {
        return Parameters;
      }

      // The C ellipsis has no type and does not add a fixed parameter or a function Argument.
      bool isVariadic() const noexcept
      {
        return IsVariadic;
      }

      CallABI callABI() const noexcept
      {
        return ABI;
      }

      ABIKind abi() const noexcept
      {
        return ABI.ABI;
      }

      CallingConvention callingConvention() const noexcept
      {
        return ABI.Convention;
      }

    private:
      const Value &ReturnType;
      std::vector<Parameter> Parameters = {};
      CallABI ABI = {};
      bool IsVariadic = false;
  };
} // namespace ink::ir
