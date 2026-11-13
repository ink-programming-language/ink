#include "ink/backend/vm/runtime_stub/builtin_stub.h"
#include "ink/runtime/builtin.h"

#include <cstdint>
#include <cstring>

namespace ink::vm
{
  void identityStub(Frame *CallFrame) noexcept
  {
    std::int64_t Value;
    std::memcpy(&Value, CallFrame->SP - 8, sizeof(Value));
    const std::int64_t Result = ::identity(Value);
    std::memcpy(CallFrame->SP - 16, &Result, sizeof(Result));
  }
} // namespace ink::vm
