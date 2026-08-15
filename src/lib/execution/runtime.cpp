#include "runtime.h"

#include <cstddef>
#include <cstdint>
#include <limits>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#elif defined(__linux__)
#include <cerrno>
#include <sys/syscall.h>
#include <unistd.h>
#else
#error "Ink execution runtime output is only implemented for Windows and Linux"
#endif

namespace ink::execution
{
  namespace
  {
    extern "C" std::int32_t inkRuntimeWriteStdout(const std::uint8_t *Data, std::size_t Size)
    {
      if ((Data == nullptr && Size != 0) || Size > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))
      {
        return -1;
      }

      std::size_t WrittenTotal = 0;
#if defined(_WIN32)
      HANDLE Output = GetStdHandle(STD_OUTPUT_HANDLE);
      if (Output == nullptr || Output == INVALID_HANDLE_VALUE)
      {
        return -1;
      }
      while (WrittenTotal < Size)
      {
        const std::size_t Remaining = Size - WrittenTotal;
        const DWORD ChunkSize = Remaining > std::numeric_limits<DWORD>::max() ? std::numeric_limits<DWORD>::max() : static_cast<DWORD>(Remaining);
        DWORD Written = 0;
        if (!WriteFile(Output, Data + WrittenTotal, ChunkSize, &Written, nullptr) || Written == 0)
        {
          return -1;
        }
        WrittenTotal += Written;
      }
#else
      while (WrittenTotal < Size)
      {
        const long Written = syscall(SYS_write, STDOUT_FILENO, Data + WrittenTotal, Size - WrittenTotal);
        if (Written < 0)
        {
          if (errno == EINTR)
          {
            continue;
          }
          return -1;
        }
        if (Written == 0)
        {
          return -1;
        }
        WrittenTotal += static_cast<std::size_t>(Written);
      }
#endif
      return static_cast<std::int32_t>(WrittenTotal);
    }
  } // namespace

  bool registerRuntimeSymbols(NativeSymbolRegistry &Registry)
  {
    if (Registry.findAddress("ink_rt_write_stdout") != nullptr)
    {
      return true;
    }
    return Registry.registerSymbol("ink_rt_write_stdout", reinterpret_cast<NativeFunctionAddress>(&inkRuntimeWriteStdout));
  }
} // namespace ink::execution
