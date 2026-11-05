#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/type/type.h"

#include <cstdint>

namespace ink::ir
{
  // Copies an interned name handle; the borrowed parameter type must outlive this metadata and its copies.
  class Parameter
  {
    public:
      enum class Kind : std::uint8_t
      {
        Positional,
        Named,
        Variadic,
      };

      Parameter(const Type &ParameterType, core::Name Name = core::NAME_None, Kind ParameterKind = Kind::Positional) noexcept
          : ParameterType(&ParameterType),
            Name(Name),
            ParameterKind(ParameterKind)
      {
      }

      Parameter(const Type &&, core::Name = core::NAME_None, Kind = Kind::Positional) = delete;

      core::Name name() const noexcept
      {
        return Name;
      }

      const Type &type() const noexcept
      {
        return *ParameterType;
      }

      Kind kind() const noexcept
      {
        return ParameterKind;
      }

    private:
      const Type *ParameterType;
      core::Name Name;
      Kind ParameterKind;
  };
} // namespace ink::ir
