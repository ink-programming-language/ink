#pragma once

#include "ink/core/name/name.h"
#include "ink/backend/vm/module/machine_constant.h"
#include "ink/backend/vm/module/machine_function.h"
#include "ink/backend/vm/module/machine_global_variable.h"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <list>
#include <ranges>
#include <span>

namespace ink::vm
{
  enum class MachineModuleDataError : std::uint8_t
  {
    InvalidAlignment,
    InitializerTooLarge,
    SizeOverflow,
  };

  // Owns editable VM code and data independently of IR nodes or an encoded ObjectFile.
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

      MachineFunction &createFunction(core::Name FunctionName = core::NAME_None);
      [[nodiscard]] std::expected<std::reference_wrapper<MachineConstant>, MachineModuleDataError> createConstant(std::span<const std::uint8_t> Bytes, std::uint64_t Alignment = 1, core::Name ConstantName = core::NAME_None);
      [[nodiscard]] std::expected<std::reference_wrapper<MachineGlobalVariable>, MachineModuleDataError> createGlobalVariable(std::uint64_t ByteSize, std::uint64_t Alignment = 1, std::span<const std::uint8_t> InitialBytes = {}, core::Name GlobalName = core::NAME_None);

      auto functions(this auto &&Self) noexcept
      {
        return std::ranges::subrange(Self.Functions.begin(), Self.Functions.end());
      }

      auto constants(this auto &&Self) noexcept
      {
        return std::ranges::subrange(Self.Constants.begin(), Self.Constants.end());
      }

      auto globalVariables(this auto &&Self) noexcept
      {
        return std::ranges::subrange(Self.Globals.begin(), Self.Globals.end());
      }

      std::size_t functionCount() const noexcept
      {
        return Functions.size();
      }

      std::size_t constantCount() const noexcept
      {
        return Constants.size();
      }

      std::size_t globalVariableCount() const noexcept
      {
        return Globals.size();
      }

    private:
      core::Name Name;
      std::list<MachineFunction> Functions;
      std::list<MachineConstant> Constants;
      std::list<MachineGlobalVariable> Globals;
  };
} // namespace ink::vm
