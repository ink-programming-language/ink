#include "ink/backend/vm/runtime_stub/builtin_stub.h"
#include "ink/runtime/builtin.h"

#include <cstdint>

namespace ink::vm
{
  void identityStub(Frame *CallFrame) noexcept
  {
    const auto Value = CallFrame->getArgument<0, std::int64_t>();
    CallFrame->setReturnValue<std::int64_t, std::int64_t>(::identity(Value));
  }
} // namespace ink::vm
