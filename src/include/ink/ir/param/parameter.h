#ifndef INK_IR_PARAM_PARAMETER_H
#define INK_IR_PARAM_PARAMETER_H

#include "ink/core/name/name.h"
#include "ink/ir/type/value.h"

#include <cstddef>
#include <utility>

namespace ink::ir
{
  class Function;
  class FunctionType;
  class Type;

  // A signature parameter descriptor or a function-owned formal value. Referenced types are borrowed.
  class Parameter final : public Value
  {
    public:
      Parameter(const Type *ParameterType, bool Variadic = false) noexcept;

      // Signature descriptors have no owning function.
      auto *function(this auto &&Self) noexcept
      {
        return Self.Parent ? &std::forward_like<decltype(Self) &>(*Self.Parent) : nullptr;
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
      friend class FunctionType;

      Parameter(Function &Parent, std::size_t Index) noexcept;

      Function *Parent = nullptr;
      const Type *ParameterType = nullptr;
      std::size_t Index = 0;
      bool Variadic = false;
      core::Name Name = core::NAME_None;
  };
} // namespace ink::ir

#endif
