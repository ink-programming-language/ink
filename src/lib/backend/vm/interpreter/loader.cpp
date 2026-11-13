#include "ink/backend/vm/interpreter/loader.h"

namespace ink::vm
{
  Loader::Loader(vm::Interpreter &Interpreter) noexcept
      : Interpreter(Interpreter)
  {
  }

  void Loader::loadLibrary(std::vector<std::uint8_t> &, std::uint64_t, ExecutableFile &)
  {
  }
} // namespace ink::vm
