#include "runtime_symbols.h"

#include <algorithm>
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
#error "Ink execution runtime input is only implemented for Windows and Linux"
#endif

namespace ink::execution
{
  namespace
  {
    extern "C" std::int32_t runtimeRead(std::int32_t Descriptor, std::uint8_t *Data, std::size_t Size)
    {
      if (Descriptor != 0 || (Data == nullptr && Size != 0))
      {
        return -1;
      }
      if (Size == 0)
      {
        return 0;
      }
      const std::size_t Requested = std::min(Size, static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()));
#if defined(_WIN32)
      HANDLE Input = GetStdHandle(STD_INPUT_HANDLE);
      if (Input == nullptr || Input == INVALID_HANDLE_VALUE)
      {
        return -1;
      }
      DWORD Read = 0;
      if (!ReadFile(Input, Data, static_cast<DWORD>(Requested), &Read, nullptr))
      {
        const DWORD Error = GetLastError();
        return Error == ERROR_BROKEN_PIPE || Error == ERROR_HANDLE_EOF ? 0 : -1;
      }
      return static_cast<std::int32_t>(Read);
#else
      long Read = 0;
      do
      {
        Read = syscall(SYS_read, STDIN_FILENO, Data, Requested);
      } while (Read < 0 && errno == EINTR);
      return Read < 0 ? -1 : static_cast<std::int32_t>(Read);
#endif
    }
  } // namespace

  NativeFunctionAddress readRuntimeAddress() noexcept
  {
    return reinterpret_cast<NativeFunctionAddress>(&runtimeRead);
  }
} // namespace ink::execution
