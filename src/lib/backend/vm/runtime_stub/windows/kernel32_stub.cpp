#include "ink/backend/vm/runtime_stub/windows/kernel32_stub.h"

#if INK_PLATFORM_WINDOWS
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>

namespace ink::vm
{
  void getStdHandleStub(Frame *CallFrame) noexcept
  {
    const auto StandardHandle = CallFrame->getArgument<0, DWORD>();
    CallFrame->setReturnValue<HANDLE, DWORD>(::GetStdHandle(StandardHandle));
  }

  void writeFileStub(Frame *CallFrame) noexcept
  {
    const auto File = CallFrame->getArgument<0, HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED>();
    const auto Buffer = CallFrame->getArgument<1, HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED>();
    const auto BytesToWrite = CallFrame->getArgument<2, HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED>();
    const auto BytesWritten = CallFrame->getArgument<3, HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED>();
    const auto Overlapped = CallFrame->getArgument<4, HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED>();
    CallFrame->setReturnValue<BOOL, HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED>(::WriteFile(File, Buffer, BytesToWrite, BytesWritten, Overlapped));
  }
} // namespace ink::vm
#endif
