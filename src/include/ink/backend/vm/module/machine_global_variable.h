#pragma once

#include "ink/core/name/name.h"
#include "ink/backend/vm/module/machine_symbol.h"

#include <cstdint>
#include <span>
#include <utility>
#include <vector>

namespace ink::vm
{
  class MachineModule;

  // MachineModule-owned global storage; writable and read-only data both zero-fill an uninitialized suffix when loaded.
  class MachineGlobalVariable final
  {
    public:
      explicit MachineGlobalVariable(MachineSymbolLinkage Linkage = MachineSymbolLinkage::Local, bool IsConstant = false) noexcept
          : Linkage(Linkage),
            IsConstant(IsConstant)
      {
      }

      MachineGlobalVariable(const MachineGlobalVariable &) = delete;
      MachineGlobalVariable &operator=(const MachineGlobalVariable &) = delete;
      MachineGlobalVariable(MachineGlobalVariable &&) = delete;
      MachineGlobalVariable &operator=(MachineGlobalVariable &&) = delete;

      core::Name name() const noexcept
      {
        return Name;
      }

      MachineSymbolLinkage linkage() const noexcept
      {
        return Linkage;
      }

      void setLinkage(MachineSymbolLinkage Value) noexcept
      {
        Linkage = Value;
      }

      bool isConstant() const noexcept
      {
        return IsConstant;
      }

      auto *parent(this auto &&Self) noexcept
      {
        return Self.Parent ? &std::forward_like<decltype(Self) &>(*Self.Parent) : nullptr;
      }

      std::uint64_t alignment() const noexcept
      {
        return Alignment;
      }

      std::uint64_t byteSize() const noexcept
      {
        return ByteSize;
      }

      std::span<const std::uint8_t> initialBytes() const noexcept
      {
        return InitialBytes;
      }

    private:
      friend class MachineModule;

      core::Name Name = core::NAME_None;
      MachineSymbolLinkage Linkage;
      bool IsConstant;
      MachineModule *Parent = nullptr;
      std::uint64_t ByteSize = 0;
      std::uint64_t Alignment = 1;
      std::vector<std::uint8_t> InitialBytes;
  };
} // namespace ink::vm
