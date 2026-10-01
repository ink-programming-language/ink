#include "ink/execution/ffi/native_symbol.h"

#include <cstdlib>
#include <string>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <tlhelp32.h>
#elif defined(__linux__)
#include <dlfcn.h>
#endif

namespace ink::execution
{
#if defined(_WIN32)
  namespace
  {
    HMODULE currentRuntimeModule() noexcept
    {
      HMODULE Module = nullptr;
      if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, reinterpret_cast<LPCWSTR>(&std::malloc), &Module))
      {
        return nullptr;
      }
      return Module;
    }
  } // namespace
#endif

  NativeSymbol findNativeSymbol(std::string_view Name)
  {
    if (Name.empty() || Name.find('\0') != std::string_view::npos)
    {
      return nullptr;
    }
    const std::string SymbolName(Name);
#if defined(_WIN32)
    if (HMODULE Executable = GetModuleHandleW(nullptr))
    {
      if (FARPROC Symbol = GetProcAddress(Executable, SymbolName.c_str()))
      {
        return reinterpret_cast<NativeSymbol>(Symbol);
      }
    }
    if (HMODULE Runtime = currentRuntimeModule())
    {
      if (FARPROC Symbol = GetProcAddress(Runtime, SymbolName.c_str()))
      {
        return reinterpret_cast<NativeSymbol>(Symbol);
      }
    }
    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());
    if (Snapshot == INVALID_HANDLE_VALUE)
    {
      return nullptr;
    }
    MODULEENTRY32W Entry{};
    Entry.dwSize = sizeof(Entry);
    FARPROC Symbol = nullptr;
    if (Module32FirstW(Snapshot, &Entry))
    {
      do
      {
        Symbol = GetProcAddress(Entry.hModule, SymbolName.c_str());
      } while (!Symbol && Module32NextW(Snapshot, &Entry));
    }
    CloseHandle(Snapshot);
    return reinterpret_cast<NativeSymbol>(Symbol);
#elif defined(__linux__)
    dlerror();
    void *Symbol = dlsym(RTLD_DEFAULT, SymbolName.c_str());
    return dlerror() == nullptr ? reinterpret_cast<NativeSymbol>(Symbol) : nullptr;
#else
    return nullptr;
#endif
  }
} // namespace ink::execution
