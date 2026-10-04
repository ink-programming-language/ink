#include <stddef.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#else
#include <errno.h>
#include <unistd.h>
#endif

// The compiler supplies the complete diagnostic. The standalone AOT runtime
// depends only on the operating system, not Ink, the VM, a logger or C++ objects.
extern "C"
{
  [[noreturn]] void ink_aot_panic(const char *Message, size_t Length)
  {
#if defined(_WIN32)
    const HANDLE Error = GetStdHandle(STD_ERROR_HANDLE);
    while (Length != 0)
    {
      const DWORD Count = Length > MAXDWORD ? MAXDWORD : static_cast<DWORD>(Length);
      DWORD Written = 0;
      if (!WriteFile(Error, Message, Count, &Written, nullptr) || Written == 0)
      {
        break;
      }
      Message += Written;
      Length -= Written;
    }
    ExitProcess(1);
#else
    while (Length != 0)
    {
      const ssize_t Written = write(STDERR_FILENO, Message, Length);
      if (Written < 0 && errno == EINTR)
      {
        continue;
      }
      if (Written <= 0)
      {
        break;
      }
      Message += Written;
      Length -= static_cast<size_t>(Written);
    }
    _exit(1);
#endif
  }
}
