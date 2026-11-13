#include "ink/backend/vm/vm_runtime/runtime.h"
#include "ink/runtime/builtin.h"
#include "ink/core/name/name_pool.h"

namespace ink::vm
{
  const std::array<NativeSymbolAddress, 1> NativeSymbolAddresses = {{
      {core::NamePool::instance().intern("identity"), reinterpret_cast<void *>(&::identity)},
  }};
} // namespace ink::vm
