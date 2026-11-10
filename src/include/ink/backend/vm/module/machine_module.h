#pragma once

#include "ink/core/name/name.h"
#include "ink/backend/vm/module/machine_function.h"
#include "ink/backend/vm/module/machine_global_variable.h"
#include "ink/backend/vm/module/machine_symbol.h"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <list>
#include <ranges>
#include <span>
#include <vector>

namespace ink::vm
{
  enum class MachineModuleDataError : std::uint8_t
  {
    InvalidAlignment,
    InitializerTooLarge,
    SizeOverflow,
  };

  // Owns editable VM code and data independently of IR nodes or an encoded executable image.
  class MachineModule final
  {
    public:
      explicit MachineModule(core::Name Name = core::NAME_None) noexcept;

      MachineModule(const MachineModule &) = delete;
      MachineModule &operator=(const MachineModule &) = delete;
      MachineModule(MachineModule &&) = delete;
      MachineModule &operator=(MachineModule &&) = delete;

      core::Name name() const noexcept
      {
        return Name;
      }

      MachineFunction &createFunction(core::Name FunctionName = core::NAME_None, MachineSymbolLinkage Linkage = MachineSymbolLinkage::Local);
      [[nodiscard]] std::expected<std::reference_wrapper<MachineGlobalVariable>, MachineModuleDataError> createGlobalVariable(std::uint64_t ByteSize, std::uint64_t Alignment = 1, std::span<const std::uint8_t> InitialBytes = {}, core::Name GlobalName = core::NAME_None, MachineSymbolLinkage Linkage = MachineSymbolLinkage::Local, bool IsConstant = false);
      void declareSymbol(core::Name SymbolName, MachineSymbolKind Kind, bool IsConstant = false);
      void declareNativeFunction(core::Name SymbolName, NativeFunctionImport Import);
      void declareNativeData(core::Name SymbolName, NativeDataImport Import);

      std::span<const MachineSymbolDeclaration> symbolDeclarations() const noexcept
      {
        return SymbolDeclarations;
      }

      std::span<const MachineNativeFunctionDeclaration> nativeFunctions() const noexcept
      {
        return NativeFunctions;
      }

      std::span<const MachineNativeDataDeclaration> nativeData() const noexcept
      {
        return NativeData;
      }

      auto functions(this auto &&Self) noexcept
      {
        return std::ranges::subrange(Self.Functions.begin(), Self.Functions.end());
      }

      auto globalVariables(this auto &&Self) noexcept
      {
        return std::ranges::subrange(Self.Globals.begin(), Self.Globals.end());
      }

      std::size_t functionCount() const noexcept
      {
        return Functions.size();
      }

      std::size_t globalVariableCount() const noexcept
      {
        return Globals.size();
      }

    private:
      core::Name Name;
      std::list<MachineFunction> Functions;
      std::list<MachineGlobalVariable> Globals;
      std::vector<MachineSymbolDeclaration> SymbolDeclarations;
      std::vector<MachineNativeFunctionDeclaration> NativeFunctions;
      std::vector<MachineNativeDataDeclaration> NativeData;
  };
} // namespace ink::vm
