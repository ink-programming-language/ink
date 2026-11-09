#pragma once

#include "ink/core/name/name.h"

#include <cstdint>
#include <span>
#include <utility>
#include <vector>

namespace ink::vm
{
  class MachineModule;

  // MachineModule-owned writable storage; the uninitialized suffix is zero-filled when loaded.
  class MachineGlobalVariable final
  {
    public:
      MachineGlobalVariable() noexcept = default;
      MachineGlobalVariable(const MachineGlobalVariable &) = delete;
      MachineGlobalVariable &operator=(const MachineGlobalVariable &) = delete;
      MachineGlobalVariable(MachineGlobalVariable &&) = delete;
      MachineGlobalVariable &operator=(MachineGlobalVariable &&) = delete;

      core::Name name() const noexcept
      {
        return Name;
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
      MachineModule *Parent = nullptr;
      std::uint64_t ByteSize = 0;
      std::uint64_t Alignment = 1;
      std::vector<std::uint8_t> InitialBytes;
  };
} // namespace ink::vm
