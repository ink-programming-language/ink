#include "../../../core/support/environment_test_support.h"

#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/backend/vm/runtime_stub/builtin_stub.h"
#include "ink/backend/vm/vm_runtime/runtime.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <limits>
#include <span>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ink::vm::test
{
  namespace
  {
    std::uint64_t hostAddress(const void *Pointer) noexcept
    {
      return static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(Pointer));
    }

    std::uint64_t codeAddress(const Interpreter &VM, std::size_t Offset) noexcept
    {
      return hostAddress(VM.getCodeSegment().data() + Offset);
    }

    std::uint64_t readBoundAddress(const Interpreter &VM, std::size_t Offset) noexcept
    {
      std::uint64_t Address = 0;
      for (std::size_t Index = 0; Index < 8; ++Index)
      {
        Address |= static_cast<std::uint64_t>(VM.getCodeSegment()[Offset + Index]) << (Index * 8);
      }
      return Address;
    }

    void storeExpectedAddress(std::vector<std::uint8_t> &Bytes, std::size_t Offset, std::uint64_t Address)
    {
      for (std::size_t Index = 0; Index < 8; ++Index)
      {
        Bytes[Offset + Index] = static_cast<std::uint8_t>(Address >> (Index * 8));
      }
    }

    void expectCodeBytes(const Interpreter &VM, std::span<const std::uint8_t> Expected)
    {
      EXPECT_TRUE(std::ranges::equal(VM.getCodeSegment(), Expected));
    }
  }

  static_assert(!std::is_copy_constructible_v<Interpreter> && !std::is_copy_assignable_v<Interpreter>);
  static_assert(!std::is_move_constructible_v<Interpreter> && !std::is_move_assignable_v<Interpreter>);
  static_assert(std::is_same_v<decltype(std::declval<Interpreter &>().loadLibrary(std::declval<ExecutableFile>())), std::uint32_t>);
  static_assert(std::is_same_v<decltype(std::declval<const Interpreter &>().getCodeSegment()), std::span<const std::uint8_t>>);
  static_assert(std::is_same_v<decltype(std::declval<const Interpreter &>().getCodeSegmentCapacity()), std::size_t>);

  // Construction registers every native address, and repeated initialization preserves later VM exports with the same name.
  TEST(VMInterpreterTest, InitializesNativeSymbolsWithoutOverwritingVMExports)
  {
    Interpreter VM;
    ASSERT_EQ(VM.getExportSymbolMap().size(), NativeSymbolAddresses.size());
    for (const auto &Symbol : NativeSymbolAddresses)
    {
      const auto Position = VM.getExportSymbolMap().find(Symbol.Name);
      ASSERT_NE(Position, VM.getExportSymbolMap().end());
      EXPECT_EQ(Position->second, hostAddress(Symbol.Address));
      EXPECT_EQ(VM.resolveSymbolAddress(Symbol.Name), Symbol.Address);
    }
    const auto NativeSymbols = VM.getExportSymbolMap();
    VM.initialize();
    EXPECT_EQ(VM.getExportSymbolMap(), NativeSymbols);

    const auto IdentityName = core::NamePool::instance().intern("identity");
    ExecutableFile Shadow;
    Shadow.getImage().resize(8, 0x5A);
    Shadow.getExportSymbols().push_back({IdentityName, 4});
    ASSERT_NE(VM.loadLibrary(std::move(Shadow)), Interpreter::InvalidLibraryHandle);
    EXPECT_EQ(VM.resolveSymbolAddress(IdentityName), static_cast<const void *>(VM.getCodeSegment().data() + 4));
    const auto ShadowedSymbols = VM.getExportSymbolMap();
    VM.initialize();
    EXPECT_EQ(VM.getExportSymbolMap(), ShadowedSymbols);
    EXPECT_EQ(VM.resolveSymbolAddress(IdentityName), static_cast<const void *>(VM.getCodeSegment().data() + 4));
  }

  // A failed first load removes its image, exports, and tentative handle so the first successful load still receives handle one.
  TEST(VMInterpreterTest, RollsBackFailedFirstLibraryLoad)
  {
    const auto ExportName = core::NamePool::instance().intern("interpreter-failed-first-export");
    Interpreter VM;
    const auto InitialSymbols = VM.getExportSymbolMap();
    const auto *InitialData = VM.getCodeSegment().data();
    const auto InitialCapacity = VM.getCodeSegmentCapacity();
    ExecutableFile Invalid;
    Invalid.getImage() = {0x11};
    Invalid.getExportSymbols().push_back({ExportName, 0});
    Invalid.getDynamicImportSymbols().push_back({core::NamePool::instance().intern("interpreter-missing-first-import")});

    EXPECT_EQ(VM.loadLibrary(std::move(Invalid)), Interpreter::InvalidLibraryHandle);
    EXPECT_TRUE(VM.getCodeSegment().empty());
    EXPECT_EQ(VM.getCodeSegment().data(), InitialData);
    EXPECT_EQ(VM.getCodeSegmentCapacity(), InitialCapacity);
    EXPECT_EQ(VM.getExportSymbolMap(), InitialSymbols);
    EXPECT_EQ(VM.findExecutable(1), nullptr);

    ExecutableFile Valid;
    Valid.getImage() = {0x22};
    Valid.getExportSymbols().push_back({ExportName, 0});
    const auto Handle = VM.loadLibrary(std::move(Valid));
    EXPECT_EQ(Handle, 1U);
    const auto *Stored = VM.findExecutable(Handle);
    ASSERT_NE(Stored, nullptr);
    EXPECT_EQ(Stored->getImage(), (std::vector<std::uint8_t>{0x22}));
    auto ExpectedSymbols = InitialSymbols;
    ExpectedSymbols[ExportName] = codeAddress(VM, 0);
    EXPECT_EQ(VM.getExportSymbolMap(), ExpectedSymbols);
  }

  // A failed load after capacity growth preserves prior bytes, current export addresses, borrowed files, and the next handle.
  TEST(VMInterpreterTest, RollsBackFailedLibraryLoadAfterCodeSegmentGrowth)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_VM_INITIAL_CODE_SEGMENT_BYTES");
    ASSERT_TRUE(Environment.set("4"));
    const auto SharedName = core::NamePool::instance().intern("interpreter-rollback-shared-export");
    const auto FirstOnlyName = core::NamePool::instance().intern("interpreter-rollback-first-export");
    const auto NewName = core::NamePool::instance().intern("interpreter-rollback-new-export");
    const std::vector<std::uint8_t> FirstImage{0x11, 0x22, 0x33};
    Interpreter VM;
    ExecutableFile First;
    First.getImage() = FirstImage;
    First.getExportSymbols().push_back({SharedName, 0});
    First.getExportSymbols().push_back({FirstOnlyName, 2});
    const auto FirstHandle = VM.loadLibrary(std::move(First));
    ASSERT_EQ(FirstHandle, 1U);
    const auto *StoredFirst = VM.findExecutable(FirstHandle);
    ASSERT_NE(StoredFirst, nullptr);
    const auto OriginalCapacity = VM.getCodeSegmentCapacity();
    ASSERT_GE(OriginalCapacity, 4U);
    ASSERT_LT(OriginalCapacity, 1024U * 1024U);
    const auto OriginalSymbols = VM.getExportSymbolMap();

    ExecutableFile Invalid;
    Invalid.getImage().resize(OriginalCapacity + 1, 0x44);
    Invalid.getExportSymbols().push_back({SharedName, 1});
    Invalid.getExportSymbols().push_back({NewName, 0});
    Invalid.getDynamicImportSymbols().push_back({core::NamePool::instance().intern("interpreter-missing-later-import")});
    EXPECT_EQ(VM.loadLibrary(std::move(Invalid)), Interpreter::InvalidLibraryHandle);

    expectCodeBytes(VM, FirstImage);
    EXPECT_GT(VM.getCodeSegmentCapacity(), OriginalCapacity);
    auto RestoredSymbols = OriginalSymbols;
    RestoredSymbols[SharedName] = codeAddress(VM, 0);
    RestoredSymbols[FirstOnlyName] = codeAddress(VM, 2);
    EXPECT_EQ(VM.getExportSymbolMap(), RestoredSymbols);
    EXPECT_EQ(VM.findExecutable(FirstHandle), StoredFirst);
    EXPECT_EQ(StoredFirst->getImage(), FirstImage);
    EXPECT_EQ(VM.findExecutable(2), nullptr);

    ExecutableFile Next;
    Next.getImage() = {0x55, 0x66};
    Next.getExportSymbols().push_back({SharedName, 1});
    Next.getExportSymbols().push_back({NewName, 0});
    const auto NextHandle = VM.loadLibrary(std::move(Next));
    EXPECT_EQ(NextHandle, 2U);
    EXPECT_EQ(VM.findExecutable(FirstHandle), StoredFirst);
    expectCodeBytes(VM, (std::vector<std::uint8_t>{0x11, 0x22, 0x33, 0x55, 0x66}));
    auto ExpectedSymbols = OriginalSymbols;
    ExpectedSymbols[SharedName] = codeAddress(VM, 4);
    ExpectedSymbols[FirstOnlyName] = codeAddress(VM, 2);
    ExpectedSymbols[NewName] = codeAddress(VM, 3);
    EXPECT_EQ(VM.getExportSymbolMap(), ExpectedSymbols);
  }

  // Explicit library bases translate exported offsets, and later registrations overwrite only matching names.
  TEST(VMInterpreterTest, LoadsExportSymbolsFromLibraryDescriptors)
  {
    const auto FirstName = core::NamePool::instance().intern("interpreter-direct-first-export");
    const auto SecondName = core::NamePool::instance().intern("interpreter-direct-second-export");
    Interpreter VM;
    ExecutableFile Storage;
    Storage.getImage().resize(264);
    ASSERT_NE(VM.loadLibrary(std::move(Storage)), Interpreter::InvalidLibraryHandle);
    auto ExpectedSymbols = VM.getExportSymbolMap();

    LibraryDescriptor First;
    First.ImageBase = 128;
    First.ExecutableFile.getImage().resize(24);
    First.ExecutableFile.getExportSymbols().push_back({FirstName, 3});
    First.ExecutableFile.getExportSymbols().push_back({SecondName, 17});
    VM.loadExportSymbol(First);
    ExpectedSymbols[FirstName] = codeAddress(VM, 131);
    ExpectedSymbols[SecondName] = codeAddress(VM, 145);
    EXPECT_EQ(VM.getExportSymbolMap(), ExpectedSymbols);

    LibraryDescriptor Second;
    Second.ImageBase = 256;
    Second.ExecutableFile.getImage().resize(8);
    Second.ExecutableFile.getExportSymbols().push_back({FirstName, 7});
    VM.loadExportSymbol(Second);
    ExpectedSymbols[FirstName] = codeAddress(VM, 263);
    EXPECT_EQ(VM.getExportSymbolMap(), ExpectedSymbols);
  }

  // Loading unequal images uses their accumulated bases, overwrites shared exports, and preserves other addresses through growth.
  TEST(VMInterpreterTest, RegistersLibraryExportsAcrossCodeSegmentGrowth)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_VM_INITIAL_CODE_SEGMENT_BYTES");
    ASSERT_TRUE(Environment.set("4"));
    const auto SharedName = core::NamePool::instance().intern("interpreter-loaded-shared-export");
    const auto FirstOnlyName = core::NamePool::instance().intern("interpreter-loaded-first-export");
    const auto SecondOnlyName = core::NamePool::instance().intern("interpreter-loaded-second-export");
    Interpreter VM;
    auto ExpectedSymbols = VM.getExportSymbolMap();
    const auto InitialCapacity = VM.getCodeSegmentCapacity();
    ASSERT_GE(InitialCapacity, 4U);
    ASSERT_LT(InitialCapacity, 1024U * 1024U);

    ExecutableFile First;
    First.getImage() = {0x11, 0x22, 0x33};
    First.getExportSymbols().push_back({SharedName, 0});
    First.getExportSymbols().push_back({FirstOnlyName, 2});
    const auto FirstHandle = VM.loadLibrary(std::move(First));
    ASSERT_NE(FirstHandle, Interpreter::InvalidLibraryHandle);
    ExpectedSymbols[SharedName] = codeAddress(VM, 0);
    ExpectedSymbols[FirstOnlyName] = codeAddress(VM, 2);
    EXPECT_EQ(VM.getExportSymbolMap(), ExpectedSymbols);

    ExecutableFile Second;
    Second.getImage().resize(InitialCapacity + 1, 0x44);
    Second.getExportSymbols().push_back({SharedName, 1});
    Second.getExportSymbols().push_back({SecondOnlyName, 0});
    const auto SecondHandle = VM.loadLibrary(std::move(Second));
    ASSERT_NE(SecondHandle, Interpreter::InvalidLibraryHandle);
    EXPECT_NE(FirstHandle, SecondHandle);
    EXPECT_GT(VM.getCodeSegmentCapacity(), InitialCapacity);
    ExpectedSymbols[SharedName] = codeAddress(VM, 4);
    ExpectedSymbols[FirstOnlyName] = codeAddress(VM, 2);
    ExpectedSymbols[SecondOnlyName] = codeAddress(VM, 3);
    EXPECT_EQ(VM.getExportSymbolMap(), ExpectedSymbols);
  }

  // Successful and failed growth preserve old VM bindings and native addresses across shadowing, while an in-capacity failure preserves the buffer.
  TEST(VMInterpreterTest, RebasesBoundReferencesAcrossGrowthWithoutRebindingShadowedSymbols)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_VM_INITIAL_CODE_SEGMENT_BYTES");
    ASSERT_TRUE(Environment.set("4"));
    const auto SharedName = core::NamePool::instance().intern("interpreter-rebase-shared-export");
    const auto StableName = core::NamePool::instance().intern("interpreter-rebase-stable-export");
    const auto IdentityName = core::NamePool::instance().intern("identity");
    const auto NativeAddress = hostAddress(reinterpret_cast<void *>(&identityStub));
    Interpreter VM;

    ExecutableFile First;
    First.getImage().assign(32, 0xA1);
    const auto FirstImage = First.getImage();
    First.getExportSymbols() = {
        {SharedName, 24},
        {StableName, 24},
    };
    First.getDynamicImportSymbols() = {
        {SharedName},
        {IdentityName},
    };
    First.getDynamicImportSymbolRelocations() = {
        {0, 0, 1024},
        {8, 0, -24},
        {16, 1, 0},
    };
    const auto FirstHandle = VM.loadLibrary(std::move(First));
    ASSERT_EQ(FirstHandle, 1U);
    EXPECT_EQ(readBoundAddress(VM, 0), codeAddress(VM, 24) + 1024);
    EXPECT_EQ(readBoundAddress(VM, 8), codeAddress(VM, 0));
    EXPECT_EQ(readBoundAddress(VM, 16), NativeAddress);
    const auto FirstCapacity = VM.getCodeSegmentCapacity();
    ASSERT_LT(FirstCapacity, 1024U * 1024U);

    ExecutableFile Second;
    Second.getImage().assign(FirstCapacity + 16, 0xB2);
    const auto SecondImage = Second.getImage();
    Second.getExportSymbols() = {
        {SharedName, 8},
        {IdentityName, 16},
    };
    Second.getDynamicImportSymbols().push_back({StableName});
    Second.getDynamicImportSymbolRelocations().push_back({0, 0, 7});
    const auto SecondHandle = VM.loadLibrary(std::move(Second));
    ASSERT_EQ(SecondHandle, 2U);
    EXPECT_GT(VM.getCodeSegmentCapacity(), FirstCapacity);
    EXPECT_EQ(readBoundAddress(VM, 0), codeAddress(VM, 24) + 1024);
    EXPECT_EQ(readBoundAddress(VM, 8), codeAddress(VM, 0));
    EXPECT_EQ(readBoundAddress(VM, 16), NativeAddress);
    EXPECT_EQ(readBoundAddress(VM, 32), codeAddress(VM, 31));
    EXPECT_EQ(VM.resolveSymbolAddress(SharedName), static_cast<const void *>(VM.getCodeSegment().data() + 40));
    EXPECT_EQ(VM.resolveSymbolAddress(IdentityName), static_cast<const void *>(VM.getCodeSegment().data() + 48));
    EXPECT_EQ(VM.resolveSymbolAddress(StableName), static_cast<const void *>(VM.getCodeSegment().data() + 24));

    auto ExpectedAfterFailedGrowth = std::vector<std::uint8_t>(VM.getCodeSegment().begin(), VM.getCodeSegment().end());
    const auto BeforeFailedGrowthCapacity = VM.getCodeSegmentCapacity();
    auto ExpectedAfterFailedGrowthSymbols = VM.getExportSymbolMap();
    ExecutableFile FailedGrowth;
    FailedGrowth.getImage().assign(BeforeFailedGrowthCapacity + 16, 0xE5);
    FailedGrowth.getExportSymbols().push_back({SharedName, 0});
    FailedGrowth.getExportSymbols().push_back({core::NamePool::instance().intern("interpreter-rebase-failed-export"), 1});
    FailedGrowth.getDynamicImportSymbols().push_back({core::NamePool::instance().intern("interpreter-rebase-growth-missing-import")});

    EXPECT_EQ(VM.loadLibrary(std::move(FailedGrowth)), Interpreter::InvalidLibraryHandle);
    EXPECT_GT(VM.getCodeSegmentCapacity(), BeforeFailedGrowthCapacity);
    storeExpectedAddress(ExpectedAfterFailedGrowth, 0, codeAddress(VM, 24) + 1024);
    storeExpectedAddress(ExpectedAfterFailedGrowth, 8, codeAddress(VM, 0));
    storeExpectedAddress(ExpectedAfterFailedGrowth, 32, codeAddress(VM, 31));
    expectCodeBytes(VM, ExpectedAfterFailedGrowth);
    ExpectedAfterFailedGrowthSymbols[SharedName] = codeAddress(VM, 40);
    ExpectedAfterFailedGrowthSymbols[IdentityName] = codeAddress(VM, 48);
    ExpectedAfterFailedGrowthSymbols[StableName] = codeAddress(VM, 24);
    EXPECT_EQ(VM.getExportSymbolMap(), ExpectedAfterFailedGrowthSymbols);
    EXPECT_EQ(VM.findExecutable(3), nullptr);

    const std::vector<std::uint8_t> BeforeFailure(VM.getCodeSegment().begin(), VM.getCodeSegment().end());
    const auto *BeforeFailureData = VM.getCodeSegment().data();
    const auto BeforeFailureCapacity = VM.getCodeSegmentCapacity();
    const auto BeforeFailureSymbols = VM.getExportSymbolMap();
    ExecutableFile Invalid;
    Invalid.getImage().assign(std::min<std::size_t>(8, BeforeFailureCapacity - BeforeFailure.size()), 0xD4);
    if (!Invalid.getImage().empty())
    {
      Invalid.getExportSymbols().push_back({SharedName, 0});
    }
    Invalid.getDynamicImportSymbols().push_back({core::NamePool::instance().intern("interpreter-rebase-missing-import")});

    EXPECT_EQ(VM.loadLibrary(std::move(Invalid)), Interpreter::InvalidLibraryHandle);
    expectCodeBytes(VM, BeforeFailure);
    EXPECT_EQ(VM.getCodeSegment().data(), BeforeFailureData);
    EXPECT_EQ(VM.getCodeSegmentCapacity(), BeforeFailureCapacity);
    EXPECT_EQ(VM.getExportSymbolMap(), BeforeFailureSymbols);
    EXPECT_EQ(VM.findExecutable(3), nullptr);

    ExecutableFile Third;
    Third.getImage().assign(8, 0xC3);
    ASSERT_EQ(VM.loadLibrary(std::move(Third)), 3U);
    EXPECT_EQ(readBoundAddress(VM, 0), codeAddress(VM, 24) + 1024);
    EXPECT_EQ(readBoundAddress(VM, 8), codeAddress(VM, 0));
    EXPECT_EQ(readBoundAddress(VM, 16), NativeAddress);
    EXPECT_EQ(readBoundAddress(VM, 32), codeAddress(VM, 31));
    EXPECT_EQ(VM.resolveSymbolAddress(SharedName), static_cast<const void *>(VM.getCodeSegment().data() + 40));
    EXPECT_EQ(VM.resolveSymbolAddress(IdentityName), static_cast<const void *>(VM.getCodeSegment().data() + 48));
    VM.initialize();
    EXPECT_EQ(VM.resolveSymbolAddress(IdentityName), static_cast<const void *>(VM.getCodeSegment().data() + 48));
    const auto *StoredFirst = VM.findExecutable(FirstHandle);
    const auto *StoredSecond = VM.findExecutable(SecondHandle);
    ASSERT_NE(StoredFirst, nullptr);
    ASSERT_NE(StoredSecond, nullptr);
    EXPECT_EQ(StoredFirst->getImage(), FirstImage);
    EXPECT_EQ(StoredSecond->getImage(), SecondImage);
  }

  // The default code segment reserves one MiB without making unused storage part of its image bytes.
  TEST(VMInterpreterTest, ReservesDefaultCodeSegmentStorageWithoutImageBytes)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_VM_INITIAL_CODE_SEGMENT_BYTES");
    ASSERT_TRUE(Environment.set(nullptr));
    const Interpreter VM;
    EXPECT_TRUE(VM.getCodeSegment().empty());
    EXPECT_GE(VM.getCodeSegmentCapacity(), 1024U * 1024U);
  }

  // A small configured reserve grows for later images while empty loads preserve bytes and earlier executable references.
  TEST(VMInterpreterTest, AppendsImagesAcrossConfiguredCodeSegmentGrowth)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_VM_INITIAL_CODE_SEGMENT_BYTES");
    ASSERT_TRUE(Environment.set("4"));
    Interpreter VM;
    const auto InitialCapacity = VM.getCodeSegmentCapacity();
    ASSERT_GE(InitialCapacity, 4U);
    ASSERT_LT(InitialCapacity, 1024U * 1024U);
    EXPECT_TRUE(VM.getCodeSegment().empty());

    const std::vector<std::uint8_t> FirstImage{0x11, 0x22, 0x33};
    ExecutableFile First;
    First.getImage() = FirstImage;
    const auto FirstHandle = VM.loadLibrary(std::move(First));
    ASSERT_NE(FirstHandle, Interpreter::InvalidLibraryHandle);
    const auto *StoredFirst = VM.findExecutable(FirstHandle);
    ASSERT_NE(StoredFirst, nullptr);
    expectCodeBytes(VM, FirstImage);

    const auto EmptyHandle = VM.loadLibrary(ExecutableFile{});
    ASSERT_NE(EmptyHandle, Interpreter::InvalidLibraryHandle);
    const auto *StoredEmpty = VM.findExecutable(EmptyHandle);
    ASSERT_NE(StoredEmpty, nullptr);
    EXPECT_TRUE(StoredEmpty->getImage().empty());
    expectCodeBytes(VM, FirstImage);

    const std::vector<std::uint8_t> SecondImage(InitialCapacity + 1, 0x44);
    ExecutableFile Second;
    Second.getImage() = SecondImage;
    const auto SecondHandle = VM.loadLibrary(std::move(Second));
    ASSERT_NE(SecondHandle, Interpreter::InvalidLibraryHandle);
    auto ExpectedImage = FirstImage;
    ExpectedImage.insert(ExpectedImage.end(), SecondImage.begin(), SecondImage.end());
    expectCodeBytes(VM, ExpectedImage);
    EXPECT_GT(VM.getCodeSegmentCapacity(), InitialCapacity);
    EXPECT_EQ(VM.findExecutable(FirstHandle), StoredFirst);
    EXPECT_EQ(StoredFirst->getImage(), FirstImage);
    EXPECT_EQ(VM.findExecutable(EmptyHandle), StoredEmpty);
    EXPECT_TRUE(StoredEmpty->getImage().empty());
    const auto *StoredSecond = VM.findExecutable(SecondHandle);
    ASSERT_NE(StoredSecond, nullptr);
    EXPECT_EQ(StoredSecond->getImage(), SecondImage);
  }

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
    const auto ImportName = core::NamePool::instance().intern("identity");
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
