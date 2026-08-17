#include "ink/execution/runtime/native_symbol_registry.h"

#include <string>
#include <utility>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#elif defined(__linux__)
#include <dlfcn.h>
#else
#error "Ink execution native symbol lookup is only implemented for Windows and Linux"
#endif

namespace ink::execution
{
  namespace
  {
    NativeFunctionAddress findPlatformSymbol(std::string_view Name)
    {
      if (Name.empty() || Name.find('\0') != std::string_view::npos)
      {
        return nullptr;
      }

      const std::string NullTerminatedName(Name);
#if defined(_WIN32)
      const HMODULE Modules[] = {
          GetModuleHandleW(nullptr),
          GetModuleHandleW(L"kernel32.dll"),
          GetModuleHandleW(L"ucrtbase.dll"),
          GetModuleHandleW(L"msvcrt.dll"),
      };
      for (HMODULE Module : Modules)
      {
        if (Module == nullptr)
        {
          continue;
        }
        if (FARPROC Address = GetProcAddress(Module, NullTerminatedName.c_str()))
        {
          return reinterpret_cast<NativeFunctionAddress>(Address);
        }
      }
      return nullptr;
#else
      return reinterpret_cast<NativeFunctionAddress>(dlsym(RTLD_DEFAULT, NullTerminatedName.c_str()));
#endif
    }
  } // namespace

  bool NativeSymbolRegistry::registerSymbol(std::string Name, NativeFunctionAddress Address)
  {
    if (Name.empty() || Address == nullptr)
    {
      return false;
    }
    for (const NativeSymbol &Symbol : Symbols)
    {
      if (Symbol.Name == Name)
      {
        return Symbol.Address == Address;
      }
    }
    Symbols.push_back({std::move(Name), Address});
    return true;
  }

  bool NativeSymbolRegistry::resolveAndRegister(std::string_view Name)
  {
    if (findAddress(Name) != nullptr)
    {
      return true;
    }
    NativeFunctionAddress Address = findPlatformSymbol(Name);
    return Address != nullptr && registerSymbol(std::string(Name), Address);
  }

  NativeFunctionAddress NativeSymbolRegistry::findAddress(std::string_view Name) const noexcept
  {
    for (const NativeSymbol &Symbol : Symbols)
    {
      if (Symbol.Name == Name)
      {
        return Symbol.Address;
      }
    }
    return nullptr;
  }

  std::optional<std::string> NativeSymbolRegistry::findName(NativeFunctionAddress Address) const
  {
    for (const NativeSymbol &Symbol : Symbols)
    {
      if (Symbol.Address == Address)
      {
        return Symbol.Name;
      }
    }
    return std::nullopt;
  }

  const std::vector<NativeSymbol> &NativeSymbolRegistry::symbols() const noexcept
  {
    return Symbols;
  }
} // namespace ink::execution
