#ifndef INK_EXECUTION_FFI_NATIVE_SYMBOL_CACHE_H
#define INK_EXECUTION_FFI_NATIVE_SYMBOL_CACHE_H

#include "ink/execution/ffi/native_symbol.h"

#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace ink::execution
{
  // Single-threaded, owning-name cache of the first successful binding for each
  // exact symbol name. Modules are not retained: callers must keep them loaded
  // while their bindings are cached or returned addresses are still in use.
  class NativeSymbolCache final
  {
    public:
      using Resolver = std::function<NativeSymbol(std::string_view)>;

      explicit NativeSymbolCache(Resolver Resolve = findNativeSymbol);

      // Invalid names, unresolved symbols and an empty resolver return null.
      // Failures are not cached, allowing symbols from later loads to be found.
      NativeSymbol find(std::string_view Name);
      // Discards bindings so subsequent lookups use the current module set.
      // Does not unload modules or invalidate addresses already held by callers.
      void clear() noexcept;

    private:
      struct StringHash
      {
          using is_transparent = void;

          std::size_t operator()(std::string_view Name) const noexcept;
      };

      Resolver Resolve;
      std::unordered_map<std::string, NativeSymbol, StringHash, std::equal_to<>> Symbols;
  };
} // namespace ink::execution

#endif
