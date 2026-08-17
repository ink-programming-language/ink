#include "runtime/runtime_symbols.h"

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
#error "Ink execution runtime output is only implemented for Windows and Linux"
#endif

namespace ink::execution
{
  namespace
  {
    extern "C" std::int32_t runtimeWrite(std::int32_t Descriptor, const std::uint8_t *Data, std::size_t Size)
    {
      if ((Descriptor != 1 && Descriptor != 2) || (Data == nullptr && Size != 0))
      {
        return -1;
      }
      if (Size == 0)
      {
        return 0;
      }
      const std::size_t Requested = std::min(Size, static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()));
#if defined(_WIN32)
      const DWORD StreamId = Descriptor == 1 ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE;
      HANDLE Output = GetStdHandle(StreamId);
      if (Output == nullptr || Output == INVALID_HANDLE_VALUE)
      {
        return -1;
      }
      DWORD Written = 0;
      return WriteFile(Output, Data, static_cast<DWORD>(Requested), &Written, nullptr) ? static_cast<std::int32_t>(Written) : -1;
#else
      long Written = 0;
      do
      {
        Written = syscall(SYS_write, Descriptor, Data, Requested);
      } while (Written < 0 && errno == EINTR);
      return Written < 0 ? -1 : static_cast<std::int32_t>(Written);
#endif
    }
  } // namespace

  NativeFunctionAddress writeRuntimeAddress() noexcept
  {
    return reinterpret_cast<NativeFunctionAddress>(&runtimeWrite);
  }
} // namespace ink::execution
