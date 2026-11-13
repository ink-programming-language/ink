#include "ink/backend/vm/interpreter/loader.h"
#include "ink/backend/vm/executable/executable_file.h"

namespace ink::vm
{
  Loader::Loader(vm::Interpreter &Interpreter) noexcept
      : Interpreter(Interpreter)
  {
  }

  bool Loader::loadLibrary(std::vector<std::uint8_t> &CodeSegment, std::uint64_t ImageBase, ExecutableFile &Executable)
  {
    return true;
  }
} // namespace ink::vm
