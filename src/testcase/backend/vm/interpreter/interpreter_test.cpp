#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::vm::test
{
  static_assert(!std::is_copy_constructible_v<Interpreter> && !std::is_copy_assignable_v<Interpreter>);
  static_assert(!std::is_move_constructible_v<Interpreter> && !std::is_move_assignable_v<Interpreter>);
  static_assert(std::is_same_v<decltype(std::declval<Interpreter &>().loadLibrary(std::declval<ExecutableFile>())), std::uint32_t>);

  // An empty interpreter has no executable for invalid, unallocated, or maximum-valued handles.
  TEST(VMInterpreterTest, ReturnsNullForUnknownLibraryHandles)
  {
    const Interpreter VM;
    EXPECT_EQ(VM.findExecutable(Interpreter::InvalidLibraryHandle), nullptr);
    EXPECT_EQ(VM.findExecutable(1), nullptr);
    EXPECT_EQ(VM.findExecutable(std::numeric_limits<std::uint32_t>::max()), nullptr);
  }

  // Each load receives a distinct handle, and the stored image and metadata survive the caller's file lifetime.
  TEST(VMInterpreterTest, OwnsExecutableFilesUnderDistinctLibraryHandles)
  {
    const auto ImportName = core::NamePool::instance().intern("interpreter-owned-import");
    const auto ExportName = core::NamePool::instance().intern("interpreter-owned-export");
    const std::vector<std::uint8_t> FirstImage{0x11, 0x22, 0x33, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    Interpreter VM;
    const std::uint32_t FirstHandle = [&]()
    {
      ExecutableFile First;
      First.getImage() = FirstImage;
      First.getEntryOffset() = 1;
      First.getDynamicImportSymbols().push_back({ImportName});
      First.getDynamicImportSymbolRelocations().push_back({0, 0, -4});
      First.getImageBaseAddressRelocations().push_back({8, 2, 3});
      First.getExportSymbols().push_back({ExportName, 2});
      const std::uint32_t Handle = VM.loadLibrary(First);
      First.getImage()[0] = 0xFF;
      First.getEntryOffset() = 0;
      First.getDynamicImportSymbols().clear();
      First.getDynamicImportSymbolRelocations().clear();
      First.getImageBaseAddressRelocations().clear();
      First.getExportSymbols().clear();
      return Handle;
    }();
    EXPECT_NE(FirstHandle, Interpreter::InvalidLibraryHandle);

    ExecutableFile Second;
    Second.getImage() = {0x44, 0x55};
    const std::uint32_t SecondHandle = VM.loadLibrary(std::move(Second));
    EXPECT_NE(SecondHandle, Interpreter::InvalidLibraryHandle);
    EXPECT_NE(FirstHandle, SecondHandle);
    const auto *First = VM.findExecutable(FirstHandle);
    ASSERT_NE(First, nullptr);
    EXPECT_EQ(First->getImage(), FirstImage);
    EXPECT_EQ(First->getEntryOffset(), 1U);
    ASSERT_EQ(First->getDynamicImportSymbols().size(), 1U);
    EXPECT_EQ(First->getDynamicImportSymbols()[0].Name, ImportName);
    ASSERT_EQ(First->getDynamicImportSymbolRelocations().size(), 1U);
    EXPECT_EQ(First->getDynamicImportSymbolRelocations()[0].Addend, -4);
    ASSERT_EQ(First->getImageBaseAddressRelocations().size(), 1U);
    EXPECT_EQ(First->getImageBaseAddressRelocations()[0].TargetOffset, 2U);
    ASSERT_EQ(First->getExportSymbols().size(), 1U);
    EXPECT_EQ(First->getExportSymbols()[0].Name, ExportName);
    const auto *StoredSecond = VM.findExecutable(SecondHandle);
    ASSERT_NE(StoredSecond, nullptr);
    EXPECT_EQ(StoredSecond->getImage(), (std::vector<std::uint8_t>{0x44, 0x55}));
    EXPECT_EQ(VM.findExecutable(Interpreter::InvalidLibraryHandle), nullptr);
  }

  // Adding enough libraries to grow the map preserves existing handles and borrowed executable addresses.
  TEST(VMInterpreterTest, PreservesEarlierLibraryReferencesAcrossAdditionalLoads)
  {
    Interpreter VM;
    ExecutableFile First;
    First.getImage() = {0x42};
    const std::uint32_t FirstHandle = VM.loadLibrary(std::move(First));
    const auto *StoredFirst = VM.findExecutable(FirstHandle);
    ASSERT_NE(StoredFirst, nullptr);
    for (std::uint32_t Index = 0; Index < 256; ++Index)
    {
      ExecutableFile Next;
      Next.getImage().push_back(static_cast<std::uint8_t>(Index));
      const std::uint32_t Handle = VM.loadLibrary(std::move(Next));
      EXPECT_NE(Handle, Interpreter::InvalidLibraryHandle);
      EXPECT_NE(Handle, FirstHandle);
      const auto *Stored = VM.findExecutable(Handle);
      ASSERT_NE(Stored, nullptr);
      EXPECT_EQ(Stored->getImage()[0], static_cast<std::uint8_t>(Index));
    }
    EXPECT_EQ(VM.findExecutable(FirstHandle), StoredFirst);
    EXPECT_EQ(StoredFirst->getImage(), (std::vector<std::uint8_t>{0x42}));
  }
} // namespace ink::vm::test
