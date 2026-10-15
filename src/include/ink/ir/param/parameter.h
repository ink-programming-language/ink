#ifndef INK_IR_PARAM_PARAMETER_H
#define INK_IR_PARAM_PARAMETER_H

#include "ink/core/name/name.h"
#include "ink/ir/type/value.h"

#include <cstddef>
#include <utility>

namespace ink::ir
{
  class Function;
  class Type;

  // A function-owned formal value whose type and variadic status come from the function signature.
  class Parameter final : public Value
  {
    public:
      auto *function(this auto &&Self) noexcept
      {
        return &std::forward_like<decltype(Self) &>(*Self.Parent);
      }

      std::size_t index() const noexcept;
      const Type *type() const noexcept override;
      bool isVariadic() const noexcept;
      core::Name name() const noexcept;
      bool hasName() const noexcept;
      void setName(core::Name NewName) noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class Function;

      Parameter(Function &Parent, std::size_t Index) noexcept;

      Function *Parent = nullptr;
      std::size_t Index = 0;
      core::Name Name = core::NAME_None;
  };
} // namespace ink::ir

#endif
