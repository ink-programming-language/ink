#ifndef INK_EXECUTION_MODULE_ID_H
#define INK_EXECUTION_MODULE_ID_H

#include <cstddef>
#include <limits>

namespace ink::execution
{
  class ModuleId
  {
    public:
      constexpr ModuleId() noexcept = default;

      explicit constexpr ModuleId(std::size_t Value) noexcept
          : Value(Value)
      {
      }

      constexpr bool valid() const noexcept
      {
        return Value != InvalidValue;
      }

      constexpr std::size_t value() const noexcept
      {
        return Value;
      }

      friend constexpr bool operator==(ModuleId Left, ModuleId Right) noexcept
      {
        return Left.Value == Right.Value;
      }

      friend constexpr bool operator!=(ModuleId Left, ModuleId Right) noexcept
      {
        return !(Left == Right);
      }

    private:
      static constexpr std::size_t InvalidValue = std::numeric_limits<std::size_t>::max();
      std::size_t Value = InvalidValue;
  };
} // namespace ink::execution

#endif
