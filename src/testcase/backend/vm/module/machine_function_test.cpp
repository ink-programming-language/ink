#include "ink/core/name/name_pool.h"
#include "ink/backend/vm/module/machine_function.h"

#include <gtest/gtest.h>

#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <ranges>
#include <type_traits>
#include <utility>

namespace ink::vm::test
{
  static_assert(!std::is_copy_constructible_v<MachineFunction> && !std::is_move_constructible_v<MachineFunction>);
  static_assert(!std::is_copy_constructible_v<MachineFrameInfo> && !std::is_move_constructible_v<MachineFrameInfo>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<MachineFunction &>().basicBlocks())>, MachineBasicBlock &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<const MachineFunction &>().basicBlocks())>, const MachineBasicBlock &>);
  static_assert(std::is_same_v<decltype(std::declval<MachineFunction &>().frameInfo()), MachineFrameInfo &>);
  static_assert(std::is_same_v<decltype(std::declval<const MachineFunction &>().frameInfo()), const MachineFrameInfo &>);
  static_assert(std::is_same_v<std::ranges::range_reference_t<decltype(std::declval<MachineFrameInfo &>().stackSlots())>, const MachineStackSlot &>);
  static_assert(std::is_same_v<decltype(std::declval<const MachineBasicBlock &>().parent()), const MachineFunction *>);

  // A detached VM function starts without blocks or an implicit stack-frame layout.
  TEST(VMMachineFunctionTest, StartsDetachedWithEmptyBlocksAndFrame)
  {
    MachineFunction Value;
    EXPECT_EQ(Value.parent(), nullptr);
    EXPECT_EQ(Value.name(), core::NAME_None);
    EXPECT_EQ(Value.basicBlockCount(), 0U);
    EXPECT_TRUE(std::ranges::empty(Value.basicBlocks()));
    EXPECT_EQ(Value.frameInfo().stackSlotCount(), 0U);
    EXPECT_EQ(Value.frameInfo().alignment(), 1U);
    EXPECT_FALSE(Value.frameInfo().byteSize().has_value());
    EXPECT_FALSE(Value.frameInfo().baseOffset().has_value());
  }

  // Blocks remain ordered and keep their parent and identity while later blocks are appended.
  TEST(VMMachineFunctionTest, OwnsStableNamedBasicBlocks)
  {
    const auto Name = core::NamePool::instance().intern("function");
    const auto EntryName = core::NamePool::instance().intern("entry");
    MachineFunction Value(Name);
    MachineBasicBlock &First = Value.createBasicBlock(EntryName);
    const auto FirstPosition = Value.basicBlocks().begin();
    MachineBasicBlock &Second = Value.createBasicBlock();
    for (std::size_t Index = 0; Index < 128; ++Index)
    {
      Value.createBasicBlock();
    }
    EXPECT_EQ(Value.name(), Name);
    EXPECT_EQ(Value.basicBlockCount(), 130U);
    EXPECT_EQ(&*FirstPosition, &First);
    EXPECT_EQ(&*std::next(FirstPosition), &Second);
    EXPECT_EQ(First.name(), EntryName);
    EXPECT_EQ(First.parent(), &Value);
    EXPECT_EQ(Second.parent(), &Value);
    const MachineFunction &ReadOnly = Value;
    EXPECT_EQ(ReadOnly.basicBlocks().begin()->parent(), &ReadOnly);
  }

  // Frame slots are abstract until layout, and invalid alignment requests leave existing valid layouts intact.
  TEST(VMMachineFunctionTest, RejectsInvalidSlotAlignmentsWithoutInvalidation)
  {
    MachineFrameInfo Frame;
    const auto Slot = Frame.createStackSlot(4, 4);
    ASSERT_TRUE(Slot.has_value());
    ASSERT_TRUE(Frame.layout(16).has_value());
    const std::uint64_t InvalidAlignments[] = {0, 3, std::numeric_limits<std::uint64_t>::max()};
    for (const auto Alignment : InvalidAlignments)
    {
      SCOPED_TRACE(Alignment);
      const auto Invalid = Frame.createStackSlot(1, Alignment);
      ASSERT_FALSE(Invalid.has_value());
      EXPECT_EQ(Invalid.error(), MachineFrameLayoutError::InvalidAlignment);
    }
    EXPECT_EQ(Frame.stackSlotCount(), 1U);
    EXPECT_EQ(Slot->get().byteOffset(), 16U);
    EXPECT_EQ(Frame.byteSize(), 4U);
    EXPECT_EQ(Frame.baseOffset(), 16U);
  }

  // Explicit frame layout respects each alignment, caller-provided base, and exact size without ABI tail rounding.
  TEST(VMMachineFunctionTest, LaysOutSlotsFromExplicitBaseWithPadding)
  {
    MachineFunction Value;
    MachineFrameInfo &Frame = Value.frameInfo();
    const auto First = Frame.createStackSlot(4, 4);
    const auto Second = Frame.createStackSlot(8, 8);
    const auto Last = Frame.createStackSlot(1);
    ASSERT_TRUE(First.has_value());
    ASSERT_TRUE(Second.has_value());
    ASSERT_TRUE(Last.has_value());
    EXPECT_FALSE(First->get().byteOffset().has_value());
    ASSERT_TRUE(Frame.layout(16).has_value());
    EXPECT_EQ(First->get().byteOffset(), 16U);
    EXPECT_EQ(Second->get().byteOffset(), 24U);
    EXPECT_EQ(Last->get().byteOffset(), 32U);
    EXPECT_EQ(Frame.byteSize(), 17U);
    EXPECT_EQ(Frame.alignment(), 8U);
    EXPECT_EQ(Frame.baseOffset(), 16U);
    ASSERT_TRUE(Frame.layout(5).has_value());
    EXPECT_EQ(First->get().byteOffset(), 8U);
    EXPECT_EQ(Second->get().byteOffset(), 16U);
    EXPECT_EQ(Last->get().byteOffset(), 24U);
    EXPECT_EQ(Frame.byteSize(), 20U);
    EXPECT_EQ(Frame.baseOffset(), 5U);
  }

  // Appending after layout clears all derived offsets without invalidating existing slot references.
  TEST(VMMachineFunctionTest, InvalidatesLayoutOnSuccessfulSlotAppend)
  {
    MachineFrameInfo Frame;
    const auto First = Frame.createStackSlot(1);
    ASSERT_TRUE(First.has_value());
    const MachineStackSlot *FirstAddress = &First->get();
    const auto FirstPosition = Frame.stackSlots().begin();
    ASSERT_TRUE(Frame.layout(16).has_value());
    const auto Second = Frame.createStackSlot(8, 8);
    ASSERT_TRUE(Second.has_value());
    EXPECT_EQ(&*FirstPosition, FirstAddress);
    EXPECT_FALSE(First->get().byteOffset().has_value());
    EXPECT_FALSE(Second->get().byteOffset().has_value());
    EXPECT_FALSE(Frame.byteSize().has_value());
    EXPECT_FALSE(Frame.baseOffset().has_value());
    ASSERT_TRUE(Frame.layout(16).has_value());
    EXPECT_EQ(First->get().byteOffset(), 16U);
    EXPECT_EQ(Second->get().byteOffset(), 24U);
    EXPECT_EQ(Frame.byteSize(), 16U);
  }

  // Both alignment padding and slot-size arithmetic reject overflow before changing any previously valid layout.
  TEST(VMMachineFunctionTest, PreservesPreviousLayoutWhenArithmeticOverflows)
  {
    MachineFrameInfo Padding;
    const auto Aligned = Padding.createStackSlot(0, 8);
    ASSERT_TRUE(Aligned.has_value());
    ASSERT_TRUE(Padding.layout(0).has_value());
    const auto BadPadding = Padding.layout(std::numeric_limits<std::uint64_t>::max());
    ASSERT_FALSE(BadPadding.has_value());
    EXPECT_EQ(BadPadding.error(), MachineFrameLayoutError::SizeOverflow);
    EXPECT_EQ(Aligned->get().byteOffset(), 0U);
    EXPECT_EQ(Padding.byteSize(), 0U);
    EXPECT_EQ(Padding.baseOffset(), 0U);
    MachineFrameInfo Size;
    const auto Huge = Size.createStackSlot(std::numeric_limits<std::uint64_t>::max());
    ASSERT_TRUE(Huge.has_value());
    ASSERT_TRUE(Size.layout(0).has_value());
    const auto BadSize = Size.layout(1);
    ASSERT_FALSE(BadSize.has_value());
    EXPECT_EQ(BadSize.error(), MachineFrameLayoutError::SizeOverflow);
    EXPECT_EQ(Huge->get().byteOffset(), 0U);
    EXPECT_EQ(Size.byteSize(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_EQ(Size.baseOffset(), 0U);
  }

  // Empty and zero-sized frames support explicit bases without adding an implicit call header or trailing alignment.
  TEST(VMMachineFunctionTest, SupportsEmptyAndZeroSizedFrames)
  {
    MachineFrameInfo Frame;
    ASSERT_TRUE(Frame.layout(std::numeric_limits<std::uint64_t>::max()).has_value());
    EXPECT_EQ(Frame.byteSize(), 0U);
    EXPECT_EQ(Frame.baseOffset(), std::numeric_limits<std::uint64_t>::max());
    const auto Empty = Frame.createStackSlot(0, 8);
    ASSERT_TRUE(Empty.has_value());
    ASSERT_TRUE(Frame.layout(1).has_value());
    EXPECT_EQ(Empty->get().byteOffset(), 8U);
    EXPECT_EQ(Frame.byteSize(), 7U);
    EXPECT_EQ(Frame.alignment(), 8U);
  }

  // Each VM function owns a distinct frame description with independent slots and layouts.
  TEST(VMMachineFunctionTest, KeepsFunctionFramesIndependent)
  {
    MachineFunction First;
    MachineFunction Second;
    ASSERT_TRUE(First.frameInfo().createStackSlot(4, 4).has_value());
    ASSERT_TRUE(First.frameInfo().layout(16).has_value());
    EXPECT_EQ(Second.frameInfo().stackSlotCount(), 0U);
    EXPECT_FALSE(Second.frameInfo().byteSize().has_value());
    ASSERT_TRUE(Second.frameInfo().createStackSlot(8, 8).has_value());
    ASSERT_TRUE(Second.frameInfo().layout(32).has_value());
    EXPECT_EQ(First.frameInfo().byteSize(), 4U);
    EXPECT_EQ(First.frameInfo().baseOffset(), 16U);
    EXPECT_EQ(Second.frameInfo().byteSize(), 8U);
    EXPECT_EQ(Second.frameInfo().baseOffset(), 32U);
  }
} // namespace ink::vm::test
