#include "ink/execution/ffi/native_symbol_cache.h"

#include <utility>

namespace ink::execution
{
  NativeSymbolCache::NativeSymbolCache(Resolver Resolve)
      : Resolve(std::move(Resolve))
  {
  }

  std::size_t NativeSymbolCache::StringHash::operator()(std::string_view Name) const noexcept
  {
    return std::hash<std::string_view>{}(Name);
  }

  NativeSymbol NativeSymbolCache::find(std::string_view Name)
  {
    if (Name.empty() || Name.find('\0') != std::string_view::npos)
    {
      return nullptr;
    }
    const auto Existing = Symbols.find(Name);
    if (Existing != Symbols.end())
    {
      return Existing->second;
    }
    NativeSymbol Symbol = Resolve ? Resolve(Name) : nullptr;
    if (Symbol)
    {
      Symbols.emplace(std::string(Name), Symbol);
    }
    return Symbol;
  }

  void NativeSymbolCache::clear() noexcept
  {
    Symbols.clear();
  }
} // namespace ink::execution
