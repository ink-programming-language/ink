#include "ink/backend/vm/module/machine_frame_info.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <limits>
#include <utility>

namespace ink::vm
{
  std::expected<std::reference_wrapper<const MachineStackSlot>, MachineFrameLayoutError> MachineFrameInfo::createStackSlot(std::uint64_t SlotByteSize, std::uint64_t Alignment)
  {
    if (Alignment == 0 || (Alignment & (Alignment - 1)) != 0)
    {
      return std::unexpected(MachineFrameLayoutError::InvalidAlignment);
    }
    MachineStackSlot &Slot = Slots.emplace_back();
    Slot.ByteSize = SlotByteSize;
    Slot.Alignment = Alignment;
    MaximumAlignment = std::max(MaximumAlignment, Alignment);
    if (ByteSize)
    {
      for (MachineStackSlot &Existing : Slots)
      {
        Existing.ByteOffset.reset();
      }
    }
    BaseOffset.reset();
    ByteSize.reset();
    return std::cref(Slot);
  }

  std::expected<void, MachineFrameLayoutError> MachineFrameInfo::layout(std::uint64_t LayoutBaseOffset) noexcept
  {
    std::uint64_t Cursor = LayoutBaseOffset;
    for (const MachineStackSlot &Slot : Slots)
    {
      const auto Remainder = Cursor % Slot.Alignment;
      const auto Padding = Remainder == 0 ? 0 : Slot.Alignment - Remainder;
      if (Padding > std::numeric_limits<std::uint64_t>::max() - Cursor)
      {
        return std::unexpected(MachineFrameLayoutError::SizeOverflow);
      }
      Cursor += Padding;
      if (Slot.ByteSize > std::numeric_limits<std::uint64_t>::max() - Cursor)
      {
        return std::unexpected(MachineFrameLayoutError::SizeOverflow);
      }
      Cursor += Slot.ByteSize;
    }

    const auto LaidOutByteSize = Cursor - LayoutBaseOffset;
    Cursor = LayoutBaseOffset;
    for (MachineStackSlot &Slot : Slots)
    {
      const auto Remainder = Cursor % Slot.Alignment;
      Cursor += Remainder == 0 ? 0 : Slot.Alignment - Remainder;
      Slot.ByteOffset = Cursor;
      Cursor += Slot.ByteSize;
    }
    BaseOffset = LayoutBaseOffset;
    ByteSize = LaidOutByteSize;
    return {};
  }
} // namespace ink::vm
