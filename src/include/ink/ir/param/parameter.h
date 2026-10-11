#ifndef INK_IR_PARAM_PARAMETER_H
#define INK_IR_PARAM_PARAMETER_H

#include "ink/core/name.h"
#include "ink/ir/type/value.h"

#include <cstddef>

namespace ink::ir
{
  class Function;
  class Type;

  // A function-owned named formal parameter value whose type belongs to the shared signature.
  class Parameter final : public Value
  {
    public:
      Function &function() noexcept;
      const Function &function() const noexcept;
      std::size_t index() const noexcept;
      const Type *type() const noexcept;
      core::Name name() const noexcept;
      bool hasName() const noexcept;
      void setName(core::Name NewName) noexcept;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class Function;

      Parameter(Function &Parent, std::size_t Index) noexcept;

      Function &Parent;
      std::size_t Index;
      core::Name Name = core::NAME_None;
  };
} // namespace ink::ir

#endif
