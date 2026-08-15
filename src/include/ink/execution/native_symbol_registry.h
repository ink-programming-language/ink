#ifndef INK_EXECUTION_NATIVE_SYMBOL_REGISTRY_H
#define INK_EXECUTION_NATIVE_SYMBOL_REGISTRY_H

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace ink::execution
{
  using NativeFunctionAddress = void (*)(void);

  struct NativeSymbol
  {
    std::string Name;
    NativeFunctionAddress Address = nullptr;
  };

  class NativeSymbolRegistry
  {
  public:
    bool registerSymbol(std::string Name, NativeFunctionAddress Address);
    bool resolveAndRegister(std::string_view Name);
    NativeFunctionAddress findAddress(std::string_view Name) const noexcept;
    std::optional<std::string> findName(NativeFunctionAddress Address) const;
    const std::vector<NativeSymbol> &symbols() const noexcept;

  private:
    std::vector<NativeSymbol> Symbols;
  };
} // namespace ink::execution

#endif
