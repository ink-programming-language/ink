#include "ink/semantic/model/module/module.h"

#include "ink/semantic/model/context.h"

namespace ink::semantic
{
  const BuiltinType &Module::type() const noexcept
  {
    return context().getModuleType();
  }
} // namespace ink::semantic
