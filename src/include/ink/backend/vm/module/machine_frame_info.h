#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <functional>
#include <list>
#include <optional>
#include <ranges>

namespace ink::vm
{
  enum class MachineFrameLayoutError : std::uint8_t
  {
    InvalidAlignment,
    SizeOverflow,
  };

  // A stable abstract stack-storage request; byteOffset becomes available after explicit layout.
  class MachineStackSlot final
  {
    public:
      MachineStackSlot() noexcept = default;
      MachineStackSlot(const MachineStackSlot &) = delete;
      MachineStackSlot &operator=(const MachineStackSlot &) = delete;
      MachineStackSlot(MachineStackSlot &&) = delete;
      MachineStackSlot &operator=(MachineStackSlot &&) = delete;

      std::uint64_t byteSize() const noexcept
      {
        return ByteSize;
      }

      std::uint64_t alignment() const noexcept
      {
        return Alignment;
      }

      std::optional<std::uint64_t> byteOffset() const noexcept
      {
        return ByteOffset;
      }

    private:
      friend class MachineFrameInfo;

      std::uint64_t ByteSize = 0;
      std::uint64_t Alignment = 1;
      std::optional<std::uint64_t> ByteOffset;
  };

  // Describes stack storage independently of the calling ABI. Owners of VM addresses choose the layout base.
  class MachineFrameInfo final
  {
    public:
      MachineFrameInfo() noexcept = default;
      MachineFrameInfo(const MachineFrameInfo &) = delete;
      MachineFrameInfo &operator=(const MachineFrameInfo &) = delete;
      MachineFrameInfo(MachineFrameInfo &&) = delete;
      MachineFrameInfo &operator=(MachineFrameInfo &&) = delete;

      // A successful append preserves slot references and invalidates every previously computed offset.
      [[nodiscard]] std::expected<std::reference_wrapper<const MachineStackSlot>, MachineFrameLayoutError> createStackSlot(std::uint64_t SlotByteSize, std::uint64_t Alignment = 1);

      // On failure, all previously computed offsets and size remain unchanged. No tail padding is added.
      [[nodiscard]] std::expected<void, MachineFrameLayoutError> layout(std::uint64_t LayoutBaseOffset = 0) noexcept;

      auto stackSlots() const noexcept
      {
        return std::ranges::subrange(Slots.cbegin(), Slots.cend());
      }

      std::size_t stackSlotCount() const noexcept
      {
        return Slots.size();
      }

      std::uint64_t alignment() const noexcept
      {
        return MaximumAlignment;
      }

      std::optional<std::uint64_t> baseOffset() const noexcept
      {
        return BaseOffset;
      }

      // Includes inter-slot padding, excludes the prefix before BaseOffset and any ABI-specific tail padding.
      std::optional<std::uint64_t> byteSize() const noexcept
      {
        return ByteSize;
      }

    private:
      std::list<MachineStackSlot> Slots;
      // Cached maximum slot alignment, reported to the caller for aligning the frame's reference base.
      std::uint64_t MaximumAlignment = 1;
      // Start of the last successful layout, in the same coordinate system as each slot's byteOffset.
      std::optional<std::uint64_t> BaseOffset;
      std::optional<std::uint64_t> ByteSize;
  };
} // namespace ink::vm
