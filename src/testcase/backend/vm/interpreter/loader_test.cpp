#include "../../../core/support/environment_test_support.h"

#include "ink/backend/vm/executable/executable_file.h"
#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/backend/vm/interpreter/loader.h"
#include "ink/backend/vm/runtime_stub/builtin_stub.h"
#include "ink/backend/vm/vm_runtime/frame.h"
#include "ink/core/config.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace ink::vm::test
{
  static_assert(std::is_same_v<decltype(std::declval<Interpreter &>().getDiagnostics()), core::DiagnosticEngine &>);
  static_assert(std::is_same_v<decltype(std::declval<const Interpreter &>().getDiagnostics()), const core::DiagnosticEngine &>);

  namespace
  {
    std::uint64_t identityAddress() noexcept
    {
      return static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(&identityStub));
    }

    ExecutableFile makeIdentityExecutable(std::size_t ImageBytes)
    {
      ExecutableFile Executable;
      Executable.getImage().assign(ImageBytes, 0xD6);
      Executable.getDynamicImportSymbols().push_back({core::NamePool::instance().intern("identity")});
      return Executable;
    }

    std::vector<std::uint8_t> makeCodeSegment(const ExecutableFile &Executable, std::size_t PrefixBytes = 8, std::size_t SuffixBytes = 7)
    {
      std::vector<std::uint8_t> CodeSegment(PrefixBytes, 0xA5);
      CodeSegment.insert(CodeSegment.end(), Executable.getImage().begin(), Executable.getImage().end());
      CodeSegment.insert(CodeSegment.end(), SuffixBytes, 0xB6);
      return CodeSegment;
    }

    void storeLittleEndian(std::vector<std::uint8_t> &Bytes, std::size_t Offset, std::uint64_t Value)
    {
      for (std::size_t Index = 0; Index < 8; ++Index)
      {
        Bytes[Offset + Index] = static_cast<std::uint8_t>(Value >> (Index * 8));
      }
    }

    std::uint64_t readLittleEndian(const std::vector<std::uint8_t> &Bytes, std::size_t Offset)
    {
      std::uint64_t Value = 0;
      for (std::size_t Index = 0; Index < 8; ++Index)
      {
        Value |= static_cast<std::uint64_t>(Bytes[Offset + Index]) << (Index * 8);
      }
      return Value;
    }

    void expectExecutableUnchanged(const ExecutableFile &Actual, const ExecutableFile &Expected)
    {
      EXPECT_EQ(Actual.getImage(), Expected.getImage());
      EXPECT_EQ(Actual.getEntryOffset(), Expected.getEntryOffset());
      ASSERT_EQ(Actual.getDynamicImportSymbols().size(), Expected.getDynamicImportSymbols().size());
      for (std::size_t Index = 0; Index < Expected.getDynamicImportSymbols().size(); ++Index)
      {
        EXPECT_EQ(Actual.getDynamicImportSymbols()[Index].Name, Expected.getDynamicImportSymbols()[Index].Name);
      }
      ASSERT_EQ(Actual.getDynamicImportSymbolRelocations().size(), Expected.getDynamicImportSymbolRelocations().size());
      for (std::size_t Index = 0; Index < Expected.getDynamicImportSymbolRelocations().size(); ++Index)
      {
        const auto &ActualRelocation = Actual.getDynamicImportSymbolRelocations()[Index];
        const auto &ExpectedRelocation = Expected.getDynamicImportSymbolRelocations()[Index];
        EXPECT_EQ(ActualRelocation.ImageOffset, ExpectedRelocation.ImageOffset);
        EXPECT_EQ(ActualRelocation.SymbolIndex, ExpectedRelocation.SymbolIndex);
        EXPECT_EQ(ActualRelocation.Addend, ExpectedRelocation.Addend);
      }
      ASSERT_EQ(Actual.getImageBaseAddressRelocations().size(), Expected.getImageBaseAddressRelocations().size());
      for (std::size_t Index = 0; Index < Expected.getImageBaseAddressRelocations().size(); ++Index)
      {
        const auto &ActualRelocation = Actual.getImageBaseAddressRelocations()[Index];
        const auto &ExpectedRelocation = Expected.getImageBaseAddressRelocations()[Index];
        EXPECT_EQ(ActualRelocation.ImageOffset, ExpectedRelocation.ImageOffset);
        EXPECT_EQ(ActualRelocation.TargetOffset, ExpectedRelocation.TargetOffset);
        EXPECT_EQ(ActualRelocation.Addend, ExpectedRelocation.Addend);
      }
      ASSERT_EQ(Actual.getExportSymbols().size(), Expected.getExportSymbols().size());
      for (std::size_t Index = 0; Index < Expected.getExportSymbols().size(); ++Index)
      {
        EXPECT_EQ(Actual.getExportSymbols()[Index].Name, Expected.getExportSymbols()[Index].Name);
        EXPECT_EQ(Actual.getExportSymbols()[Index].Offset, Expected.getExportSymbols()[Index].Offset);
      }
    }

    void expectLibraryLinkDiagnostic(const core::CollectingDiagnosticConsumer &Collector, std::string_view SymbolName)
    {
      ASSERT_EQ(Collector.diagnostics().size(), 1U);
      const auto &Diagnostic = Collector.diagnostics().front();
      EXPECT_EQ(Diagnostic.Kind, core::DiagnosticKind::VMLibraryLinkFailed);
      EXPECT_EQ(Diagnostic.classification(), core::DiagnosticClass::User);
      EXPECT_STREQ(Diagnostic.code(), "INK-V0004");
      EXPECT_EQ(core::diagnosticDefaultSeverity(Diagnostic.Kind), core::DiagnosticSeverity::Error);
      ASSERT_EQ(Diagnostic.Arguments.size(), 2U);
      EXPECT_EQ(Diagnostic.Arguments[0].Name, core::DiagnosticArgumentName::Context);
      EXPECT_EQ(Diagnostic.Arguments[1].Name, core::DiagnosticArgumentName::Description);
      const auto &Context = std::get<std::string>(Diagnostic.Arguments[0].Value);
      const auto &Description = std::get<std::string>(Diagnostic.Arguments[1].Value);
      const auto Message = Context + " " + Description;
      EXPECT_NE(Message.find("import"), std::string::npos);
      if (!SymbolName.empty())
      {
        EXPECT_NE(Message.find(SymbolName), std::string::npos);
      }
    }

    void expectLinkFailureWithoutWrites(Interpreter &VM, Loader &LibraryLoader, std::vector<std::uint8_t> &CodeSegment, std::uint64_t ImageBase, ExecutableFile &Executable, std::string_view SymbolName)
    {
      core::CollectingDiagnosticConsumer Collector;
      VM.getDiagnostics().addConsumer(Collector);
      const auto OriginalCodeSegment = CodeSegment;
      const auto OriginalExecutable = Executable;
      EXPECT_FALSE(LibraryLoader.loadLibrary(CodeSegment, ImageBase, Executable));
      VM.getDiagnostics().removeConsumer(Collector);
      EXPECT_EQ(CodeSegment, OriginalCodeSegment);
      expectExecutableUnchanged(Executable, OriginalExecutable);
      expectLibraryLinkDiagnostic(Collector, SymbolName);
    }

    void expectLoaderICE(Loader &LibraryLoader, std::vector<std::uint8_t> &CodeSegment, std::uint64_t ImageBase, ExecutableFile &Executable)
    {
      EXPECT_DEATH(static_cast<void>(LibraryLoader.loadLibrary(CodeSegment, ImageBase, Executable)), "INK-V0002");
    }
  }

  // Symbol queries return the registered Frame adapter or nullptr without publishing load diagnostics.
  TEST(VMLoaderTest, ResolvesNativeSymbolAddressesWithoutReportingDiagnostics)
  {
    Interpreter VM;
    core::CollectingDiagnosticConsumer Collector;
    VM.getDiagnostics().addConsumer(Collector);
    const Interpreter &View = VM;
    const auto IdentityName = core::NamePool::instance().intern("identity");
    const auto MissingName = core::NamePool::instance().intern("loader-query-missing-import");

    EXPECT_EQ(View.resolveSymbolAddress(IdentityName), reinterpret_cast<void *>(&identityStub));
    EXPECT_EQ(View.resolveSymbolAddress(MissingName), nullptr);
    EXPECT_EQ(View.resolveSymbolAddress(core::NAME_None), nullptr);
    EXPECT_TRUE(Collector.diagnostics().empty());
    VM.getDiagnostics().removeConsumer(Collector);
  }

  // Nonaligned image offsets are valid when their actual patch addresses are aligned, including unsorted and adjacent fields.
  TEST(VMLoaderTest, PatchesUnalignedNativeAddressesWithSignedAddendsWithoutChangingExecutable)
  {
    Interpreter VM;
    Loader LibraryLoader(VM);
    auto Executable = makeIdentityExecutable(48);
    Executable.getEntryOffset() = 2;
    Executable.getDynamicImportSymbolRelocations() = {
        {11, 0, 7},
        {3, 0, -5},
        {27, 0, 0},
    };
    Executable.getImageBaseAddressRelocations().push_back({40, 2, 3});
    Executable.getExportSymbols().push_back({core::NamePool::instance().intern("loader-patched-export"), 2});
    const auto OriginalExecutable = Executable;
    auto CodeSegment = makeCodeSegment(Executable, 5);
    auto Expected = CodeSegment;
    const auto Address = identityAddress();
    ASSERT_GE(Address, 5U);
    ASSERT_LE(Address, std::numeric_limits<std::uint64_t>::max() - 7);
    // The five-byte prefix moves image fields 3, 11, and 27 to aligned code offsets 8, 16, and 32.
    storeLittleEndian(Expected, 8, Address - 5);
    storeLittleEndian(Expected, 16, Address + 7);
    storeLittleEndian(Expected, 32, Address);

    ASSERT_TRUE(LibraryLoader.loadLibrary(CodeSegment, 5, Executable));

    EXPECT_EQ(CodeSegment, Expected);
    expectExecutableUnchanged(Executable, OriginalExecutable);
  }

  // A relocated identity address is the Frame adapter and can write a result through the native stack protocol.
  TEST(VMLoaderTest, InvokesFrameAdapterThroughRelocatedNativeAddress)
  {
    Interpreter VM;
    Loader LibraryLoader(VM);
    auto Executable = makeIdentityExecutable(8);
    Executable.getDynamicImportSymbolRelocations().push_back({0, 0, 0});
    auto CodeSegment = makeCodeSegment(Executable);
    ASSERT_TRUE(LibraryLoader.loadLibrary(CodeSegment, 8, Executable));
    const auto Address = readLittleEndian(CodeSegment, 8);
    ASSERT_EQ(Address, identityAddress());
    const auto Stub = reinterpret_cast<decltype(&identityStub)>(static_cast<std::uintptr_t>(Address));

    alignas(8) std::array<std::uint8_t, 32> Stack;
    Stack.fill(0x3D);
    const std::int64_t Argument = -0x0123456789ABCDEF;
    std::memcpy(Stack.data() + 16, &Argument, sizeof(Argument));
    auto Expected = Stack;
    std::memcpy(Expected.data() + 8, &Argument, sizeof(Argument));
    Frame CallFrame{Stack.data() + 24};
    auto *const InitialSP = CallFrame.SP;

    Stub(&CallFrame);

    EXPECT_EQ(Stack, Expected);
    EXPECT_EQ(CallFrame.SP, InitialSP);
  }

  // Exports are registered before imports so a library can relocate a reference to its own host address.
  TEST(VMLoaderTest, ResolvesSameLibraryExportsBeforePatchingImports)
  {
    Interpreter VM;
    Loader LibraryLoader(VM);
    const auto SelfName = core::NamePool::instance().intern("loader-self-export");
    ExecutableFile Executable;
    Executable.getImage().assign(24, 0xD6);
    Executable.getExportSymbols().push_back({SelfName, 16});
    Executable.getDynamicImportSymbols().push_back({SelfName});
    Executable.getDynamicImportSymbolRelocations().push_back({0, 0, 0});
    const auto OriginalExecutable = Executable;
    auto CodeSegment = makeCodeSegment(Executable);
    auto Expected = CodeSegment;
    const auto Address = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(CodeSegment.data() + 24));
    storeLittleEndian(Expected, 8, Address);

    ASSERT_TRUE(LibraryLoader.loadLibrary(CodeSegment, 8, Executable));

    EXPECT_EQ(CodeSegment, Expected);
    EXPECT_EQ(VM.resolveSymbolAddress(SelfName), static_cast<void *>(CodeSegment.data() + 24));
    const auto Position = VM.getExportSymbolMap().find(SelfName);
    ASSERT_NE(Position, VM.getExportSymbolMap().end());
    EXPECT_EQ(Position->second, Address);
    expectExecutableUnchanged(Executable, OriginalExecutable);
  }

  // Every import must resolve, including unused entries, and a missing name reports a recoverable link error before any patch is written.
  TEST(VMLoaderTest, ReportsMissingImportsIncludingUnusedEntriesWithoutWriting)
  {
    Interpreter VM;
    Loader LibraryLoader(VM);
    auto Executable = makeIdentityExecutable(16);
    const auto MissingName = core::NamePool::instance().intern("loader-missing-import");
    Executable.getDynamicImportSymbols().push_back({MissingName});
    auto CodeSegment = makeCodeSegment(Executable);

    // No relocation uses either import, so failure establishes that the complete import table is resolved.
    expectLinkFailureWithoutWrites(VM, LibraryLoader, CodeSegment, 8, Executable, "loader-missing-import");
    Executable.getDynamicImportSymbolRelocations().push_back({0, 0, 7});
    expectLinkFailureWithoutWrites(VM, LibraryLoader, CodeSegment, 8, Executable, "loader-missing-import");
    Executable.getDynamicImportSymbolRelocations().push_back({8, 1, 0});
    expectLinkFailureWithoutWrites(VM, LibraryLoader, CodeSegment, 8, Executable, "loader-missing-import");
    Executable.getDynamicImportSymbols()[1].Name = core::NAME_None;
    expectLinkFailureWithoutWrites(VM, LibraryLoader, CodeSegment, 8, Executable, {});
  }

  // Relocation indices must select the resolved import table; malformed indices are internal executable errors.
  TEST(VMLoaderDeathTest, RejectsOutOfRangeImportIndices)
  {
    Interpreter VM;
    Loader LibraryLoader(VM);
    auto Executable = makeIdentityExecutable(16);
    Executable.getDynamicImportSymbolRelocations().push_back({0, 0, 0});
    auto CodeSegment = makeCodeSegment(Executable);
    Executable.getDynamicImportSymbolRelocations()[0].SymbolIndex = 1;
    expectLoaderICE(LibraryLoader, CodeSegment, 8, Executable);
    Executable.getDynamicImportSymbolRelocations()[0].SymbolIndex = std::numeric_limits<std::uint32_t>::max();
    expectLoaderICE(LibraryLoader, CodeSegment, 8, Executable);
    Executable.getDynamicImportSymbols().clear();
    Executable.getDynamicImportSymbolRelocations()[0].SymbolIndex = 0;
    expectLoaderICE(LibraryLoader, CodeSegment, 8, Executable);
  }

  // Truncated fields, invalid image windows, and aliasing the unbound source image are fatal internal invariants.
  TEST(VMLoaderDeathTest, RejectsInvalidRelocationFieldsAndImageWindows)
  {
    Interpreter VM;
    Loader LibraryLoader(VM);
    auto Executable = makeIdentityExecutable(16);
    Executable.getDynamicImportSymbolRelocations().push_back({0, 0, 0});
    auto CodeSegment = makeCodeSegment(Executable);
    const std::uint64_t InvalidFieldOffsets[] = {
        9,
        16,
        std::numeric_limits<std::uint64_t>::max(),
    };
    for (const auto Offset : InvalidFieldOffsets)
    {
      SCOPED_TRACE(Offset);
      Executable.getDynamicImportSymbolRelocations()[0].ImageOffset = Offset;
      expectLoaderICE(LibraryLoader, CodeSegment, 8, Executable);
    }
    Executable.getDynamicImportSymbolRelocations()[0].ImageOffset = 0;
    expectLoaderICE(LibraryLoader, CodeSegment, CodeSegment.size() + 1, Executable);
    expectLoaderICE(LibraryLoader, CodeSegment, std::numeric_limits<std::uint64_t>::max(), Executable);
    expectLoaderICE(LibraryLoader, CodeSegment, CodeSegment.size() - Executable.getImage().size() + 1, Executable);
    expectLoaderICE(LibraryLoader, Executable.getImage(), 0, Executable);
    Executable.getImage().resize(7);
    expectLoaderICE(LibraryLoader, CodeSegment, 8, Executable);
  }

