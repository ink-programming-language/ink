#include "ink/backend/vm/vm_runtime/runtime.h"
#include "ink/backend/vm/runtime_stub/builtin_stub.h"
#include "ink/backend/vm/runtime_stub/windows/kernel32_stub.h"
#include "ink/core/name/name_pool.h"

namespace ink::vm
{
  const std::array<NativeSymbolAddress, INK_PLATFORM_WINDOWS ? 3 : 1> NativeSymbolAddresses = {{
      {core::NamePool::instance().intern("identity"), reinterpret_cast<void *>(&identityStub)},
#if INK_PLATFORM_WINDOWS
      {core::NamePool::instance().intern("GetStdHandle"), reinterpret_cast<void *>(&getStdHandleStub)},
      {core::NamePool::instance().intern("WriteFile"), reinterpret_cast<void *>(&writeFileStub)},
#endif
  }};
} // namespace ink::vm
