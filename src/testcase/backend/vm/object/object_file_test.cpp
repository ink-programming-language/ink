#include "../../../core/support/environment_test_support.h"

#include "ink/core/support/version.h"
#include "ink/backend/vm/object/object_file.h"
#include "ink/backend/vm/object/object_file_validator.h"

#include <gtest/gtest.h>

#include <cstdint>
#include <limits>
#include <utility>

namespace ink::vm::test
{
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
    Entry.Name = "entry";
    Entry.Linkage = ObjectSymbolLinkage::Export;
    Entry.Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, 0, 4};
    Object.getSymbols().getEntries().push_back(Entry);
    ObjectSymbol Helper;
    Helper.Name = ".Lhelper";
    Helper.Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, 4, 4};
    Object.getSymbols().getEntries().push_back(Helper);
    ObjectSymbol Counter;
    Counter.Name = "counter";
    Counter.Linkage = ObjectSymbolLinkage::Export;
    Counter.Definition = ObjectSymbolDefinition{ObjectSectionKind::GlobalStorage, 0, 4};
    Object.getSymbols().getEntries().push_back(Counter);
    ObjectSymbol Constant;
    Constant.Name = ".Lconstant";
    Constant.Definition = ObjectSymbolDefinition{ObjectSectionKind::Constants, 0, 3};
    Object.getSymbols().getEntries().push_back(Constant);
    ObjectSymbol Buffer;
    Buffer.Name = "buffer";
    Buffer.Definition = ObjectSymbolDefinition{ObjectSectionKind::GlobalStorage, 8, 56};
    Object.getSymbols().getEntries().push_back(Buffer);
    ObjectSymbol ImportedA;
    ImportedA.Name = "external_a";
    ImportedA.Linkage = ObjectSymbolLinkage::Import;
    Object.getSymbols().getEntries().push_back(ImportedA);
    ObjectSymbol ImportedB;
    ImportedB.Name = "external_b";
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

  void expectObjectError(const ObjectFile &Object, ObjectValidationErrorKind Kind, std::uint64_t RecordIndex)
  {
    const auto Error = ObjectFileValidator::validate(Object);
    ASSERT_TRUE(Error.has_value());
    EXPECT_EQ(Error->Kind, Kind);
    EXPECT_EQ(Error->RecordIndex, RecordIndex);
  }

  // A new object has the current header and an empty symbol-reference table without register placeholders.
  TEST(VMObjectFileTest, AcceptsEmptyObjectWithoutReservedBaseSlots)
  {
    core::test::ScopedEnvironmentVariable FormatEnvironment("INK_OBJECT_FORMAT_VERSION");
    core::test::ScopedEnvironmentVariable InstructionSetEnvironment("INK_OBJECT_INSTRUCTION_SET_VERSION");
    ASSERT_TRUE(FormatEnvironment.set(nullptr));
    ASSERT_TRUE(InstructionSetEnvironment.set(nullptr));
    const ObjectFile Object;
    EXPECT_EQ(Object.getHeader().getMagic(), ObjectFileMagic);
    EXPECT_EQ(Object.getHeader().getFormatVersion(), core::Version(2, 0, 0));
    EXPECT_EQ(Object.getHeader().getFormatVersion().toString(), "2.0.0");
    EXPECT_EQ(Object.getHeader().getInstructionSetVersion(), core::Version(1, 0, 0));
    EXPECT_EQ(Object.getHeader().getInstructionSetVersion().toString(), "1.0.0");
    EXPECT_EQ(Object.getHeader().getAddressBitWidth(), 64U);
    EXPECT_EQ(Object.getHeader().getByteOrder(), ObjectByteOrder::Little);
    EXPECT_TRUE(Object.getConstants().getBytes().empty());
    EXPECT_EQ(Object.getGlobalStorage().getByteSize(), 0U);
    EXPECT_TRUE(Object.getGlobalStorage().getInitialBytes().empty());
    EXPECT_TRUE(Object.getBaseReferences().getEntries().empty());
    EXPECT_FALSE(ObjectFileValidator::validate(Object).has_value());
  }

  // Structural validation accepts opaque code, global debug offsets, both storage regions, exports, and unresolved imports.
  TEST(VMObjectFileTest, AcceptsCompleteObjectWithoutDecodingCodeBytes)
  {
    const ObjectFile Object = makeValidObjectFile();
    EXPECT_FALSE(ObjectFileValidator::validate(Object).has_value());
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

  // Copying preserves independent code, constants, global initializers, symbol definitions, base references, and debug strings.
  TEST(VMObjectFileTest, OwnsCopiedObjectContentsIndependently)
  {
    ObjectFile Original = makeValidObjectFile();
    const ObjectFile Copy = Original;
    Original.getCode().getBytes()[0] = 0;
    Original.getConstants().getBytes()[0] = 'x';
    Original.getGlobalStorage().getInitialBytes()[0] = 99;
    Original.getSymbols().getEntries()[0].Name = "changed";
    Original.getSymbols().getEntries()[0].Definition->ByteSize = 2;
    Original.getBaseReferences().getEntries()[0].SymbolIndex = 6;
    Original.getDebugInfo()->getSourceFiles()[0] = "changed.ink";
    EXPECT_EQ(Copy.getCode().getBytes()[0], 0xFF);
    EXPECT_EQ(Copy.getConstants().getBytes()[0], 'o');
    EXPECT_EQ(Copy.getGlobalStorage().getInitialBytes()[0], 1U);
    EXPECT_EQ(Copy.getSymbols().getEntries()[0].Name, "entry");
    EXPECT_EQ(Copy.getSymbols().getEntries()[0].Definition->ByteSize, 4U);
    EXPECT_EQ(Copy.getBaseReferences().getEntries()[0].SymbolIndex, 0U);
    EXPECT_EQ(Copy.getDebugInfo()->getSourceFiles()[0], "main.ink");
    EXPECT_FALSE(ObjectFileValidator::validate(Copy).has_value());
  }

  // Header validation rejects corrupt magic, unsupported versions and address width, and invalid byte-order values.
  TEST(VMObjectFileTest, RejectsUnsupportedOrCorruptHeaders)
  {
    core::test::ScopedEnvironmentVariable FormatEnvironment("INK_OBJECT_FORMAT_VERSION");
    core::test::ScopedEnvironmentVariable InstructionSetEnvironment("INK_OBJECT_INSTRUCTION_SET_VERSION");
    ASSERT_TRUE(FormatEnvironment.set(nullptr));
    ASSERT_TRUE(InstructionSetEnvironment.set(nullptr));
    ObjectFile Object = makeValidObjectFile();
    Object.getHeader().getMagic()[0] = 'X';
    expectObjectError(Object, ObjectValidationErrorKind::InvalidHeader, 0);
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
      expectObjectError(Object, ObjectValidationErrorKind::InvalidHeader, 0);
    }
    const core::Version InvalidInstructionSetVersions[] = {
        core::Version(2, 0, 0),
        core::Version(1, 1, 0),
        core::Version(1, 0, 1),
    };
    for (const auto &Version : InvalidInstructionSetVersions)
    {
      SCOPED_TRACE(Version.toString());
      Object = makeValidObjectFile();
      Object.getHeader().getInstructionSetVersion() = Version;
      expectObjectError(Object, ObjectValidationErrorKind::InvalidHeader, 0);
    }
    Object = makeValidObjectFile();
    Object.getHeader().getAddressBitWidth() = 32;
    expectObjectError(Object, ObjectValidationErrorKind::InvalidHeader, 0);
    Object = makeValidObjectFile();
    Object.getHeader().getByteOrder() = static_cast<ObjectByteOrder>(255);
    expectObjectError(Object, ObjectValidationErrorKind::InvalidHeader, 0);
  }

  // Header construction reads distinct configured versions and validation rejects either header version when it differs.
  TEST(VMObjectFileTest, ConstructsAndValidatesHeadersUsingConfiguredVersions)
  {
    core::test::ScopedEnvironmentVariable FormatEnvironment("INK_OBJECT_FORMAT_VERSION");
    core::test::ScopedEnvironmentVariable InstructionSetEnvironment("INK_OBJECT_INSTRUCTION_SET_VERSION");
    ASSERT_TRUE(FormatEnvironment.set("3.2.1"));
    ASSERT_TRUE(InstructionSetEnvironment.set("7.8.9"));
    ObjectFile Object = makeValidObjectFile();
    EXPECT_EQ(Object.getHeader().getFormatVersion(), core::Version(3, 2, 1));
    EXPECT_EQ(Object.getHeader().getInstructionSetVersion(), core::Version(7, 8, 9));
    EXPECT_FALSE(ObjectFileValidator::validate(Object).has_value());
    Object.getHeader().getFormatVersion() = core::Version(3, 2, 2);
    expectObjectError(Object, ObjectValidationErrorKind::InvalidHeader, 0);
    Object.getHeader().getFormatVersion() = core::Version(3, 2, 1);
    Object.getHeader().getInstructionSetVersion() = core::Version(7, 9, 9);
    expectObjectError(Object, ObjectValidationErrorKind::InvalidHeader, 0);
  }

  // Code symbol extents reject unsigned wraparound while allowing empty end ranges and overlapping aliases.
  TEST(VMObjectFileTest, ChecksCodeSymbolRangesWithoutUnsignedWraparound)
  {
    ObjectFile Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[1].Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, std::numeric_limits<std::uint64_t>::max(), 4};
    expectObjectError(Object, ObjectValidationErrorKind::InvalidSymbol, 1);
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[1].Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, 4, std::numeric_limits<std::uint64_t>::max()};
    expectObjectError(Object, ObjectValidationErrorKind::InvalidSymbol, 1);
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[1].Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, Object.getCode().byteSize(), 0};
    EXPECT_FALSE(ObjectFileValidator::validate(Object).has_value());
    Object.getSymbols().getEntries()[1].Definition->ByteSize = 1;
    expectObjectError(Object, ObjectValidationErrorKind::InvalidSymbol, 1);
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[1].Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, 2, 4};
    EXPECT_FALSE(ObjectFileValidator::validate(Object).has_value());
  }

  // Constants and global storage independently require nonzero power-of-two alignments.
  TEST(VMObjectFileTest, RejectsInvalidStorageAlignments)
  {
    const std::uint64_t InvalidAlignments[] = {0, 3};
    for (const auto Alignment : InvalidAlignments)
    {
      SCOPED_TRACE(Alignment);
      ObjectFile Object = makeValidObjectFile();
      Object.getConstants().getAlignment() = Alignment;
      expectObjectError(Object, ObjectValidationErrorKind::InvalidConstantSection, 0);
      Object = makeValidObjectFile();
      Object.getGlobalStorage().getAlignment() = Alignment;
      expectObjectError(Object, ObjectValidationErrorKind::InvalidGlobalStorage, 0);
    }
  }

  // Global storage accepts no initializer or a full-capacity initializer, but rejects a prefix beyond its byte size.
  TEST(VMObjectFileTest, ChecksGlobalInitializationPrefixAgainstCapacity)
  {
    ObjectFile Object = makeValidObjectFile();
    Object.getGlobalStorage().getInitialBytes().clear();
    EXPECT_EQ(Object.getGlobalStorage().getByteSize(), 64U);
    EXPECT_TRUE(Object.getGlobalStorage().getInitialBytes().empty());
    EXPECT_FALSE(ObjectFileValidator::validate(Object).has_value());
    Object.getGlobalStorage().getInitialBytes().assign(64, 0xAB);
    EXPECT_FALSE(ObjectFileValidator::validate(Object).has_value());
    Object.getGlobalStorage().getInitialBytes().push_back(0xCD);
    expectObjectError(Object, ObjectValidationErrorKind::InvalidGlobalStorage, 0);
  }

  // Symbol definitions reject unknown section categories rather than treating them as code or another storage region.
  TEST(VMObjectFileTest, RejectsUnknownSymbolSections)
  {
    ObjectFile Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[0].Definition->Section = static_cast<ObjectSectionKind>(255);
    expectObjectError(Object, ObjectValidationErrorKind::InvalidSymbol, 0);
  }

  // Constants and global symbol ranges reject overrun and overflow while allowing empty symbols at each region's end.
  TEST(VMObjectFileTest, ChecksSymbolRangesAgainstConstantAndGlobalStorage)
  {
    ObjectFile Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[3].Definition = ObjectSymbolDefinition{ObjectSectionKind::Constants, 3, 1};
    expectObjectError(Object, ObjectValidationErrorKind::InvalidSymbol, 3);
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[3].Definition = ObjectSymbolDefinition{ObjectSectionKind::Constants, 3, 0};
    EXPECT_FALSE(ObjectFileValidator::validate(Object).has_value());
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[3].Definition = ObjectSymbolDefinition{ObjectSectionKind::Constants, 2, std::numeric_limits<std::uint64_t>::max()};
    expectObjectError(Object, ObjectValidationErrorKind::InvalidSymbol, 3);
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[2].Definition = ObjectSymbolDefinition{ObjectSectionKind::GlobalStorage, 64, 1};
    expectObjectError(Object, ObjectValidationErrorKind::InvalidSymbol, 2);
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[2].Definition = ObjectSymbolDefinition{ObjectSectionKind::GlobalStorage, 64, 0};
    EXPECT_FALSE(ObjectFileValidator::validate(Object).has_value());
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[2].Definition = ObjectSymbolDefinition{ObjectSectionKind::GlobalStorage, 2, std::numeric_limits<std::uint64_t>::max()};
    expectObjectError(Object, ObjectValidationErrorKind::InvalidSymbol, 2);
    Object = makeValidObjectFile();
    Object.getGlobalStorage().getByteSize() = std::numeric_limits<std::uint64_t>::max();
    Object.getSymbols().getEntries()[4].Definition = ObjectSymbolDefinition{ObjectSectionKind::GlobalStorage, std::numeric_limits<std::uint64_t>::max() - 3, 4};
    expectObjectError(Object, ObjectValidationErrorKind::InvalidSymbol, 4);
    Object.getSymbols().getEntries()[4].Definition = ObjectSymbolDefinition{ObjectSectionKind::GlobalStorage, std::numeric_limits<std::uint64_t>::max() - 3, 3};
    EXPECT_FALSE(ObjectFileValidator::validate(Object).has_value());
  }

  // Explicit linkage rejects contradictory definitions without changing the declared import intent.
  TEST(VMObjectFileTest, RejectsContradictorySymbolLinkageMetadata)
  {
    ObjectFile Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[5].Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, 0, 4};
    EXPECT_TRUE(Object.getSymbols().getEntries()[5].isImport());
    expectObjectError(Object, ObjectValidationErrorKind::InvalidSymbol, 5);
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[0].Definition.reset();
    EXPECT_FALSE(Object.getSymbols().getEntries()[0].isImport());
    expectObjectError(Object, ObjectValidationErrorKind::InvalidSymbol, 0);
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[1].Definition.reset();
    EXPECT_FALSE(Object.getSymbols().getEntries()[1].isImport());
    expectObjectError(Object, ObjectValidationErrorKind::InvalidSymbol, 1);
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[6].Linkage = static_cast<ObjectSymbolLinkage>(255);
    expectObjectError(Object, ObjectValidationErrorKind::InvalidSymbol, 6);
  }

  // Local names may match imports or exports while imported and exported symbols share one external namespace.
  TEST(VMObjectFileTest, AllowsLocalNamesWithoutExternalConflicts)
  {
    ObjectFile Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[1].Name = "entry";
    Object.getSymbols().getEntries()[3].Name = "entry";
    Object.getSymbols().getEntries()[4].Name = "external_a";
    EXPECT_FALSE(ObjectFileValidator::validate(Object).has_value());
    Object.getSymbols().getEntries()[6].Name = "entry";
    expectObjectError(Object, ObjectValidationErrorKind::InvalidSymbol, 6);
  }

  // External name uniqueness rejects import/import, export/export, and export/import collisions by name alone.
  TEST(VMObjectFileTest, RejectsDuplicateImportAndExportNames)
  {
    ObjectFile Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[6].Name = "external_a";
    expectObjectError(Object, ObjectValidationErrorKind::InvalidSymbol, 6);
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[2].Name = "entry";
    expectObjectError(Object, ObjectValidationErrorKind::InvalidSymbol, 2);
    Object = makeValidObjectFile();
    Object.getSymbols().getEntries()[0].Name = "external_a";
    expectObjectError(Object, ObjectValidationErrorKind::InvalidSymbol, 5);
  }

  // A symbol table may have no references, and slot zero can directly name a definition or an unresolved import.
  TEST(VMObjectFileTest, AcceptsEmptyAndZeroBasedSymbolReferences)
  {
    ObjectFile Object = makeValidObjectFile();
    Object.getBaseReferences().getEntries().clear();
    EXPECT_FALSE(ObjectFileValidator::validate(Object).has_value());
    Object.getBaseReferences().getEntries().push_back({0});
    EXPECT_FALSE(ObjectFileValidator::validate(Object).has_value());
    Object.getBaseReferences().getEntries()[0].SymbolIndex = 5;
    EXPECT_FALSE(ObjectFileValidator::validate(Object).has_value());
    EXPECT_TRUE(Object.getSymbols().getEntries()[Object.getBaseReferences().getEntries()[0].SymbolIndex].isImport());
  }

  // Every reference, including slot zero, must name an existing symbol rather than a register or an absent symbol.
  TEST(VMObjectFileTest, RejectsDanglingBaseReferencesAtEverySlot)
  {
    const ObjectFile Valid = makeValidObjectFile();
    for (std::uint64_t Index = 0; Index < Valid.getBaseReferences().getEntries().size(); ++Index)
    {
      SCOPED_TRACE(Index);
      ObjectFile Object = Valid;
      Object.getBaseReferences().getEntries()[Index].SymbolIndex = static_cast<std::uint32_t>(Object.getSymbols().getEntries().size());
      expectObjectError(Object, ObjectValidationErrorKind::InvalidBaseReference, Index);
      Object.getBaseReferences().getEntries()[Index].SymbolIndex = std::numeric_limits<std::uint32_t>::max();
      expectObjectError(Object, ObjectValidationErrorKind::InvalidBaseReference, Index);
    }
    ObjectFile Empty;
    Empty.getBaseReferences().getEntries().push_back({0});
    expectObjectError(Empty, ObjectValidationErrorKind::InvalidBaseReference, 0);
  }

  // Debug records use global code offsets and reject dangling file indices, code-end offsets, and zero-based locations.
  TEST(VMObjectFileTest, RejectsInvalidGlobalDebugLocations)
  {
    ObjectFile Object = makeValidObjectFile();
    Object.getDebugInfo()->getLines()[1].FileIndex = 2;
    expectObjectError(Object, ObjectValidationErrorKind::InvalidDebugInfo, 1);
    Object = makeValidObjectFile();
    Object.getDebugInfo()->getLines()[1].CodeOffset = 8;
    expectObjectError(Object, ObjectValidationErrorKind::InvalidDebugInfo, 1);
    Object = makeValidObjectFile();
    Object.getDebugInfo()->getLines()[1].CodeOffset = std::numeric_limits<std::uint64_t>::max();
    expectObjectError(Object, ObjectValidationErrorKind::InvalidDebugInfo, 1);
    Object = makeValidObjectFile();
    Object.getDebugInfo()->getLines()[1].Line = 0;
    expectObjectError(Object, ObjectValidationErrorKind::InvalidDebugInfo, 1);
    Object = makeValidObjectFile();
    Object.getDebugInfo()->getLines()[1].Column = 0;
    expectObjectError(Object, ObjectValidationErrorKind::InvalidDebugInfo, 1);
  }
} // namespace ink::vm::test
