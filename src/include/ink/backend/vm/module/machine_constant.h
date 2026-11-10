#pragma once

#include "ink/core/name/name.h"

#include <cstdint>
#include <span>
#include <utility>
#include <vector>

namespace ink::vm
{
  class MachineModule;

  // MachineModule-owned read-only data. The payload has no image offset before executable emission.
  class MachineConstant final
  {
    public:
      MachineConstant() noexcept = default;
      MachineConstant(const MachineConstant &) = delete;
      MachineConstant &operator=(const MachineConstant &) = delete;
      MachineConstant(MachineConstant &&) = delete;
      MachineConstant &operator=(MachineConstant &&) = delete;

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
        return static_cast<std::uint64_t>(Bytes.size());
      }

      std::span<const std::uint8_t> bytes() const noexcept
      {
        return Bytes;
      }

    private:
      friend class MachineModule;

      core::Name Name = core::NAME_None;
      MachineModule *Parent = nullptr;
      std::uint64_t Alignment = 1;
      std::vector<std::uint8_t> Bytes;
  };
} // namespace ink::vm
