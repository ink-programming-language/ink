#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/core/config/config_manager.h"

#include <utility>

namespace ink::vm
{
  Interpreter::Interpreter() noexcept
      : Loader(*this)
  {
    CodeSegment.reserve(core::ConfigManager::getSize<core::ConfigKind::VMInitialCodeSegmentBytes>());
  }

  std::uint32_t Interpreter::loadLibrary(ExecutableFile Executable)
  {
    if (NextLibraryHandle == InvalidLibraryHandle)
    {
      return InvalidLibraryHandle;
    }
    const std::uint32_t Handle = NextLibraryHandle++;
    auto &Library = Libraries.try_emplace(Handle, LibraryDesciptor{static_cast<std::uint64_t>(CodeSegment.size()), std::move(Executable)}).first->second;
    const auto &Image = Library.ExecutableFile.getImage();
    CodeSegment.insert(CodeSegment.end(), Image.begin(), Image.end());
    Loader.loadLibrary(CodeSegment, Library.ImageBase, Library.ExecutableFile);
    return Handle;
  }

  const ExecutableFile *Interpreter::findExecutable(std::uint32_t Handle) const noexcept
  {
    const auto Position = Libraries.find(Handle);
    return Position == Libraries.end() ? nullptr : &Position->second.ExecutableFile;
  }
} // namespace ink::vm
