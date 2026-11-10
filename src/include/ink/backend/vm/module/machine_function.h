#pragma once

#include "ink/core/name/name.h"
#include "ink/backend/vm/module/machine_basic_block.h"
#include "ink/backend/vm/module/machine_frame_info.h"
#include "ink/backend/vm/module/machine_symbol.h"

#include <cstddef>
#include <list>
#include <ranges>
#include <utility>

namespace ink::vm
{
  class MachineModule;

  // Owns blocks and frame descriptions. Appending preserves block addresses and existing iterators.
  class MachineFunction final
  {
    public:
      explicit MachineFunction(core::Name Name = core::NAME_None, MachineSymbolLinkage Linkage = MachineSymbolLinkage::Local) noexcept;

      MachineFunction(const MachineFunction &) = delete;
      MachineFunction &operator=(const MachineFunction &) = delete;
      MachineFunction(MachineFunction &&) = delete;
      MachineFunction &operator=(MachineFunction &&) = delete;

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

      auto *parent(this auto &&Self) noexcept
      {
        return Self.Parent ? &std::forward_like<decltype(Self) &>(*Self.Parent) : nullptr;
      }

      MachineBasicBlock &createBasicBlock(core::Name BlockName = core::NAME_None);

      auto basicBlocks(this auto &&Self) noexcept
      {
        return std::ranges::subrange(Self.Blocks.begin(), Self.Blocks.end());
      }

      std::size_t basicBlockCount() const noexcept
      {
        return Blocks.size();
      }

      auto &frameInfo(this auto &&Self) noexcept
      {
        return Self.Frame;
      }

    private:
      friend class MachineModule;

      core::Name Name;
      MachineSymbolLinkage Linkage;
      MachineModule *Parent = nullptr;
      MachineFrameInfo Frame;
      std::list<MachineBasicBlock> Blocks;
  };
} // namespace ink::vm
