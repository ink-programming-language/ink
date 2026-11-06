#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/type/type.h"

#include <cstdint>
#include <type_traits>

namespace ink::ir
{
  // Every parameter has its own address identity; copies preserve signature metadata, not identity.
  // Borrowed type values must remain alive whenever the parameter or its signature copies are used.
  // Destruction does not access borrowed type values.
  // A parameter used as a type operand must not be reassigned or relocated while borrowed.
  class Parameter : public Value
  {
    public:
      enum class Kind : std::uint8_t
      {
        Positional,
        Named,
        Variadic,
      };

      Parameter(ValueRef ParameterType, core::Name Name = core::NAME_None, Kind ParameterKind = Kind::Positional) noexcept
          : Value(ParameterType),
            Name(Name),
            ParameterKind(ParameterKind)
      {
      }

      // Explicit reference wrappers select a type operand instead of copying the wrapped parameter.
      template <typename WrappedValue> requires std::is_convertible_v<WrappedValue *, const Value *>
      Parameter(std::reference_wrapper<WrappedValue> ParameterType, core::Name Name = core::NAME_None, Kind ParameterKind = Kind::Positional) noexcept
          : Parameter(ValueRef(ParameterType.get()), Name, ParameterKind)
      {
      }

      Parameter(const Parameter &Other) noexcept
          : Value(ValueRef{Other.getType()}),
            Name(Other.Name),
            ParameterKind(Other.ParameterKind)
      {
      }

      Parameter &operator=(const Parameter &Other) noexcept
      {
        ValueType = &Other.getType();
        Name = Other.Name;
        ParameterKind = Other.ParameterKind;
        return *this;
      }

      core::Name name() const noexcept
      {
        return Name;
      }

      const Value &type() const noexcept
      {
        return getType();
      }

      Kind kind() const noexcept
      {
        return ParameterKind;
      }

    private:
      core::Name Name;
      Kind ParameterKind;
  };
} // namespace ink::ir
