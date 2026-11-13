#include "ink/runtime/builtin.h"

namespace ink::runtime::builtin
{
  std::int64_t identity(std::int64_t Value) noexcept
  {
    return Value;
  }
} // namespace ink::runtime::builtin
