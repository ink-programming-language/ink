#include "ink/backend/vm/interpreter/interpreter.h"

#include <utility>

namespace ink::vm
{
  Interpreter::Interpreter() noexcept
      : Loader(*this)
  {
  }

  std::uint32_t Interpreter::loadLibrary(ExecutableFile Executable)
  {
    if (NextLibraryHandle == InvalidLibraryHandle)
    {
      return InvalidLibraryHandle;
    }
    const std::uint32_t Handle = NextLibraryHandle++;
    auto &Library = Libraries.try_emplace(Handle, std::move(Executable)).first->second;
    Loader.loadLibrary(Library);
    return Handle;
  }

  const ExecutableFile *Interpreter::findExecutable(std::uint32_t Handle) const noexcept
  {
    const auto Position = Libraries.find(Handle);
    return Position == Libraries.end() ? nullptr : &Position->second;
  }
} // namespace ink::vm
