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
      Executable.getNativeFunctions() = {
          {NativeSymbolProvider::Native, Names.intern("counter"), NativeCallKind::C},
          {NativeSymbolProvider::SystemApi, Names.intern("counter"), NativeCallKind::InkNative},
      };
      Executable.getNativeData() = {
          {Names.intern("counter")},
          {Names.intern("second_counter")},
          {Names.intern("buffer")},
      };
      Executable.getNativeDataRelocations() = {
          {16, 2, 0},
          {8, 1, std::numeric_limits<std::int64_t>::max()},
          {0, 0, std::numeric_limits<std::int64_t>::min()},
      };
      return Executable;
    }

    void expectExecutableICE(const ExecutableFile &Executable, const char *Pattern)
    {
      const core::DiagnosticEngine Diagnostics;
      EXPECT_DEATH(ExecutableFileValidator::validate(Executable, Diagnostics), Pattern);
    }
  } // namespace

  template <typename ImportType>
  concept HasProvider = requires(ImportType Import)
  {
    Import.Provider;
  };

  static_assert(std::is_same_v<decltype(std::declval<ExecutableFile &>().getImage()), std::vector<std::uint8_t> &>);
  static_assert(std::is_same_v<decltype(std::declval<const ExecutableFile &>().getImage()), const std::vector<std::uint8_t> &>);
  static_assert(std::is_same_v<decltype(std::declval<ExecutableFile &>().getEntryOffset()), std::uint64_t &>);
  static_assert(std::is_same_v<decltype(std::declval<const ExecutableFile &>().getEntryOffset()), const std::uint64_t &>);
  static_assert(std::is_same_v<decltype(std::declval<ExecutableFile &>().getNativeFunctions()), std::vector<NativeFunctionImport> &>);
  static_assert(std::is_same_v<decltype(std::declval<const ExecutableFile &>().getNativeFunctions()), const std::vector<NativeFunctionImport> &>);
  static_assert(std::is_same_v<decltype(std::declval<ExecutableFile &>().getNativeData()), std::vector<NativeDataImport> &>);
  static_assert(std::is_same_v<decltype(std::declval<const ExecutableFile &>().getNativeData()), const std::vector<NativeDataImport> &>);
  static_assert(std::is_same_v<decltype(std::declval<ExecutableFile &>().getNativeDataRelocations()), std::vector<NativeDataRelocation> &>);
  static_assert(std::is_same_v<decltype(std::declval<const ExecutableFile &>().getNativeDataRelocations()), const std::vector<NativeDataRelocation> &>);
  static_assert(std::is_same_v<decltype(ExecutableFileValidator::validate(std::declval<const ExecutableFile &>(), std::declval<const core::DiagnosticEngine &>())), void>);
  static_assert(noexcept(ExecutableFileValidator::validate(std::declval<const ExecutableFile &>(), std::declval<const core::DiagnosticEngine &>())));
  static_assert(std::is_copy_constructible_v<ExecutableFile> && std::is_copy_assignable_v<ExecutableFile>);
  static_assert(std::is_move_constructible_v<ExecutableFile> && std::is_move_assignable_v<ExecutableFile>);
  static_assert(std::is_same_v<decltype(NativeFunctionImport{}.Name), core::Name>);
  static_assert(std::is_same_v<decltype(NativeDataImport{}.Name), core::Name>);
  static_assert(HasProvider<NativeFunctionImport> && !HasProvider<NativeDataImport>);
  static_assert(NativeDataRelocation::ByteSize == 8);

  // Empty images have no entry, and nonempty images reject offsets at their end or at uint64 maximum.
  TEST(VMExecutableFileDeathTest, RequiresTheEntryToIdentifyAnExistingImageByte)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableFile Executable;
    EXPECT_EQ(Executable.byteSize(), 0U);
    EXPECT_EQ(Executable.getEntryOffset(), 0U);
    EXPECT_TRUE(Executable.getNativeFunctions().empty());
    EXPECT_TRUE(Executable.getNativeData().empty());
    EXPECT_TRUE(Executable.getNativeDataRelocations().empty());
    expectExecutableICE(Executable, "INK-V0002.*image.*entry offset is outside the image");

    Executable.getImage().push_back(0xFF);
    EXPECT_EQ(Executable.byteSize(), 1U);
    ExecutableFileValidator::validate(Executable, Diagnostics);
    Executable.getEntryOffset() = Executable.byteSize();
    expectExecutableICE(Executable, "INK-V0002.*image.*entry offset is outside the image");
    Executable.getEntryOffset() = std::numeric_limits<std::uint64_t>::max();
    expectExecutableICE(Executable, "INK-V0002.*image.*entry offset is outside the image");
  }

  // Function imports need no data relocations, and either provider category may use C or InkNative calls.
  TEST(VMExecutableFileTest, AcceptsFunctionImportsWithIndependentProvidersAndCallKinds)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    ExecutableFile Executable;
    Executable.getImage().push_back(0xFF);
    Executable.getNativeFunctions() = {
        {NativeSymbolProvider::Native, Names.intern("c"), NativeCallKind::C},
        {NativeSymbolProvider::Native, Names.intern("ink"), NativeCallKind::InkNative},
        {NativeSymbolProvider::SystemApi, Names.intern("c"), NativeCallKind::C},
        {NativeSymbolProvider::SystemApi, Names.intern("ink"), NativeCallKind::InkNative},
    };
    EXPECT_TRUE(Executable.getNativeData().empty());
    EXPECT_TRUE(Executable.getNativeDataRelocations().empty());
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // Opaque images accept a last-byte entry, reverse-ordered adjacent data patches, and the same name in both import tables.
  TEST(VMExecutableFileTest, AcceptsIndependentImportTablesAndAdjacentDataRelocations)
  {
    const core::DiagnosticEngine Diagnostics;
    const ExecutableFile Executable = makeValidExecutableFile();
    ExecutableFileValidator::validate(Executable, Diagnostics);
    EXPECT_EQ(Executable.getEntryOffset(), Executable.byteSize() - 1);
    EXPECT_EQ(Executable.getNativeFunctions()[0].Name, Executable.getNativeData()[0].Name);
    EXPECT_NE(Executable.getNativeData()[0].Name, Executable.getNativeData()[1].Name);
    EXPECT_EQ(Executable.getNativeDataRelocations()[0].ImageOffset, 16U);
    EXPECT_EQ(Executable.getNativeDataRelocations()[1].Addend, std::numeric_limits<std::int64_t>::max());
    EXPECT_EQ(Executable.getNativeDataRelocations()[2].Addend, std::numeric_limits<std::int64_t>::min());
  }

  // Copies independently own image bytes, entry metadata, both import tables and their name handles, and data relocations.
  TEST(VMExecutableFileTest, OwnsCopiedContentsIndependently)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    ExecutableFile Original = makeValidExecutableFile();
    const ExecutableFile Copy = Original;
    Original.getImage()[0] = 0;
    Original.getEntryOffset() = 0;
    Original.getNativeFunctions()[0].Provider = NativeSymbolProvider::SystemApi;
    Original.getNativeFunctions()[0].Name = Names.intern("changed_function");
    Original.getNativeFunctions()[0].CallKind = NativeCallKind::InkNative;
    Original.getNativeData()[0].Name = Names.intern("changed_data");
    Original.getNativeDataRelocations()[0].ImageOffset = 0;
    Original.getNativeDataRelocations()[0].SymbolIndex = 0;
    Original.getNativeDataRelocations()[0].Addend = 99;

    EXPECT_EQ(Copy.getImage()[0], 0xFF);
    EXPECT_EQ(Copy.getEntryOffset(), 23U);
    EXPECT_EQ(Copy.getNativeFunctions()[0].Provider, NativeSymbolProvider::Native);
    EXPECT_EQ(Copy.getNativeFunctions()[0].Name, Names.intern("counter"));
    EXPECT_EQ(Copy.getNativeFunctions()[0].CallKind, NativeCallKind::C);
    EXPECT_EQ(Copy.getNativeData()[0].Name, Names.intern("counter"));
    EXPECT_EQ(Copy.getNativeDataRelocations()[0].ImageOffset, 16U);
    EXPECT_EQ(Copy.getNativeDataRelocations()[0].SymbolIndex, 2U);
    EXPECT_EQ(Copy.getNativeDataRelocations()[0].Addend, 0);
    ExecutableFileValidator::validate(Copy, Diagnostics);
  }

  // Function imports reject missing name handles, unknown providers, and unknown call kinds with function-table indices.
  TEST(VMExecutableFileDeathTest, RejectsInvalidNativeFunctionImports)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    ExecutableFile Executable = makeValidExecutableFile();
    Executable.getNativeFunctions()[1].Name = core::NAME_None;
    expectExecutableICE(Executable, "INK-V0002.*native function import #1.*name is not registered in the name pool");
    Executable.getNativeFunctions()[1].Name = Names.intern("counter");
    Executable.getNativeFunctions()[1].Provider = static_cast<NativeSymbolProvider>(255);
    expectExecutableICE(Executable, "INK-V0002.*native function import #1.*provider is invalid");
    Executable.getNativeFunctions()[1].Provider = NativeSymbolProvider::SystemApi;
    Executable.getNativeFunctions()[1].CallKind = static_cast<NativeCallKind>(255);
    expectExecutableICE(Executable, "INK-V0002.*native function import #1.*call kind is invalid");
    Executable.getNativeFunctions()[1].CallKind = NativeCallKind::InkNative;
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // Data imports reject missing name handles with data-table indices independent of the function table.
  TEST(VMExecutableFileDeathTest, RejectsInvalidNativeDataImports)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableFile Executable = makeValidExecutableFile();
    Executable.getNativeData()[2].Name = core::NAME_None;
    expectExecutableICE(Executable, "INK-V0002.*native data import #2.*name is not registered in the name pool");
    Executable.getNativeData()[2].Name = core::NamePool::instance().intern("buffer");
    ExecutableFileValidator::validate(Executable, Diagnostics);
  }

  // Functions reject duplicate provider/name pairs across call kinds, while data imports reject any repeated name.
  TEST(VMExecutableFileDeathTest, RejectsDuplicateIdentitiesWithinEachImportTable)
  {
    core::NamePool &Names = core::NamePool::instance();
    ExecutableFile Executable = makeValidExecutableFile();
    Executable.getNativeFunctions().push_back({NativeSymbolProvider::Native, Names.intern("counter"), NativeCallKind::InkNative});
    expectExecutableICE(Executable, "INK-V0002.*native function import #2.*provider and name are duplicated");
    Executable.getNativeFunctions().pop_back();
    Executable.getNativeData().push_back({Names.intern("counter")});
    expectExecutableICE(Executable, "INK-V0002.*native data import #3.*name is duplicated");
  }

  // Data patches accept unaligned exact-width fields but reject overruns, overflow, and indices found only in the function table.
  TEST(VMExecutableFileDeathTest, ChecksDataRelocationBoundsAndDataOnlyIndices)
  {
    const core::DiagnosticEngine Diagnostics;
    core::NamePool &Names = core::NamePool::instance();
    ExecutableFile Executable;
    Executable.getImage().resize(16);
    Executable.getNativeFunctions() = {
        {NativeSymbolProvider::Native, Names.intern("first"), NativeCallKind::C},
        {NativeSymbolProvider::Native, Names.intern("second"), NativeCallKind::InkNative},
    };
    Executable.getNativeData().push_back({Names.intern("data")});
    Executable.getNativeDataRelocations().push_back({8, 0, -1});
    ExecutableFileValidator::validate(Executable, Diagnostics);
    Executable.getNativeDataRelocations()[0].ImageOffset = 1;
    ExecutableFileValidator::validate(Executable, Diagnostics);
    Executable.getNativeDataRelocations()[0].ImageOffset = 9;
    expectExecutableICE(Executable, "INK-V0002.*native data relocation #0.*address field is outside the image");
    Executable.getNativeDataRelocations()[0].ImageOffset = std::numeric_limits<std::uint64_t>::max();
    expectExecutableICE(Executable, "INK-V0002.*native data relocation #0.*address field is outside the image");
    Executable.getNativeDataRelocations()[0].ImageOffset = 0;
    Executable.getNativeDataRelocations()[0].SymbolIndex = 1;
    expectExecutableICE(Executable, "INK-V0002.*native data relocation #0.*symbol index does not name a native data import");
    Executable.getNativeDataRelocations()[0].SymbolIndex = std::numeric_limits<std::uint32_t>::max();
    expectExecutableICE(Executable, "INK-V0002.*native data relocation #0.*symbol index does not name a native data import");
    Executable.getNativeDataRelocations()[0].SymbolIndex = 0;
    Executable.getNativeData().clear();
    expectExecutableICE(Executable, "INK-V0002.*native data relocation #0.*symbol index does not name a native data import");
  }

  // Unsorted data patch ranges report the larger original index of a conflicting pair, including duplicate patch locations.
  TEST(VMExecutableFileDeathTest, RejectsOverlapsUsingOriginalRelocationIndices)
  {
    const core::DiagnosticEngine Diagnostics;
    ExecutableFile Executable;
    Executable.getImage().resize(48);
    Executable.getNativeData().push_back({core::NamePool::instance().intern("data")});
    Executable.getNativeDataRelocations() = {
        {16, 0, 0},
        {40, 0, 0},
        {0, 0, 0},
    };
    ExecutableFileValidator::validate(Executable, Diagnostics);

    Executable.getNativeDataRelocations()[0].ImageOffset = 7;
    expectExecutableICE(Executable, "INK-V0002.*native data relocation #2.*address field overlaps native data relocation #0");
    Executable.getNativeDataRelocations()[0].ImageOffset = 0;
    expectExecutableICE(Executable, "INK-V0002.*native data relocation #2.*address field overlaps native data relocation #0");
  }
} // namespace ink::vm::test
