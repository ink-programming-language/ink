#include "../../../core/support/environment_test_support.h"

#include "ink/core/support/version.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/name/name_pool.h"
#include "ink/backend/vm/object/object_file.h"
#include "ink/backend/vm/object/object_file_validator.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

namespace ink::vm::test
{
  static_assert(std::is_same_v<decltype(ObjectSymbol::Name), core::Name>);
  static_assert(std::is_same_v<decltype(ObjectFileValidator::validate(std::declval<const ObjectFile &>(), std::declval<const core::DiagnosticEngine &>())), void>);
  static_assert(noexcept(ObjectFileValidator::validate(std::declval<const ObjectFile &>(), std::declval<const core::DiagnosticEngine &>())));

  ObjectFile makeValidObjectFile()
  {
    ObjectFile Object;
    Object.getCode().getBytes() = {
        0xFF,
        0xFF,
        0x00,
        0x81,
        0xFF,
        0xFF,
        0x02,
        0x03,
    };
    Object.getConstants().getBytes() = {'o', 'k', 0};
    Object.getGlobalStorage().getAlignment() = 16;
    Object.getGlobalStorage().getByteSize() = 64;
    Object.getGlobalStorage().getInitialBytes() = {1, 2, 3, 4};

    ObjectSymbol Entry;
    Entry.Name = core::NamePool::instance().intern("entry");
    Entry.Linkage = ObjectSymbolLinkage::Export;
    Entry.Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, 0, 4};
    Object.getSymbols().getEntries().push_back(Entry);
    ObjectSymbol Helper;
    Helper.Name = core::NamePool::instance().intern(".Lhelper");
    Helper.Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, 4, 4};
    Object.getSymbols().getEntries().push_back(Helper);
    ObjectSymbol Counter;
    Counter.Name = core::NamePool::instance().intern("counter");
    Counter.Linkage = ObjectSymbolLinkage::Export;
    Counter.Definition = ObjectSymbolDefinition{ObjectSectionKind::GlobalStorage, 0, 4};
    Object.getSymbols().getEntries().push_back(Counter);
    ObjectSymbol Constant;
    Constant.Name = core::NamePool::instance().intern(".Lconstant");
    Constant.Definition = ObjectSymbolDefinition{ObjectSectionKind::Constants, 0, 3};
    Object.getSymbols().getEntries().push_back(Constant);
    ObjectSymbol Buffer;
    Buffer.Name = core::NamePool::instance().intern("buffer");
    Buffer.Definition = ObjectSymbolDefinition{ObjectSectionKind::GlobalStorage, 8, 56};
    Object.getSymbols().getEntries().push_back(Buffer);
    ObjectSymbol ImportedA;
    ImportedA.Name = core::NamePool::instance().intern("external_a");
    ImportedA.Linkage = ObjectSymbolLinkage::Import;
    Object.getSymbols().getEntries().push_back(ImportedA);
    ObjectSymbol ImportedB;
    ImportedB.Name = core::NamePool::instance().intern("external_b");
    ImportedB.Linkage = ObjectSymbolLinkage::Import;
    Object.getSymbols().getEntries().push_back(ImportedB);

    Object.getBaseReferences().getEntries().push_back({0});
    Object.getBaseReferences().getEntries().push_back({2});
    Object.getBaseReferences().getEntries().push_back({5});
    Object.getBaseReferences().getEntries().push_back({6});
    ObjectDebugInfo Debug;
    Debug.getSourceFiles() = {"main.ink", "library.ink"};
    Debug.getLines() = {
        {0, 0, 1, 1},
        {3, 0, 2, 4},
        {4, 1, 1, 1},
        {7, 1, 2, 1},
    };
    Object.getDebugInfo() = std::move(Debug);
    return Object;
  }

  void expectObjectICE(const ObjectFile &Object, const std::string &Pattern)
  {
    const core::DiagnosticEngine Diagnostics;
    EXPECT_DEATH(ObjectFileValidator::validate(Object, Diagnostics), Pattern);
  }

  // A new object has the current header and an empty symbol-reference table without register placeholders.
  TEST(VMObjectFileTest, AcceptsEmptyObjectWithoutReservedBaseSlots)
  {
    const core::DiagnosticEngine Diagnostics;
    core::test::ScopedEnvironmentVariable FormatEnvironment("INK_OBJECT_FORMAT_VERSION");
    core::test::ScopedEnvironmentVariable InstructionSetEnvironment("INK_OBJECT_INSTRUCTION_SET_VERSION");
    ASSERT_TRUE(FormatEnvironment.set(nullptr));
    ASSERT_TRUE(InstructionSetEnvironment.set(nullptr));
    const ObjectFile Object;
    EXPECT_EQ(Object.getHeader().getMagic(), ObjectFileMagic);
    EXPECT_EQ(Object.getHeader().getFormatVersion(), core::Version(2, 0, 0));
    EXPECT_EQ(Object.getHeader().getFormatVersion().toString(), "2.0.0");
    EXPECT_EQ(Object.getHeader().getInstructionSetVersion(), core::Version(1, 1, 0));
    EXPECT_EQ(Object.getHeader().getInstructionSetVersion().toString(), "1.1.0");
    EXPECT_EQ(Object.getHeader().getAddressBitWidth(), 64U);
    EXPECT_EQ(Object.getHeader().getByteOrder(), ObjectByteOrder::Little);
    EXPECT_TRUE(Object.getConstants().getBytes().empty());
    EXPECT_EQ(Object.getGlobalStorage().getByteSize(), 0U);
    EXPECT_TRUE(Object.getGlobalStorage().getInitialBytes().empty());
    EXPECT_TRUE(Object.getBaseReferences().getEntries().empty());
    ObjectFileValidator::validate(Object, Diagnostics);
  }

  // Structural validation accepts opaque code, global debug offsets, both storage regions, exports, and unresolved imports.
  TEST(VMObjectFileTest, AcceptsCompleteObjectWithoutDecodingCodeBytes)
  {
    const core::DiagnosticEngine Diagnostics;
    const ObjectFile Object = makeValidObjectFile();
    ObjectFileValidator::validate(Object, Diagnostics);
    EXPECT_EQ(Object.getConstants().byteSize(), 3U);
    EXPECT_EQ(Object.getGlobalStorage().getByteSize(), 64U);
    EXPECT_EQ(Object.getGlobalStorage().getInitialBytes().size(), 4U);
    EXPECT_FALSE(Object.getSymbols().getEntries()[0].isImport());
    EXPECT_TRUE(Object.getSymbols().getEntries()[5].isImport());
    EXPECT_TRUE(Object.getSymbols().getEntries()[6].isImport());
    ASSERT_EQ(Object.getBaseReferences().getEntries().size(), 4U);
    EXPECT_EQ(Object.getBaseReferences().getEntries()[0].SymbolIndex, 0U);
    EXPECT_EQ(Object.getBaseReferences().getEntries()[2].SymbolIndex, 5U);
    EXPECT_EQ(Object.getBaseReferences().getEntries()[3].SymbolIndex, 6U);
    ASSERT_TRUE(Object.getDebugInfo().has_value());
    EXPECT_EQ(Object.getSymbols().getEntries()[0].Definition->Section, ObjectSectionKind::Code);
    EXPECT_EQ(Object.getSymbols().getEntries()[1].Definition->ByteOffset, 4U);
    EXPECT_EQ(Object.getSymbols().getEntries()[2].Definition->Section, ObjectSectionKind::GlobalStorage);
    EXPECT_EQ(Object.getSymbols().getEntries()[3].Definition->Section, ObjectSectionKind::Constants);
    EXPECT_EQ(Object.getSymbols().getEntries()[4].Definition->Section, ObjectSectionKind::GlobalStorage);
    EXPECT_EQ(Object.getDebugInfo()->getLines()[2].CodeOffset, 4U);
  }

  // Copies keep independent containers and rebindable name handles while sharing interned spellings; debug strings remain independently owned.
  TEST(VMObjectFileTest, OwnsCopiedObjectContentsIndependently)
  {
    const core::DiagnosticEngine Diagnostics;
    ObjectFile Original = makeValidObjectFile();
    const ObjectFile Copy = Original;
    Original.getCode().getBytes()[0] = 0;
    Original.getConstants().getBytes()[0] = 'x';
    Original.getGlobalStorage().getInitialBytes()[0] = 99;
    Original.getSymbols().getEntries()[0].Name = core::NamePool::instance().intern("changed");
    Original.getSymbols().getEntries()[0].Definition->ByteSize = 2;
    Original.getBaseReferences().getEntries()[0].SymbolIndex = 6;
    Original.getDebugInfo()->getSourceFiles()[0] = "changed.ink";
    EXPECT_EQ(Copy.getCode().getBytes()[0], 0xFF);
    EXPECT_EQ(Copy.getConstants().getBytes()[0], 'o');
    EXPECT_EQ(Copy.getGlobalStorage().getInitialBytes()[0], 1U);
    EXPECT_EQ(Copy.getSymbols().getEntries()[0].Name, core::NamePool::instance().intern("entry"));
    EXPECT_EQ(Copy.getSymbols().getEntries()[0].Definition->ByteSize, 4U);
    EXPECT_EQ(Copy.getBaseReferences().getEntries()[0].SymbolIndex, 0U);
    EXPECT_EQ(Copy.getDebugInfo()->getSourceFiles()[0], "main.ink");
    ObjectFileValidator::validate(Copy, Diagnostics);
  }

  // Header validation rejects corrupt magic, unsupported versions and address width, and invalid byte-order values.
  TEST(VMObjectFileDeathTest, RejectsUnsupportedOrCorruptHeaders)
  {
    core::test::ScopedEnvironmentVariable FormatEnvironment("INK_OBJECT_FORMAT_VERSION");
    core::test::ScopedEnvironmentVariable InstructionSetEnvironment("INK_OBJECT_INSTRUCTION_SET_VERSION");
    ASSERT_TRUE(FormatEnvironment.set(nullptr));
    ASSERT_TRUE(InstructionSetEnvironment.set(nullptr));
    ObjectFile Object = makeValidObjectFile();
    Object.getHeader().getMagic()[0] = 'X';
    expectObjectICE(Object, "INK-V0003.*header.*magic is invalid");
    const core::Version InvalidFormatVersions[] = {
        core::Version(1, 0, 0),
        core::Version(3, 0, 0),
        core::Version(2, 1, 0),
        core::Version(2, 0, 1),
    };
    for (const auto &Version : InvalidFormatVersions)
    {
      SCOPED_TRACE(Version.toString());
      Object = makeValidObjectFile();
      Object.getHeader().getFormatVersion() = Version;
      expectObjectICE(Object, "INK-V0003.*header.*object format version is unsupported");
    }
    const core::Version InvalidInstructionSetVersions[] = {
        core::Version(2, 0, 0),
        core::Version(1, 0, 0),
        core::Version(1, 2, 0),
        core::Version(1, 1, 1),
    };
    for (const auto &Version : InvalidInstructionSetVersions)
    {
      SCOPED_TRACE(Version.toString());
      Object = makeValidObjectFile();
      Object.getHeader().getInstructionSetVersion() = Version;
      expectObjectICE(Object, "INK-V0003.*header.*instruction-set version is unsupported");
    }
    Object = makeValidObjectFile();
    Object.getHeader().getAddressBitWidth() = 32;
    expectObjectICE(Object, "INK-V0003.*header.*address width must be 64 bits");
    Object = makeValidObjectFile();
    Object.getHeader().getByteOrder() = static_cast<ObjectByteOrder>(255);
    expectObjectICE(Object, "INK-V0003.*header.*byte order is invalid");
  }

  // Header construction reads distinct configured versions and validation rejects either header version when it differs.
  TEST(VMObjectFileDeathTest, ConstructsAndValidatesHeadersUsingConfiguredVersions)
  {
    const core::DiagnosticEngine Diagnostics;
    core::test::ScopedEnvironmentVariable FormatEnvironment("INK_OBJECT_FORMAT_VERSION");
    core::test::ScopedEnvironmentVariable InstructionSetEnvironment("INK_OBJECT_INSTRUCTION_SET_VERSION");
    ASSERT_TRUE(FormatEnvironment.set("3.2.1"));
    ASSERT_TRUE(InstructionSetEnvironment.set("7.8.9"));
    ObjectFile Object = makeValidObjectFile();
    EXPECT_EQ(Object.getHeader().getFormatVersion(), core::Version(3, 2, 1));
    EXPECT_EQ(Object.getHeader().getInstructionSetVersion(), core::Version(7, 8, 9));
    ObjectFileValidator::validate(Object, Diagnostics);
    Object.getHeader().getFormatVersion() = core::Version(3, 2, 2);
    expectObjectICE(Object, "INK-V0003.*header.*object format version is unsupported");
    Object.getHeader().getFormatVersion() = core::Version(3, 2, 1);
    Object.getHeader().getInstructionSetVersion() = core::Version(7, 9, 9);
    expectObjectICE(Object, "INK-V0003.*header.*instruction-set version is unsupported");
  }

  // Code symbol extents reject unsigned wraparound while allowing empty end ranges and overlapping aliases.
  TEST(VMObjectFileDeathTest, ChecksCodeSymbolRangesWithoutUnsignedWraparound)
  {
    const core::DiagnosticEngine Diagnostics;
    ObjectFile Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[1].Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, std::numeric_limits<std::uint64_t>::max(), 4};
    expectObjectICE(Object, "INK-V0003.*symbol #1.*definition range is outside its section");
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[1].Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, 4, std::numeric_limits<std::uint64_t>::max()};
    expectObjectICE(Object, "INK-V0003.*symbol #1.*definition range is outside its section");
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[1].Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, Object.getCode().byteSize(), 0};
    ObjectFileValidator::validate(Object, Diagnostics);
    Object.getSymbols().getEntries()[1].Definition->ByteSize = 1;
    expectObjectICE(Object, "INK-V0003.*symbol #1.*definition range is outside its section");
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[1].Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, 2, 4};
    ObjectFileValidator::validate(Object, Diagnostics);
  }

  // Constants and global storage independently require nonzero power-of-two alignments.
  TEST(VMObjectFileDeathTest, RejectsInvalidStorageAlignments)
  {
    const std::uint64_t InvalidAlignments[] = {0, 3};
    for (const auto Alignment : InvalidAlignments)
    {
      SCOPED_TRACE(Alignment);
      ObjectFile Object = makeValidObjectFile();
      Object.getConstants().getAlignment() = Alignment;
      expectObjectICE(Object, "INK-V0003.*constants.*alignment must be a nonzero power of two");
      Object = makeValidObjectFile();
      Object.getGlobalStorage().getAlignment() = Alignment;
      expectObjectICE(Object, "INK-V0003.*global storage.*alignment must be a nonzero power of two");
    }
  }

  // Global storage accepts no initializer or a full-capacity initializer, but rejects a prefix beyond its byte size.
  TEST(VMObjectFileDeathTest, ChecksGlobalInitializationPrefixAgainstCapacity)
  {
    const core::DiagnosticEngine Diagnostics;
    ObjectFile Object = makeValidObjectFile();
    Object.getGlobalStorage().getInitialBytes().clear();
    EXPECT_EQ(Object.getGlobalStorage().getByteSize(), 64U);
    EXPECT_TRUE(Object.getGlobalStorage().getInitialBytes().empty());
    ObjectFileValidator::validate(Object, Diagnostics);
    Object.getGlobalStorage().getInitialBytes().assign(64, 0xAB);
    ObjectFileValidator::validate(Object, Diagnostics);
    Object.getGlobalStorage().getInitialBytes().push_back(0xCD);
    expectObjectICE(Object, "INK-V0003.*global storage.*initializer exceeds the declared storage size");
  }

  // Symbol definitions reject unknown section categories rather than treating them as code or another storage region.
  TEST(VMObjectFileDeathTest, RejectsUnknownSymbolSections)
  {
    ObjectFile Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[0].Definition->Section = static_cast<ObjectSectionKind>(255);
    expectObjectICE(Object, "INK-V0003.*symbol #0.*definition section is invalid");
  }

  // Constants and global symbol ranges reject overrun and overflow while allowing empty symbols at each region's end.
  TEST(VMObjectFileDeathTest, ChecksSymbolRangesAgainstConstantAndGlobalStorage)
  {
    const core::DiagnosticEngine Diagnostics;
    ObjectFile Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[3].Definition = ObjectSymbolDefinition{ObjectSectionKind::Constants, 3, 1};
    expectObjectICE(Object, "INK-V0003.*symbol #3.*definition range is outside its section");
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[3].Definition = ObjectSymbolDefinition{ObjectSectionKind::Constants, 3, 0};
    ObjectFileValidator::validate(Object, Diagnostics);
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[3].Definition = ObjectSymbolDefinition{ObjectSectionKind::Constants, 2, std::numeric_limits<std::uint64_t>::max()};
    expectObjectICE(Object, "INK-V0003.*symbol #3.*definition range is outside its section");
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[2].Definition = ObjectSymbolDefinition{ObjectSectionKind::GlobalStorage, 64, 1};
    expectObjectICE(Object, "INK-V0003.*symbol #2.*definition range is outside its section");
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[2].Definition = ObjectSymbolDefinition{ObjectSectionKind::GlobalStorage, 64, 0};
    ObjectFileValidator::validate(Object, Diagnostics);
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[2].Definition = ObjectSymbolDefinition{ObjectSectionKind::GlobalStorage, 2, std::numeric_limits<std::uint64_t>::max()};
    expectObjectICE(Object, "INK-V0003.*symbol #2.*definition range is outside its section");
    Object = makeValidObjectFile();
    Object.getGlobalStorage().getByteSize() = std::numeric_limits<std::uint64_t>::max();
    Object.getSymbols().getEntries()[4].Definition = ObjectSymbolDefinition{ObjectSectionKind::GlobalStorage, std::numeric_limits<std::uint64_t>::max() - 3, 4};
    expectObjectICE(Object, "INK-V0003.*symbol #4.*definition range is outside its section");
    Object.getSymbols().getEntries()[4].Definition = ObjectSymbolDefinition{ObjectSectionKind::GlobalStorage, std::numeric_limits<std::uint64_t>::max() - 3, 3};
    ObjectFileValidator::validate(Object, Diagnostics);
  }

  // Explicit linkage rejects contradictory definitions without changing the declared import intent.
  TEST(VMObjectFileDeathTest, RejectsContradictorySymbolLinkageMetadata)
  {
    ObjectFile Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[5].Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, 0, 4};
    EXPECT_TRUE(Object.getSymbols().getEntries()[5].isImport());
    expectObjectICE(Object, "INK-V0003.*symbol #5.*import must not have a definition");
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[0].Definition.reset();
    EXPECT_FALSE(Object.getSymbols().getEntries()[0].isImport());
    expectObjectICE(Object, "INK-V0003.*symbol #0.*local or exported symbol requires a definition");
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[1].Definition.reset();
    EXPECT_FALSE(Object.getSymbols().getEntries()[1].isImport());
    expectObjectICE(Object, "INK-V0003.*symbol #1.*local or exported symbol requires a definition");
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[6].Linkage = static_cast<ObjectSymbolLinkage>(255);
    expectObjectICE(Object, "INK-V0003.*symbol #6.*linkage is invalid");
  }

  // Default and empty-interned symbol names remain NAME_None and are rejected as unnamed declarations.
  TEST(VMObjectFileDeathTest, RejectsUnnamedSymbolHandles)
  {
    const ObjectSymbol Unnamed;
    EXPECT_EQ(Unnamed.Name, core::NAME_None);
    ObjectFile Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[0].Name = Unnamed.Name;
    expectObjectICE(Object, "INK-V0003.*symbol #0.*name is not registered in the name pool");
    Object.getSymbols().getEntries()[0].Name = core::NamePool::instance().intern("");
    expectObjectICE(Object, "INK-V0003.*symbol #0.*name is not registered in the name pool");
  }

  // Equal external spellings containing NUL bytes from separate buffers share one identity and still trigger duplicate-symbol validation.
  TEST(VMObjectFileDeathTest, DetectsInternedExternalNamesAcrossIndependentBuffers)
  {
    std::string FirstBuffer("object_external\0identity", sizeof("object_external\0identity") - 1);
    const std::string SecondBuffer(FirstBuffer.data(), FirstBuffer.size());
    ASSERT_NE(FirstBuffer.data(), SecondBuffer.data());
    core::NamePool &Names = core::NamePool::instance();
    ObjectFile Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[5].Name = Names.intern(FirstBuffer);
    Object.getSymbols().getEntries()[6].Name = Names.intern(SecondBuffer);
    EXPECT_EQ(Object.getSymbols().getEntries()[5].Name, Object.getSymbols().getEntries()[6].Name);
    FirstBuffer[0] = 'X';
    EXPECT_EQ(Names.text(Object.getSymbols().getEntries()[5].Name), SecondBuffer);
    expectObjectICE(Object, "INK-V0003.*symbol #6.*external name is duplicated");
  }

  // Local names may match imports or exports while imported and exported symbols share one external namespace.
  TEST(VMObjectFileDeathTest, AllowsLocalNamesWithoutExternalConflicts)
  {
    const core::DiagnosticEngine Diagnostics;
    ObjectFile Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[1].Name = core::NamePool::instance().intern("entry");
    Object.getSymbols().getEntries()[3].Name = core::NamePool::instance().intern("entry");
    Object.getSymbols().getEntries()[4].Name = core::NamePool::instance().intern("external_a");
    ObjectFileValidator::validate(Object, Diagnostics);
    Object.getSymbols().getEntries()[6].Name = core::NamePool::instance().intern("entry");
    expectObjectICE(Object, "INK-V0003.*symbol #6.*external name is duplicated");
  }

  // External name uniqueness rejects import/import, export/export, and export/import collisions by name alone.
  TEST(VMObjectFileDeathTest, RejectsDuplicateImportAndExportNames)
  {
    ObjectFile Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[6].Name = core::NamePool::instance().intern("external_a");
    expectObjectICE(Object, "INK-V0003.*symbol #6.*external name is duplicated");
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[2].Name = core::NamePool::instance().intern("entry");
    expectObjectICE(Object, "INK-V0003.*symbol #2.*external name is duplicated");
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[0].Name = core::NamePool::instance().intern("external_a");
    expectObjectICE(Object, "INK-V0003.*symbol #5.*external name is duplicated");
  }

  // A symbol table may have no references, and slot zero can directly name a definition or an unresolved import.
  TEST(VMObjectFileTest, AcceptsEmptyAndZeroBasedSymbolReferences)
  {
    const core::DiagnosticEngine Diagnostics;
    ObjectFile Object = makeValidObjectFile();
    Object.getBaseReferences().getEntries().clear();
    ObjectFileValidator::validate(Object, Diagnostics);
    Object.getBaseReferences().getEntries().push_back({0});
    ObjectFileValidator::validate(Object, Diagnostics);
    Object.getBaseReferences().getEntries()[0].SymbolIndex = 5;
    ObjectFileValidator::validate(Object, Diagnostics);
    EXPECT_TRUE(Object.getSymbols().getEntries()[Object.getBaseReferences().getEntries()[0].SymbolIndex].isImport());
  }

  // Every reference, including slot zero, must name an existing symbol rather than a register or an absent symbol.
  TEST(VMObjectFileDeathTest, RejectsDanglingBaseReferencesAtEverySlot)
  {
    const ObjectFile Valid = makeValidObjectFile();
    for (std::uint64_t Index = 0; Index < Valid.getBaseReferences().getEntries().size(); ++Index)
    {
      SCOPED_TRACE(Index);
      ObjectFile Object = Valid;
      Object.getBaseReferences().getEntries()[Index].SymbolIndex = static_cast<std::uint32_t>(Object.getSymbols().getEntries().size());
      expectObjectICE(Object, "INK-V0003.*base reference #" + std::to_string(Index) + ".*symbol index is outside the symbol table");
      Object.getBaseReferences().getEntries()[Index].SymbolIndex = std::numeric_limits<std::uint32_t>::max();
      expectObjectICE(Object, "INK-V0003.*base reference #" + std::to_string(Index) + ".*symbol index is outside the symbol table");
    }
    ObjectFile Empty;
    Empty.getBaseReferences().getEntries().push_back({0});
    expectObjectICE(Empty, "INK-V0003.*base reference #0.*symbol index is outside the symbol table");
  }

  // Debug records use global code offsets and reject dangling file indices, code-end offsets, and zero-based locations.
  TEST(VMObjectFileDeathTest, RejectsInvalidGlobalDebugLocations)
  {
    ObjectFile Object = makeValidObjectFile();
    Object.getDebugInfo()->getLines()[1].FileIndex = 2;
    expectObjectICE(Object, "INK-V0003.*debug line #1.*source file index is outside the source file table");
    Object = makeValidObjectFile();
    Object.getDebugInfo()->getLines()[1].CodeOffset = 8;
    expectObjectICE(Object, "INK-V0003.*debug line #1.*code offset is outside the code section");
    Object = makeValidObjectFile();
    Object.getDebugInfo()->getLines()[1].CodeOffset = std::numeric_limits<std::uint64_t>::max();
    expectObjectICE(Object, "INK-V0003.*debug line #1.*code offset is outside the code section");
    Object = makeValidObjectFile();
    Object.getDebugInfo()->getLines()[1].Line = 0;
    expectObjectICE(Object, "INK-V0003.*debug line #1.*line must be one-based");
    Object = makeValidObjectFile();
    Object.getDebugInfo()->getLines()[1].Column = 0;
    expectObjectICE(Object, "INK-V0003.*debug line #1.*column must be one-based");
  }
} // namespace ink::vm::test
