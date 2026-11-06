#pragma once

#include "ink/ir/value/value.h"

#include <cstdint>

namespace ink::ir
{
  // Signature metadata borrows its type value; copies retain that identity without creating an IR value.
  // Borrowed type values must remain alive during use; destruction does not access them.
  class Parameter
  {
    public:
      enum class Kind : std::uint8_t
      {
        Positional,
        Named,
        Variadic,
      };

      constexpr Parameter(ValueRef ParameterType, Kind ParameterKind = Kind::Positional) noexcept
          : ParameterKind(ParameterKind),
            ParameterType(&ParameterType.get())
      {
      }

      constexpr const Value &type() const noexcept
      {
        return *ParameterType;
      }

      constexpr Kind kind() const noexcept
      {
        return ParameterKind;
      }

    private:
      Kind ParameterKind;
      const Value *ParameterType;
  };
} // namespace ink::ir
