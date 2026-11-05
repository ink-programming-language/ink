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
          const Value *ReturnType;
          std::span<const Parameter> Parameters;

          std::size_t hash() const noexcept;
          bool operator==(const Key &Other) const noexcept;
      };

      std::size_t hash() const noexcept;
      bool operator==(const FunctionType &Other) const noexcept;
      bool operator==(const Key &Other) const noexcept;

      static constexpr TypeKind Kind = TypeKind::Function;

      explicit FunctionType(ValueRef ReturnType, std::span<const Parameter> Parameters = {});

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

    private:
      const Value &ReturnType;
      std::vector<Parameter> Parameters;
  };
} // namespace ink::ir
