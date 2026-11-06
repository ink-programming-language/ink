#pragma once

#include "ink/ir/type/call_abi.h"
#include "ink/ir/type/parameter.h"

#include <cstddef>
#include <span>
#include <vector>

namespace ink::ir
{
  // Owns copied parameter metadata and interned name handles; borrowed return and parameter types must outlive this type.
  // Signature identity includes parameter names and kinds, their order, borrowed type identities, and the call ABI.
  class FunctionType final : public Type
  {
    public:
      struct Key
      {
          const Value *ReturnType = nullptr;
          std::span<const Parameter> Parameters = {};
          CallABI ABI = {};

          std::size_t hash() const noexcept;
          bool operator==(const Key &Other) const noexcept;
      };

      std::size_t hash() const noexcept;
      bool operator==(const FunctionType &Other) const noexcept;
      bool operator==(const Key &Other) const noexcept;

      static constexpr TypeKind Kind = TypeKind::Function;

      explicit FunctionType(ValueRef ReturnType, std::span<const Parameter> Parameters = {}, CallABI ABI = {});

      static constexpr bool classof(const Value *Value) noexcept
      {
        return Type::classof(Value) && static_cast<const Type *>(Value)->getKind() == Kind;
      }

      const Value &returnType() const noexcept
      {
        return ReturnType;
      }

      std::span<const Parameter> parameters() const noexcept
      {
        return Parameters;
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
  };
} // namespace ink::ir
