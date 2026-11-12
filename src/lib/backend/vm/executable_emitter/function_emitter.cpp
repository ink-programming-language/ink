#include "ink/backend/vm/executable_emitter/function_emitter.h"

#include "ink/backend/vm/executable_emitter/executable_emitter.h"

namespace ink::vm
{
  FunctionEmitter::FunctionEmitter(ExecutableEmitter &Emitter) noexcept
      : Emitter(Emitter)
  {
  }
} // namespace ink::vm
