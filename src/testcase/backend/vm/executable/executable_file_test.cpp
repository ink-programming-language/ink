#include "ink/backend/vm/executable/executable_file.h"
#include "ink/backend/vm/executable/executable_file_validator.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace ink::vm::test
{
  namespace
  {
    ExecutableFile makeValidExecutableFile()
    {
      core::NamePool &Names = core::NamePool::instance();
      ExecutableFile Executable;
      Executable.getImage().resize(24, 0xFF);
      Executable.getEntryOffset() = 23;
      Executable.getDynamicImportSymbols() = {
          {Names.intern("counter")},
          {Names.intern("callback")},
          {Names.intern("buffer")},
      };
      Executable.getDynamicImportSymbolRelocations() = {
          {16, 2, 0},
          {8, 0, std::numeric_limits<std::int64_t>::max()},
          {0, 0, std::numeric_limits<std::int64_t>::min()},
      };
      Executable.getExportSymbols() = {
          {Names.intern("counter"), 0},
          {Names.intern("entry"), 23},
          {Names.intern("entry_alias"), 23},
      };
      return Executable;
    }

    void expectExecutableICE(const ExecutableFile &Executable, const char *Pattern)
    {
      const core::DiagnosticEngine Diagnostics;
      EXPECT_DEATH(ExecutableFileValidator::validate(Executable, Diagnostics), Pattern);
    }
  } // namespace

  static_assert(std::is_same_v<decltype(std::declval<ExecutableFile &>().getImage()), std::vector<std::uint8_t> &>);
  static_assert(std::is_same_v<decltype(std::declval<const ExecutableFile &>().getImage()), const std::vector<std::uint8_t> &>);
  static_assert(std::is_same_v<decltype(std::declval<ExecutableFile &>().getEntryOffset()), std::uint64_t &>);
  static_assert(std::is_same_v<decltype(std::declval<const ExecutableFile &>().getEntryOffset()), const std::uint64_t &>);
  static_assert(std::is_same_v<decltype(std::declval<ExecutableFile &>().getDynamicImportSymbols()), std::vector<DynamicImportSymbol> &>);
  static_assert(std::is_same_v<decltype(std::declval<const ExecutableFile &>().getDynamicImportSymbols()), const std::vector<DynamicImportSymbol> &>);
  static_assert(std::is_same_v<decltype(std::declval<ExecutableFile &>().getDynamicImportSymbolRelocations()), std::vector<DynamicImportSymbolRelocation> &>);
  static_assert(std::is_same_v<decltype(std::declval<const ExecutableFile &>().getDynamicImportSymbolRelocations()), const std::vector<DynamicImportSymbolRelocation> &>);
  static_assert(std::is_same_v<decltype(std::declval<ExecutableFile &>().getExportSymbols()), std::vector<ExportSymbol> &>);
  static_assert(std::is_same_v<decltype(std::declval<const ExecutableFile &>().getExportSymbols()), const std::vector<ExportSymbol> &>);
  static_assert(std::is_same_v<decltype(ExecutableFileValidator::validate(std::declval<const ExecutableFile &>(), std::declval<const core::DiagnosticEngine &>())), void>);
  static_assert(noexcept(ExecutableFileValidator::validate(std::declval<const ExecutableFile &>(), std::declval<const core::DiagnosticEngine &>())));
  static_assert(std::is_copy_constructible_v<ExecutableFile> && std::is_copy_assignable_v<ExecutableFile>);
  static_assert(std::is_move_constructible_v<ExecutableFile> && std::is_move_assignable_v<ExecutableFile>);
  static_assert(std::is_same_v<decltype(DynamicImportSymbol{}.Name), core::Name>);
  static_assert(std::is_same_v<decltype(ExportSymbol{}.Name), core::Name>);
  static_assert(std::is_same_v<decltype(ExportSymbol{}.Offset), std::uint64_t>);
  static_assert(DynamicImportSymbolRelocation::ByteSize == 8);

  // Empty images have no entry, and nonempty images reject offsets at their end or at uint64 maximum.
  TEST(VMExecutableFileDeathTest, RequiresTheEntryToIdentifyAnExistingImageByte)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableFile Executable;
    EXPECT_EQ(Executable.byteSize(), 0U);
    EXPECT_EQ(Executable.getEntryOffset(), 0U);
    EXPECT_TRUE(Executable.getDynamicImportSymbols().empty());
    EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
    EXPECT_TRUE(Executable.getExportSymbols().empty());
    expectExecutableICE(Executable, "INK-V0002.*image.*entry offset is outside the image");

    Executable.getImage().push_back(0xFF);
    EXPECT_EQ(Executable.byteSize(), 1U);
    ExecutableFileValidator::validate(Executable, Diagnostics);
    Executable.getEntryOffset() = Executable.byteSize();
    expectExecutableICE(Executable, "INK-V0002.*image.*entry offset is outside the image");
    Executable.getEntryOffset() = std::numeric_limits<std::uint64_t>::max();
    expectExecutableICE(Executable, "INK-V0002.*image.*entry offset is outside the image");
  }

  // Distinct host names remain valid imports even when no address relocation uses them.
  TEST(VMExecutableFileTest, AcceptsDistinctImportsWithoutRelocations)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    ExecutableFile Executable;
    Executable.getImage().push_back(0xFF);
    Executable.getDynamicImportSymbols() = {
        {Names.intern("first")},
        {Names.intern("second")},
    };
    EXPECT_TRUE(Executable.getDynamicImportSymbolRelocations().empty());
    EXPECT_TRUE(Executable.getExportSymbols().empty());
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // A unified symbol may serve multiple unsorted adjacent address fields with arbitrary signed addends.
  TEST(VMExecutableFileTest, AcceptsSharedImportSymbolsAndAdjacentRelocations)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableFile Executable = makeValidExecutableFile();
    ExecutableFileValidator::validate(Executable, Diagnostics);
    EXPECT_EQ(Executable.getEntryOffset(), Executable.byteSize() - 1);
    EXPECT_NE(Executable.getDynamicImportSymbols()[0].Name, Executable.getDynamicImportSymbols()[1].Name);
    EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[0].ImageOffset, 16U);
    EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[1].SymbolIndex, Executable.getDynamicImportSymbolRelocations()[2].SymbolIndex);
    EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[1].Addend, std::numeric_limits<std::int64_t>::max());
    EXPECT_EQ(Executable.getDynamicImportSymbolRelocations()[2].Addend, std::numeric_limits<std::int64_t>::min());
  }

  // Exports accept first-byte and last-byte offsets, multiple names for one address, and names also used by imports.
  TEST(VMExecutableFileTest, AcceptsExportAliasesAndIndependentImportNames)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableFile Executable = makeValidExecutableFile();
    ExecutableFileValidator::validate(Executable, Diagnostics);
    EXPECT_EQ(Executable.getExportSymbols()[0].Name, Executable.getDynamicImportSymbols()[0].Name);
    EXPECT_EQ(Executable.getExportSymbols()[0].Offset, 0U);
    EXPECT_EQ(Executable.getExportSymbols()[1].Offset, Executable.byteSize() - 1);
    EXPECT_EQ(Executable.getExportSymbols()[1].Offset, Executable.getExportSymbols()[2].Offset);
    EXPECT_NE(Executable.getExportSymbols()[1].Name, Executable.getExportSymbols()[2].Name);
    Executable.getDynamicImportSymbolRelocations().clear();
    Executable.getDynamicImportSymbols().clear();
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // Copies independently own image bytes, entry metadata, imports, address relocations, and exports.
  TEST(VMExecutableFileTest, OwnsCopiedContentsIndependently)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    ExecutableFile Original = makeValidExecutableFile();
    const ExecutableFile Copy = Original;
    Original.getImage()[0] = 0;
    Original.getEntryOffset() = 0;
    Original.getDynamicImportSymbols()[0].Name = Names.intern("changed_import");
    Original.getDynamicImportSymbolRelocations()[0].ImageOffset = 0;
    Original.getDynamicImportSymbolRelocations()[0].SymbolIndex = 0;
    Original.getDynamicImportSymbolRelocations()[0].Addend = 99;
    Original.getExportSymbols()[0].Name = Names.intern("changed_export");
    Original.getExportSymbols()[0].Offset = 1;

    EXPECT_EQ(Copy.getImage()[0], 0xFF);
    EXPECT_EQ(Copy.getEntryOffset(), 23U);
    EXPECT_EQ(Copy.getDynamicImportSymbols()[0].Name, Names.intern("counter"));
    EXPECT_EQ(Copy.getDynamicImportSymbolRelocations()[0].ImageOffset, 16U);
    EXPECT_EQ(Copy.getDynamicImportSymbolRelocations()[0].SymbolIndex, 2U);
    EXPECT_EQ(Copy.getDynamicImportSymbolRelocations()[0].Addend, 0);
    EXPECT_EQ(Copy.getExportSymbols()[0].Name, Names.intern("counter"));
    EXPECT_EQ(Copy.getExportSymbols()[0].Offset, 0U);
    ExecutableFileValidator::validate(Copy, Diagnostics);
  }

  // Unified import records reject missing name handles while identifying the original record index.
  TEST(VMExecutableFileDeathTest, RejectsInvalidDynamicImportSymbols)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    ExecutableFile Executable = makeValidExecutableFile();
    Executable.getDynamicImportSymbols()[1].Name = core::NAME_None;
    expectExecutableICE(Executable, "INK-V0002.*dynamic import symbol #1.*name is not registered in the name pool");
    Executable.getDynamicImportSymbols()[1].Name = Names.intern("callback");
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // The import table rejects duplicate host names regardless of how many logical aliases reference them.
  TEST(VMExecutableFileDeathTest, RejectsDuplicateDynamicImportNames)
  {
    ExecutableFile Executable = makeValidExecutableFile();
    Executable.getDynamicImportSymbols().push_back(Executable.getDynamicImportSymbols()[0]);
    expectExecutableICE(Executable, "INK-V0002.*dynamic import symbol #3.*name is duplicated");
  }

  // Export names must be registered and unique even when duplicate names refer to different image offsets.
  TEST(VMExecutableFileDeathTest, RejectsInvalidAndDuplicateExportNames)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    ExecutableFile Executable = makeValidExecutableFile();
    Executable.getExportSymbols()[1].Name = core::NAME_None;
    expectExecutableICE(Executable, "INK-V0002.*export symbol #1.*name is not registered in the name pool");
    Executable.getExportSymbols()[1].Name = Names.intern("counter");
    expectExecutableICE(Executable, "INK-V0002.*export symbol #1.*name is duplicated");
    Executable.getExportSymbols()[1].Name = Names.intern("entry");
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // Exports may name existing image bytes but never the end offset or uint64 maximum.
  TEST(VMExecutableFileDeathTest, RejectsExportOffsetsOutsideTheImage)
  {
    ExecutableFile Executable = makeValidExecutableFile();
    Executable.getExportSymbols()[1].Offset = Executable.byteSize();
    expectExecutableICE(Executable, "INK-V0002.*export symbol #1.*offset is outside the image");
    Executable.getExportSymbols()[1].Offset = std::numeric_limits<std::uint64_t>::max();
    expectExecutableICE(Executable, "INK-V0002.*export symbol #1.*offset is outside the image");
  }

  // Address patches accept exact-width unaligned fields and any import index, but reject overruns, overflow, and absent imports.
  TEST(VMExecutableFileDeathTest, ChecksUnifiedRelocationBoundsAndSymbolIndices)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    ExecutableFile Executable;
    Executable.getImage().resize(16);
    Executable.getDynamicImportSymbols() = {
        {Names.intern("first")},
        {Names.intern("second")},
    };
    Executable.getDynamicImportSymbolRelocations().push_back({8, 1, -1});
    ExecutableFileValidator::validate(Executable, Diagnostics);
    Executable.getDynamicImportSymbolRelocations()[0].ImageOffset = 1;
    ExecutableFileValidator::validate(Executable, Diagnostics);
    Executable.getDynamicImportSymbolRelocations()[0].ImageOffset = 9;
    expectExecutableICE(Executable, "INK-V0002.*dynamic import symbol relocation #0.*address field is outside the image");
    Executable.getDynamicImportSymbolRelocations()[0].ImageOffset = std::numeric_limits<std::uint64_t>::max();
    expectExecutableICE(Executable, "INK-V0002.*dynamic import symbol relocation #0.*address field is outside the image");
    Executable.getDynamicImportSymbolRelocations()[0].ImageOffset = 0;
    Executable.getDynamicImportSymbolRelocations()[0].SymbolIndex = 2;
    expectExecutableICE(Executable, "INK-V0002.*dynamic import symbol relocation #0.*symbol index does not name a dynamic import symbol");
    Executable.getDynamicImportSymbolRelocations()[0].SymbolIndex = std::numeric_limits<std::uint32_t>::max();
    expectExecutableICE(Executable, "INK-V0002.*dynamic import symbol relocation #0.*symbol index does not name a dynamic import symbol");
    Executable.getDynamicImportSymbolRelocations()[0].SymbolIndex = 0;
    Executable.getDynamicImportSymbols().clear();
    expectExecutableICE(Executable, "INK-V0002.*dynamic import symbol relocation #0.*symbol index does not name a dynamic import symbol");
  }

  // Unsorted patch ranges report the larger original index of a conflicting pair, including duplicate patch locations.
  TEST(VMExecutableFileDeathTest, RejectsOverlapsUsingOriginalRelocationIndices)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableFile Executable;
    Executable.getImage().resize(48);
    Executable.getDynamicImportSymbols().push_back({core::NamePool::instance().intern("shared")});
    Executable.getDynamicImportSymbolRelocations() = {
        {16, 0, 0},
        {40, 0, 0},
        {0, 0, 0},
    };
    ExecutableFileValidator::validate(Executable, Diagnostics);

    Executable.getDynamicImportSymbolRelocations()[0].ImageOffset = 7;
    expectExecutableICE(Executable, "INK-V0002.*dynamic import symbol relocation #2.*address field overlaps dynamic import symbol relocation #0");
    Executable.getDynamicImportSymbolRelocations()[0].ImageOffset = 0;
    expectExecutableICE(Executable, "INK-V0002.*dynamic import symbol relocation #2.*address field overlaps dynamic import symbol relocation #0");
  }
} // namespace ink::vm::test
