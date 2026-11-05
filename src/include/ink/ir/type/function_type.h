#pragma once

#include "ink/ir/type/parameter.h"

#include <cstddef>
#include <span>
#include <vector>

namespace ink::ir
{
  // Owns copied parameter metadata and interned name handles; borrowed return and parameter types must outlive this type.
  // Signature identity includes parameter names and kinds, their order, and the identity of each borrowed type.
  class FunctionType final : public Type
  {
    public:
      struct Key
      {
          const Type *ReturnType;
          std::span<const Parameter> Parameters;

          std::size_t hash() const noexcept;
          bool operator==(const Key &Other) const noexcept;
      };

      std::size_t hash() const noexcept;
      bool operator==(const FunctionType &Other) const noexcept;
      bool operator==(const Key &Other) const noexcept;

      static constexpr TypeKind Kind = TypeKind::Function;

      explicit FunctionType(const Type &ReturnType, std::span<const Parameter> Parameters = {});
      FunctionType(const Type &&, std::span<const Parameter> = {}) = delete;

      static constexpr bool classof(const Type *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      const Type &returnType() const noexcept
      {
        return ReturnType;
      }

      std::span<const Parameter> parameters() const noexcept
      {
        return Parameters;
      }

    private:
      const Type &ReturnType;
      std::vector<Parameter> Parameters;
  };
} // namespace ink::ir
