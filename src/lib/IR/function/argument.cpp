#include "ink/ir/function/argument.h"

#include "ink/ir/function/function.h"

namespace ink::ir
{
  Parameter::Kind Argument::kind() const noexcept
  {
    return Parent->signature().parameters()[Index].kind();
  }
} // namespace ink::ir