#if INK_BUILD_DEBUG || INK_BUILD_DEVELOPMENT
  // Assertion-enabled builds reject an actual patch address that is misaligned even when the image base is aligned.
  TEST(VMLoaderDeathTest, RejectsMisalignedActualPatchAddressWhenAssertionsAreEnabled)
  {
    Interpreter VM;
    Loader LibraryLoader(VM);
    auto Executable = makeIdentityExecutable(16);
    Executable.getDynamicImportSymbolRelocations().push_back({1, 0, 0});
    auto CodeSegment = makeCodeSegment(Executable);
    ASSERT_EQ(reinterpret_cast<std::uintptr_t>(CodeSegment.data() + 8) % 8, 0U);

    EXPECT_DEATH(static_cast<void>(LibraryLoader.loadLibrary(CodeSegment, 8, Executable)), "check failed");
  }
#endif

  // Subtracting more than the resolved address is fatal, including INT64_MIN whose magnitude must not overflow signed arithmetic.
  TEST(VMLoaderDeathTest, RejectsAddressUnderflowIncludingMinimumSignedAddend)
  {
    Interpreter VM;
    Loader LibraryLoader(VM);
    const auto Address = identityAddress();
    ASSERT_GT(Address, 0U);
    ASSERT_LT(Address, static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()));
    auto Executable = makeIdentityExecutable(8);
    Executable.getDynamicImportSymbolRelocations().push_back({0, 0, std::numeric_limits<std::int64_t>::min()});
    auto CodeSegment = makeCodeSegment(Executable);
    expectLoaderICE(LibraryLoader, CodeSegment, 8, Executable);
    Executable.getDynamicImportSymbolRelocations()[0].Addend = -static_cast<std::int64_t>(Address) - 1;
    expectLoaderICE(LibraryLoader, CodeSegment, 8, Executable);
  }

  // Signed addends can produce exact zero or one and valid addresses with the high unsigned bit set.
  TEST(VMLoaderTest, PreservesRepresentableSignedAddendBoundaryResults)
  {
    Interpreter VM;
    Loader LibraryLoader(VM);
    const auto Address = identityAddress();
    ASSERT_GT(Address, 0U);
    ASSERT_LT(Address, static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()));
    auto Executable = makeIdentityExecutable(8);
    Executable.getDynamicImportSymbolRelocations().push_back({0, 0, 0});
    auto CodeSegment = makeCodeSegment(Executable);
    Executable.getDynamicImportSymbolRelocations()[0].Addend = -static_cast<std::int64_t>(Address);
    auto Expected = CodeSegment;
    storeLittleEndian(Expected, 8, 0);
    ASSERT_TRUE(LibraryLoader.loadLibrary(CodeSegment, 8, Executable));
    EXPECT_EQ(CodeSegment, Expected);

    Executable.getDynamicImportSymbolRelocations()[0].Addend = -static_cast<std::int64_t>(Address) + 1;
    storeLittleEndian(Expected, 8, 1);
    ASSERT_TRUE(LibraryLoader.loadLibrary(CodeSegment, 8, Executable));
    EXPECT_EQ(CodeSegment, Expected);

    Executable.getDynamicImportSymbolRelocations()[0].Addend = std::numeric_limits<std::int64_t>::max();
    storeLittleEndian(Expected, 8, Address + static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max()));
    ASSERT_TRUE(LibraryLoader.loadLibrary(CodeSegment, 8, Executable));
    EXPECT_EQ(CodeSegment, Expected);
  }

  // A recoverable import link error uses the interpreter's diagnostics and restores state before a later valid load reuses the handle.
  TEST(VMLoaderTest, RollsBackInterpreterStateAfterUnresolvedImport)
  {
    core::test::ScopedEnvironmentVariable Environment("INK_VM_INITIAL_CODE_SEGMENT_BYTES");
    ASSERT_TRUE(Environment.set("4"));
    const auto ExportName = core::NamePool::instance().intern("loader-transaction-export");
    const auto FailedExportName = core::NamePool::instance().intern("loader-transaction-failed-export");
    Interpreter VM;
    ExecutableFile First;
    First.getImage() = {0x11, 0x22, 0x33};
    First.getExportSymbols().push_back({ExportName, 0});
    const auto FirstHandle = VM.loadLibrary(std::move(First));
    ASSERT_EQ(FirstHandle, 1U);
    const auto *StoredFirst = VM.findExecutable(FirstHandle);
    ASSERT_NE(StoredFirst, nullptr);
    const std::vector<std::uint8_t> OriginalBytes(VM.getCodeSegment().begin(), VM.getCodeSegment().end());
    const auto OriginalCapacity = VM.getCodeSegmentCapacity();
    const auto OriginalExports = VM.getExportSymbolMap();
    ASSERT_GE(OriginalCapacity, 4U);
    ASSERT_LT(OriginalCapacity, 1024U * 1024U);

    auto Invalid = makeIdentityExecutable(OriginalCapacity + 17);
    Invalid.getDynamicImportSymbols().push_back({core::NamePool::instance().intern("loader-transaction-missing-import")});
    Invalid.getDynamicImportSymbolRelocations() = {
        {0, 0, 0},
        {8, 1, 0},
    };
    Invalid.getExportSymbols().push_back({ExportName, 1});
    Invalid.getExportSymbols().push_back({FailedExportName, 0});
    core::CollectingDiagnosticConsumer Collector;
    VM.getDiagnostics().addConsumer(Collector);

    EXPECT_EQ(VM.loadLibrary(std::move(Invalid)), Interpreter::InvalidLibraryHandle);
    VM.getDiagnostics().removeConsumer(Collector);
    expectLibraryLinkDiagnostic(Collector, "loader-transaction-missing-import");
    EXPECT_TRUE(std::ranges::equal(VM.getCodeSegment(), OriginalBytes));
    EXPECT_GT(VM.getCodeSegmentCapacity(), OriginalCapacity);
    auto RestoredExports = OriginalExports;
    RestoredExports[ExportName] = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(VM.getCodeSegment().data()));
    EXPECT_EQ(VM.getExportSymbolMap(), RestoredExports);
    EXPECT_EQ(VM.findExecutable(FirstHandle), StoredFirst);
    EXPECT_EQ(VM.findExecutable(2), nullptr);

    auto Next = makeIdentityExecutable(16);
    Next.getDynamicImportSymbolRelocations().push_back({5, 0, 0});
    Next.getExportSymbols().push_back({ExportName, 8});
    auto Expected = OriginalBytes;
    Expected.insert(Expected.end(), Next.getImage().begin(), Next.getImage().end());
    storeLittleEndian(Expected, 8, identityAddress());
    const auto NextHandle = VM.loadLibrary(std::move(Next));

    ASSERT_EQ(NextHandle, 2U);
    EXPECT_TRUE(std::ranges::equal(VM.getCodeSegment(), Expected));
    EXPECT_EQ(VM.findExecutable(FirstHandle), StoredFirst);
    EXPECT_EQ(StoredFirst->getImage(), OriginalBytes);
    auto ExpectedExports = OriginalExports;
    ExpectedExports[ExportName] = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(VM.getCodeSegment().data() + 11));
    EXPECT_EQ(VM.getExportSymbolMap(), ExpectedExports);
    EXPECT_EQ(VM.getExportSymbolMap().find(FailedExportName), VM.getExportSymbolMap().end());
  }

  // Export offsets at or beyond the image end are fatal executable invariants, including exporting from an empty image.
  TEST(VMLoaderDeathTest, RejectsInvalidExportOffsets)
  {
    Interpreter VM;
    Loader LibraryLoader(VM);
    auto Executable = makeIdentityExecutable(16);
    Executable.getDynamicImportSymbolRelocations().push_back({0, 0, 0});
    Executable.getExportSymbols().push_back({core::NamePool::instance().intern("loader-outside-image-export"), 16});
    auto CodeSegment = makeCodeSegment(Executable);
    expectLoaderICE(LibraryLoader, CodeSegment, 8, Executable);
    Executable.getExportSymbols()[0].Offset = std::numeric_limits<std::uint64_t>::max();
    expectLoaderICE(LibraryLoader, CodeSegment, 8, Executable);
    Executable.getImage().clear();
    Executable.getDynamicImportSymbolRelocations().clear();
    Executable.getExportSymbols()[0].Offset = 0;
    expectLoaderICE(LibraryLoader, CodeSegment, 8, Executable);
  }
} // namespace ink::vm::test
