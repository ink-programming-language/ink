#include "ink/backend/vm/interpreter/loader.h"

namespace ink::vm
{
  Loader::Loader(vm::Interpreter &Interpreter) noexcept
      : Interpreter(Interpreter)
  {
  }

  void Loader::loadLibrary(ExecutableFile &)
  {
  }
} // namespace ink::vm
