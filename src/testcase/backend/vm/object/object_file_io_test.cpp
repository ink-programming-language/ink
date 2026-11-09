#include "../../../core/support/environment_test_support.h"

#include "ink/backend/vm/object/object_file_reader.h"
#include "ink/backend/vm/object/object_file_writer.h"
#include "ink/core/name/name_pool.h"
#include "ink/core/diagnostic/diagnostic_engine.h"

#include <gtest/gtest.h>

#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace ink::vm::test
{
  class VMObjectFileIOTest : public testing::Test
  {
    protected:
      void SetUp() override
      {
        ASSERT_TRUE(FormatEnvironment.set(nullptr));
        ASSERT_TRUE(InstructionSetEnvironment.set(nullptr));
      }

    private:
      core::test::ScopedEnvironmentVariable FormatEnvironment{"INK_OBJECT_FORMAT_VERSION"};
      core::test::ScopedEnvironmentVariable InstructionSetEnvironment{"INK_OBJECT_INSTRUCTION_SET_VERSION"};
  };

  class ScopedObjectIODirectory final
  {
    public:
      ScopedObjectIODirectory()
      {
        std::error_code Error;
        const auto Root = std::filesystem::temp_directory_path(Error);
        if (Error)
        {
          return;
        }
        static std::atomic<std::uint64_t> Sequence{0};
        const auto Stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        for (std::uint32_t Attempt = 0; Attempt < 64; ++Attempt)
        {
          const auto Candidate = Root / ("ink-object-io-" + std::to_string(Stamp) + "-" + std::to_string(Sequence.fetch_add(1)));
          if (std::filesystem::create_directory(Candidate, Error))
          {
            Directory = Candidate;
            return;
          }
          if (Error && Error != std::errc::file_exists)
          {
            return;
          }
          Error.clear();
        }
      }

      ~ScopedObjectIODirectory()
      {
        if (Directory.empty())
        {
          return;
        }
        std::error_code Error;
        std::filesystem::remove(filePath(), Error);
        EXPECT_FALSE(Error);
        Error.clear();
        std::filesystem::remove(Directory, Error);
        EXPECT_FALSE(Error);
      }

      ScopedObjectIODirectory(const ScopedObjectIODirectory &) = delete;
      ScopedObjectIODirectory &operator=(const ScopedObjectIODirectory &) = delete;

      bool valid() const noexcept
      {
        return !Directory.empty();
      }

      const std::filesystem::path &directoryPath() const noexcept
      {
        return Directory;
      }

      std::filesystem::path filePath() const
      {
        return Directory / "object.inko";
      }

    private:
      std::filesystem::path Directory;
  };

  std::vector<std::uint8_t> bytesFromGoldenHex(std::string_view Text)
  {
    std::vector<std::uint8_t> Bytes;
    int High = -1;
    for (const char Character : Text)
    {
      if (Character == ' ' || Character == '\n' || Character == '\r' || Character == '\t')
      {
        continue;
      }
      const int Digit = Character >= '0' && Character <= '9' ? Character - '0' : Character >= 'a' && Character <= 'f' ? Character - 'a' + 10
                                                                                                                      : -1;
      if (Digit < 0)
      {
        return {};
      }
      if (High < 0)
      {
        High = Digit;
      }
      else
      {
        Bytes.push_back(static_cast<std::uint8_t>(High * 16 + Digit));
        High = -1;
      }
    }
    return High < 0 ? Bytes : std::vector<std::uint8_t>{};
  }

  // These bytes are written by hand from the wire specification, independently of either codec.
  std::vector<std::uint8_t> littleEndianObjectGolden()
  {
    return bytesFromGoldenHex(R"(
49 4e 4b 4f
00
40 00
02 00 00 00
00 00 00 00
00 00 00 00
01 00 00 00
01 00 00 00
00 00 00 00
02 00 00 00 00 00 00 00 aa bb
04 00 00 00 00 00 00 00
01 00 00 00 00 00 00 00 cc
08 00 00 00 00 00 00 00
09 00 00 00 00 00 00 00
02 00 00 00 00 00 00 00 dd ee
01 00 00 00 00 00 00 00
01 00 00 00 00 00 00 00 78
01 01 02
01 00 00 00 00 00 00 00
02 00 00 00 00 00 00 00
01 00 00 00 00 00 00 00
00 00 00 00
01
01 00 00 00 00 00 00 00
01 00 00 00 00 00 00 00 73
01 00 00 00 00 00 00 00
01 00 00 00 00 00 00 00
00 00 00 00
04 03 02 01
08 07 06 05
)");
  }

  std::vector<std::uint8_t> bigEndianObjectGolden()
  {
    return bytesFromGoldenHex(R"(
49 4e 4b 4f
01
00 40
00 00 00 02
00 00 00 00
00 00 00 00
00 00 00 01
00 00 00 01
00 00 00 00
00 00 00 00 00 00 00 02 aa bb
00 00 00 00 00 00 00 04
00 00 00 00 00 00 00 01 cc
00 00 00 00 00 00 00 08
00 00 00 00 00 00 00 09
00 00 00 00 00 00 00 02 dd ee
00 00 00 00 00 00 00 01
00 00 00 00 00 00 00 01 78
01 01 02
00 00 00 00 00 00 00 01
00 00 00 00 00 00 00 02
00 00 00 00 00 00 00 01
00 00 00 00
01
00 00 00 00 00 00 00 01
00 00 00 00 00 00 00 01 73
00 00 00 00 00 00 00 01
00 00 00 00 00 00 00 01
00 00 00 00
01 02 03 04
05 06 07 08
)");
  }

  std::vector<std::uint8_t> emptyObjectGolden()
  {
    return bytesFromGoldenHex(R"(
49 4e 4b 4f
00
40 00
02 00 00 00
00 00 00 00
00 00 00 00
01 00 00 00
01 00 00 00
00 00 00 00
00 00 00 00 00 00 00 00
01 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
01 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00
00
)");
  }

  ObjectFile makeGoldenObject(ObjectByteOrder ByteOrder)
  {
    ObjectFile Object;
    Object.getHeader().getByteOrder() = ByteOrder;
    Object.getCode().getBytes() = {0xAA, 0xBB};
    Object.getConstants().getAlignment() = 4;
    Object.getConstants().getBytes() = {0xCC};
    Object.getGlobalStorage().getAlignment() = 8;
    Object.getGlobalStorage().getByteSize() = 9;
    Object.getGlobalStorage().getInitialBytes() = {0xDD, 0xEE};
    ObjectSymbol Symbol;
    Symbol.Name = core::NamePool::instance().intern("x");
    Symbol.Linkage = ObjectSymbolLinkage::Export;
    Symbol.Definition = ObjectSymbolDefinition{ObjectSectionKind::GlobalStorage, 1, 2};
    Object.getSymbols().getEntries().push_back(Symbol);
    Object.getBaseReferences().getEntries().push_back({0});
    ObjectDebugInfo Debug;
    Debug.getSourceFiles().push_back("s");
    Debug.getLines().push_back({1, 0, 0x01020304, 0x05060708});
    Object.getDebugInfo() = std::move(Debug);
    return Object;
  }

  ObjectFile makeCompleteIOObject(ObjectByteOrder ByteOrder)
  {
    ObjectFile Object;
    Object.getHeader().getByteOrder() = ByteOrder;
    Object.getCode().getBytes() = {0xFF, 0, 1, 2, 3, 4, 5, 6};
    Object.getConstants().getAlignment() = 4;
    Object.getConstants().getBytes() = {'o', 'k', 0};
    Object.getGlobalStorage().getAlignment() = 16;
    Object.getGlobalStorage().getByteSize() = 64;
    Object.getGlobalStorage().getInitialBytes() = {1, 2, 3, 4};
    ObjectSymbol Entry;
    Entry.Name = core::NamePool::instance().intern("entry");
    Entry.Linkage = ObjectSymbolLinkage::Export;
    Entry.Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, 0, 4};
    Object.getSymbols().getEntries().push_back(Entry);
    ObjectSymbol Constant;
    Constant.Name = core::NamePool::instance().intern(std::string_view("local\0constant", 14));
    Constant.Definition = ObjectSymbolDefinition{ObjectSectionKind::Constants, 0, 3};
    Object.getSymbols().getEntries().push_back(Constant);
    ObjectSymbol Counter;
    Counter.Name = core::NamePool::instance().intern("counter");
    Counter.Linkage = ObjectSymbolLinkage::Export;
    Counter.Definition = ObjectSymbolDefinition{ObjectSectionKind::GlobalStorage, 0, 4};
    Object.getSymbols().getEntries().push_back(Counter);
    ObjectSymbol Imported;
    Imported.Name = core::NamePool::instance().intern("external");
    Imported.Linkage = ObjectSymbolLinkage::Import;
    Object.getSymbols().getEntries().push_back(Imported);
    Object.getBaseReferences().getEntries().push_back({0});
    Object.getBaseReferences().getEntries().push_back({1});
    Object.getBaseReferences().getEntries().push_back({2});
    Object.getBaseReferences().getEntries().push_back({3});
    ObjectDebugInfo Debug;
    Debug.getSourceFiles() = {std::string("source\0file.ink", 15), "other.ink"};
    Debug.getLines() = {
        {0, 0, 1, 1},
        {7, 1, 13, 17},
    };
    Object.getDebugInfo() = std::move(Debug);
    return Object;
  }

  void expectSameObjectContents(const ObjectFile &Expected, const ObjectFile &Actual)
  {
    EXPECT_EQ(Actual.getHeader().getMagic(), Expected.getHeader().getMagic());
    EXPECT_EQ(Actual.getHeader().getByteOrder(), Expected.getHeader().getByteOrder());
    EXPECT_EQ(Actual.getHeader().getAddressBitWidth(), Expected.getHeader().getAddressBitWidth());
    EXPECT_EQ(Actual.getHeader().getFormatVersion(), Expected.getHeader().getFormatVersion());
    EXPECT_EQ(Actual.getHeader().getInstructionSetVersion(), Expected.getHeader().getInstructionSetVersion());
    EXPECT_EQ(Actual.getCode().getBytes(), Expected.getCode().getBytes());
    EXPECT_EQ(Actual.getConstants().getAlignment(), Expected.getConstants().getAlignment());
    EXPECT_EQ(Actual.getConstants().getBytes(), Expected.getConstants().getBytes());
    EXPECT_EQ(Actual.getGlobalStorage().getAlignment(), Expected.getGlobalStorage().getAlignment());
    EXPECT_EQ(Actual.getGlobalStorage().getByteSize(), Expected.getGlobalStorage().getByteSize());
    EXPECT_EQ(Actual.getGlobalStorage().getInitialBytes(), Expected.getGlobalStorage().getInitialBytes());
    const auto &ExpectedSymbols = Expected.getSymbols().getEntries();
    const auto &ActualSymbols = Actual.getSymbols().getEntries();
    ASSERT_EQ(ActualSymbols.size(), ExpectedSymbols.size());
    for (std::size_t Index = 0; Index < ExpectedSymbols.size(); ++Index)
    {
      SCOPED_TRACE(Index);
      EXPECT_EQ(ActualSymbols[Index].Name, ExpectedSymbols[Index].Name);
      EXPECT_EQ(ActualSymbols[Index].Linkage, ExpectedSymbols[Index].Linkage);
      ASSERT_EQ(ActualSymbols[Index].Definition.has_value(), ExpectedSymbols[Index].Definition.has_value());
      if (ExpectedSymbols[Index].Definition)
      {
        EXPECT_EQ(ActualSymbols[Index].Definition->Section, ExpectedSymbols[Index].Definition->Section);
        EXPECT_EQ(ActualSymbols[Index].Definition->ByteOffset, ExpectedSymbols[Index].Definition->ByteOffset);
        EXPECT_EQ(ActualSymbols[Index].Definition->ByteSize, ExpectedSymbols[Index].Definition->ByteSize);
      }
    }
    const auto &ExpectedBases = Expected.getBaseReferences().getEntries();
    const auto &ActualBases = Actual.getBaseReferences().getEntries();
    ASSERT_EQ(ActualBases.size(), ExpectedBases.size());
    for (std::size_t Index = 0; Index < ExpectedBases.size(); ++Index)
    {
      EXPECT_EQ(ActualBases[Index].SymbolIndex, ExpectedBases[Index].SymbolIndex);
    }
    ASSERT_EQ(Actual.getDebugInfo().has_value(), Expected.getDebugInfo().has_value());
    if (Expected.getDebugInfo())
    {
      EXPECT_EQ(Actual.getDebugInfo()->getSourceFiles(), Expected.getDebugInfo()->getSourceFiles());
      const auto &ExpectedLines = Expected.getDebugInfo()->getLines();
      const auto &ActualLines = Actual.getDebugInfo()->getLines();
      ASSERT_EQ(ActualLines.size(), ExpectedLines.size());
      for (std::size_t Index = 0; Index < ExpectedLines.size(); ++Index)
      {
        EXPECT_EQ(ActualLines[Index].CodeOffset, ExpectedLines[Index].CodeOffset);
        EXPECT_EQ(ActualLines[Index].FileIndex, ExpectedLines[Index].FileIndex);
        EXPECT_EQ(ActualLines[Index].Line, ExpectedLines[Index].Line);
        EXPECT_EQ(ActualLines[Index].Column, ExpectedLines[Index].Column);
      }
    }
  }

  void expectCompleteRoundTrip(ObjectByteOrder ByteOrder)
  {
    const auto Original = makeCompleteIOObject(ByteOrder);
    const auto Encoded = ObjectFileWriter::write(Original, core::DiagnosticEngine{});
    ASSERT_TRUE(Encoded.has_value());
    const auto Decoded = ObjectFileReader::read(*Encoded, core::DiagnosticEngine{});
    ASSERT_TRUE(Decoded.has_value());
    expectSameObjectContents(Original, *Decoded);
    EXPECT_EQ(core::NamePool::instance().text(Decoded->getSymbols().getEntries()[1].Name), std::string_view("local\0constant", 14));
    const auto Rewritten = ObjectFileWriter::write(*Decoded, core::DiagnosticEngine{});
    ASSERT_TRUE(Rewritten.has_value());
    EXPECT_EQ(*Rewritten, *Encoded);
  }

  void overwriteLittleInteger(std::vector<std::uint8_t> &Bytes, std::size_t Offset, std::uint64_t Value, std::size_t ByteCount)
  {
    for (std::size_t Index = 0; Index < ByteCount; ++Index)
    {
      Bytes[Offset + Index] = static_cast<std::uint8_t>(Value >> (8 * Index));
    }
  }

  // Little-endian round trips preserve every region, linkage, reference, debug record, and embedded NUL string.
  TEST_F(VMObjectFileIOTest, RoundTripsCompleteLittleEndianObject)
  {
    expectCompleteRoundTrip(ObjectByteOrder::Little);
  }

  // Big-endian round trips preserve the same complete object and rewrite to exactly the original bytes.
  TEST_F(VMObjectFileIOTest, RoundTripsCompleteBigEndianObject)
  {
    expectCompleteRoundTrip(ObjectByteOrder::Big);
  }

  // Separate input byte buffers intern equal symbol spellings into one handle without changing the existing wire bytes.
  TEST_F(VMObjectFileIOTest, InternsSymbolNamesAcrossIndependentInputBuffers)
  {
    const auto FirstBytes = littleEndianObjectGolden();
    const auto SecondBytes = FirstBytes;
    ASSERT_NE(FirstBytes.data(), SecondBytes.data());
    const auto First = ObjectFileReader::read(FirstBytes, core::DiagnosticEngine{});
    const auto Second = ObjectFileReader::read(SecondBytes, core::DiagnosticEngine{});
    ASSERT_TRUE(First.has_value());
    ASSERT_TRUE(Second.has_value());
    ASSERT_EQ(First->getSymbols().getEntries().size(), 1U);
    ASSERT_EQ(Second->getSymbols().getEntries().size(), 1U);
    const core::Name FirstName = First->getSymbols().getEntries()[0].Name;
    const core::Name SecondName = Second->getSymbols().getEntries()[0].Name;
    EXPECT_EQ(FirstName, SecondName);
    EXPECT_EQ(core::NamePool::instance().text(FirstName), "x");
    const auto Rewritten = ObjectFileWriter::write(*Second, core::DiagnosticEngine{});
    ASSERT_TRUE(Rewritten.has_value());
    EXPECT_EQ(*Rewritten, littleEndianObjectGolden());
  }

  // An empty wire spelling becomes NAME_None and reports a fatal VM object validation diagnostic with its symbol index.
  TEST_F(VMObjectFileIOTest, RejectsEmptyWireSymbolNamesAfterInterning)
  {
    auto Bytes = littleEndianObjectGolden();
    overwriteLittleInteger(Bytes, 92, 0, 8);
    Bytes.erase(Bytes.begin() + 100);
    EXPECT_DEATH(static_cast<void>(ObjectFileReader::read(Bytes, core::DiagnosticEngine{})), "INK-V0003.*invalid VM object in symbol #0: name is not registered in the name pool");
  }

  // Independent little-endian bytes fix all field widths, order, flags, and multibyte debug values without padding.
  TEST_F(VMObjectFileIOTest, MatchesIndependentLittleEndianGolden)
  {
    const auto Golden = littleEndianObjectGolden();
    ASSERT_EQ(Golden.size(), 178U);
    const auto Expected = makeGoldenObject(ObjectByteOrder::Little);
    const auto Encoded = ObjectFileWriter::write(Expected, core::DiagnosticEngine{});
    ASSERT_TRUE(Encoded.has_value());
    EXPECT_EQ(*Encoded, Golden);
    const auto Decoded = ObjectFileReader::read(Golden, core::DiagnosticEngine{});
    ASSERT_TRUE(Decoded.has_value());
    expectSameObjectContents(Expected, *Decoded);
  }

  // Independent big-endian bytes verify byte order across u16, u32, u64, symbol definitions, and debug locations.
  TEST_F(VMObjectFileIOTest, MatchesIndependentBigEndianGolden)
  {
    const auto Golden = bigEndianObjectGolden();
    ASSERT_EQ(Golden.size(), 178U);
    const auto Expected = makeGoldenObject(ObjectByteOrder::Big);
    const auto Encoded = ObjectFileWriter::write(Expected, core::DiagnosticEngine{});
    ASSERT_TRUE(Encoded.has_value());
    EXPECT_EQ(*Encoded, Golden);
    const auto Decoded = ObjectFileReader::read(Golden, core::DiagnosticEngine{});
    ASSERT_TRUE(Decoded.has_value());
    expectSameObjectContents(Expected, *Decoded);
  }

  // Symbol references use exactly one endian-aware u32 per slot, with no presence byte or register entries.
  TEST_F(VMObjectFileIOTest, EncodesSymbolReferenceIndicesAsFixedWidthIntegers)
  {
    const ObjectByteOrder Orders[] = {ObjectByteOrder::Little, ObjectByteOrder::Big};
    const std::uint8_t ReferenceBytes[][4] = {
        {0x00, 0x01, 0x00, 0x00},
        {0x00, 0x00, 0x01, 0x00},
    };
    for (std::size_t OrderIndex = 0; OrderIndex < 2; ++OrderIndex)
    {
      SCOPED_TRACE(OrderIndex);
      ObjectFile Object;
      Object.getHeader().getByteOrder() = Orders[OrderIndex];
      Object.getSymbols().getEntries().resize(257);
      for (auto &Symbol : Object.getSymbols().getEntries())
      {
        Symbol.Name = core::NamePool::instance().intern("x");
        Symbol.Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, 0, 0};
      }
      Object.getBaseReferences().getEntries().push_back({256});
      const auto Encoded = ObjectFileWriter::write(Object, core::DiagnosticEngine{});
      ASSERT_TRUE(Encoded.has_value());
      ASSERT_EQ(Encoded->size(), 96U + 257U * 28U + 4U);
      const auto ReferenceOffset = Encoded->size() - 5;
      for (std::size_t ByteIndex = 0; ByteIndex < 4; ++ByteIndex)
      {
        EXPECT_EQ((*Encoded)[ReferenceOffset + ByteIndex], ReferenceBytes[OrderIndex][ByteIndex]);
      }
      const auto Decoded = ObjectFileReader::read(*Encoded, core::DiagnosticEngine{});
      ASSERT_TRUE(Decoded.has_value());
      ASSERT_EQ(Decoded->getBaseReferences().getEntries().size(), 1U);
      EXPECT_EQ(Decoded->getBaseReferences().getEntries()[0].SymbolIndex, 256U);
    }
  }

  // An empty object has exactly 96 bytes, no register placeholders, and an absent debug payload.
  TEST_F(VMObjectFileIOTest, MatchesMinimalEmptyObjectWireLayout)
  {
    const ObjectFile Empty;
    const auto Golden = emptyObjectGolden();
    ASSERT_EQ(Golden.size(), 96U);
    const auto Encoded = ObjectFileWriter::write(Empty, core::DiagnosticEngine{});
    ASSERT_TRUE(Encoded.has_value());
    EXPECT_EQ(*Encoded, Golden);
    const auto Decoded = ObjectFileReader::read(Golden, core::DiagnosticEngine{});
    ASSERT_TRUE(Decoded.has_value());
    expectSameObjectContents(Empty, *Decoded);
  }

  // A present but empty debug record remains distinct from an absent optional debug record after decoding.
  TEST_F(VMObjectFileIOTest, PreservesPresentEmptyDebugInfo)
  {
    ObjectFile Object;
    Object.getDebugInfo().emplace();
    const auto Encoded = ObjectFileWriter::write(Object, core::DiagnosticEngine{});
    ASSERT_TRUE(Encoded.has_value());
    EXPECT_EQ(Encoded->size(), 112U);
    const auto Decoded = ObjectFileReader::read(*Encoded, core::DiagnosticEngine{});
    ASSERT_TRUE(Decoded.has_value());
    ASSERT_TRUE(Decoded->getDebugInfo().has_value());
    EXPECT_TRUE(Decoded->getDebugInfo()->getSourceFiles().empty());
    EXPECT_TRUE(Decoded->getDebugInfo()->getLines().empty());
  }

  // Every strict prefix of a valid object fails instead of accepting a partial record or missing final field.
  TEST_F(VMObjectFileIOTest, RejectsEveryBytewiseTruncation)
  {
    const auto Golden = littleEndianObjectGolden();
    ASSERT_EQ(Golden.size(), 178U);
    for (std::size_t Length = 0; Length < Golden.size(); ++Length)
    {
      SCOPED_TRACE(Length);
      EXPECT_FALSE(ObjectFileReader::read(std::span<const std::uint8_t>(Golden.data(), Length), core::DiagnosticEngine{}).has_value());
    }
  }

  // A complete object followed by an extra byte reports trailing data at the first unconsumed byte.
  TEST_F(VMObjectFileIOTest, RejectsTrailingData)
  {
    auto Bytes = littleEndianObjectGolden();
    Bytes.push_back(0xA5);
    const auto Decoded = ObjectFileReader::read(Bytes, core::DiagnosticEngine{});
    ASSERT_FALSE(Decoded.has_value());
    EXPECT_EQ(Decoded.error().Kind, ObjectFileIOErrorKind::TrailingData);
    EXPECT_EQ(Decoded.error().ByteOffset, 178U);
  }

  // Invalid magic is a fatal header validation failure while an unknown byte-order marker remains a recoverable encoding error.
  TEST_F(VMObjectFileIOTest, RejectsCorruptMagicAndByteOrder)
  {
    auto Bytes = emptyObjectGolden();
    Bytes[0] = 'X';
    EXPECT_DEATH(static_cast<void>(ObjectFileReader::read(Bytes, core::DiagnosticEngine{})), "INK-V0003.*invalid VM object in header: magic is invalid");
    Bytes = emptyObjectGolden();
    Bytes[4] = 2;
    const auto InvalidOrder = ObjectFileReader::read(Bytes, core::DiagnosticEngine{});
    ASSERT_FALSE(InvalidOrder.has_value());
    EXPECT_EQ(InvalidOrder.error().Kind, ObjectFileIOErrorKind::InvalidEncoding);
    EXPECT_EQ(InvalidOrder.error().ByteOffset, 4U);
  }

  // Optional-definition and debug flags accept only the protocol's zero and one values.
  TEST_F(VMObjectFileIOTest, RejectsNonBooleanPresenceFlags)
  {
    const std::size_t FlagOffsets[] = {102, 132};
    for (const auto Offset : FlagOffsets)
    {
      SCOPED_TRACE(Offset);
      auto Bytes = littleEndianObjectGolden();
      Bytes[Offset] = 2;
      const auto Decoded = ObjectFileReader::read(Bytes, core::DiagnosticEngine{});
      ASSERT_FALSE(Decoded.has_value());
      EXPECT_EQ(Decoded.error().Kind, ObjectFileIOErrorKind::InvalidEncoding);
      EXPECT_EQ(Decoded.error().ByteOffset, Offset);
    }
  }

  // Unknown linkage and section enum values fail rather than being mapped to a default category.
  TEST_F(VMObjectFileIOTest, RejectsUnknownWireEnums)
  {
    const std::size_t EnumOffsets[] = {101, 103};
    for (const auto Offset : EnumOffsets)
    {
      SCOPED_TRACE(Offset);
      auto Bytes = littleEndianObjectGolden();
      Bytes[Offset] = 255;
      const auto Decoded = ObjectFileReader::read(Bytes, core::DiagnosticEngine{});
      ASSERT_FALSE(Decoded.has_value());
      EXPECT_EQ(Decoded.error().Kind, ObjectFileIOErrorKind::InvalidEncoding);
      EXPECT_EQ(Decoded.error().ByteOffset, Offset);
    }
  }

  // Huge logical global capacity needs no payload allocation, while oversized byte and string lengths cannot be read.
  TEST_F(VMObjectFileIOTest, RejectsOversizedPayloadsWithoutMaterializingGlobalCapacity)
  {
    auto CapacityBytes = emptyObjectGolden();
    overwriteLittleInteger(CapacityBytes, 63, std::numeric_limits<std::uint64_t>::max(), 8);
    const auto CapacityObject = ObjectFileReader::read(CapacityBytes, core::DiagnosticEngine{});
    ASSERT_TRUE(CapacityObject.has_value());
    EXPECT_EQ(CapacityObject->getGlobalStorage().getByteSize(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_TRUE(CapacityObject->getGlobalStorage().getInitialBytes().empty());
    const std::size_t EmptyPayloadOffsets[] = {31, 47, 71};
    for (const auto Offset : EmptyPayloadOffsets)
    {
      SCOPED_TRACE(Offset);
      auto Bytes = emptyObjectGolden();
      overwriteLittleInteger(Bytes, Offset, std::numeric_limits<std::uint64_t>::max(), 8);
      EXPECT_FALSE(ObjectFileReader::read(Bytes, core::DiagnosticEngine{}).has_value());
    }
    const std::size_t StringLengthOffsets[] = {92, 141};
    for (const auto Offset : StringLengthOffsets)
    {
      SCOPED_TRACE(Offset);
      auto Bytes = littleEndianObjectGolden();
      overwriteLittleInteger(Bytes, Offset, std::numeric_limits<std::uint64_t>::max(), 8);
      EXPECT_FALSE(ObjectFileReader::read(Bytes, core::DiagnosticEngine{}).has_value());
    }
  }

  // Indexed table counts beyond the u32 capacity report an ICE before allocation, while oversized nonindexed line payloads remain decoding errors.
  TEST_F(VMObjectFileIOTest, RejectsOversizedRecordCounts)
  {
    struct IndexedCount
    {
        std::size_t Offset;
        const char *Context;
        bool HasDebugInfo;
    };
    const IndexedCount Counts[] = {
        {79, "symbol table", false},
        {87, "base reference table", false},
        {133, "debug source file table", true},
    };
    for (const auto &Count : Counts)
    {
      SCOPED_TRACE(Count.Offset);
      auto Bytes = Count.HasDebugInfo ? littleEndianObjectGolden() : emptyObjectGolden();
      overwriteLittleInteger(Bytes, Count.Offset, std::numeric_limits<std::uint64_t>::max(), 8);
      const std::string Pattern = std::string("INK-V0003.*invalid VM object in ") + Count.Context + ": record count exceeds the u32 index range";
      EXPECT_DEATH(static_cast<void>(ObjectFileReader::read(Bytes, core::DiagnosticEngine{})), Pattern);
    }
    auto Bytes = littleEndianObjectGolden();
    overwriteLittleInteger(Bytes, 150, std::numeric_limits<std::uint64_t>::max(), 8);
    const auto Lines = ObjectFileReader::read(Bytes, core::DiagnosticEngine{});
    ASSERT_FALSE(Lines.has_value());
    EXPECT_EQ(Lines.error().Kind, ObjectFileIOErrorKind::UnexpectedEnd);
    EXPECT_EQ(Lines.error().ByteOffset, 150U);
  }

  // Reference counts require four available bytes per record before the reader allocates the table.
  TEST_F(VMObjectFileIOTest, RejectsReferenceCountsWithoutCompleteIntegerPayloads)
  {
    auto Bytes = emptyObjectGolden();
    overwriteLittleInteger(Bytes, 87, 1, 8);
    Bytes.resize(98, 0);
    const auto Decoded = ObjectFileReader::read(Bytes, core::DiagnosticEngine{});
    ASSERT_FALSE(Decoded.has_value());
    EXPECT_EQ(Decoded.error().Kind, ObjectFileIOErrorKind::UnexpectedEnd);
    EXPECT_EQ(Decoded.error().ByteOffset, 87U);
  }

  // A missing payload at the u32 count boundary remains recoverable, while exceeding that boundary reports a fatal structural error.
  TEST_F(VMObjectFileIOTest, ChecksReferenceCountIndexCapacityBeforeAllocation)
  {
    auto Bytes = emptyObjectGolden();
    const auto MaximumCount = static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max()) + 1;
    overwriteLittleInteger(Bytes, 87, MaximumCount, 8);
    const auto MissingPayload = ObjectFileReader::read(Bytes, core::DiagnosticEngine{});
    ASSERT_FALSE(MissingPayload.has_value());
    EXPECT_EQ(MissingPayload.error().Kind, ObjectFileIOErrorKind::UnexpectedEnd);
    EXPECT_EQ(MissingPayload.error().ByteOffset, 87U);
    overwriteLittleInteger(Bytes, 87, MaximumCount + 1, 8);
    EXPECT_DEATH(static_cast<void>(ObjectFileReader::read(Bytes, core::DiagnosticEngine{})), "INK-V0003.*invalid VM object in base reference table: record count exceeds the u32 index range");
  }

  // Each missing byte of a symbol-index payload is rejected rather than treated as an absent reference.
  TEST_F(VMObjectFileIOTest, RejectsTruncatedSymbolReferenceIndices)
  {
    const auto Golden = littleEndianObjectGolden();
    for (std::size_t Length = 128; Length < 132; ++Length)
    {
      SCOPED_TRACE(Length);
      const auto Decoded = ObjectFileReader::read(std::span<const std::uint8_t>(Golden.data(), Length), core::DiagnosticEngine{});
      ASSERT_FALSE(Decoded.has_value());
      EXPECT_EQ(Decoded.error().Kind, ObjectFileIOErrorKind::UnexpectedEnd);
      EXPECT_EQ(Decoded.error().ByteOffset, 120U);
    }
  }

  // Decoded alignment and reference failures report a fatal VM diagnostic naming their region or reference index.
  TEST_F(VMObjectFileIOTest, RejectsSemanticallyInvalidReferencesAndAlignments)
  {
    struct Case
    {
        std::size_t Offset;
        std::uint64_t Value;
        std::size_t ByteCount;
        const char *Pattern;
    };
    const Case Cases[] = {
        {41, 3, 8, "INK-V0003.*invalid VM object in constants: alignment must be a nonzero power of two"},
        {58, 3, 8, "INK-V0003.*invalid VM object in global storage: alignment must be a nonzero power of two"},
        {128, 1, 4, "INK-V0003.*invalid VM object in base reference #0: symbol index is outside the symbol table"},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Offset);
      auto Bytes = littleEndianObjectGolden();
      overwriteLittleInteger(Bytes, Entry.Offset, Entry.Value, Entry.ByteCount);
      EXPECT_DEATH(static_cast<void>(ObjectFileReader::read(Bytes, core::DiagnosticEngine{})), Entry.Pattern);
    }
  }

  // Unsupported width and versions report fatal header diagnostics before decoding any payload bytes.
  TEST_F(VMObjectFileIOTest, RejectsSemanticallyInvalidHeaderVersionsAndAddressWidth)
  {
    struct InvalidHeader
    {
        std::size_t Offset;
        std::uint8_t Value;
        const char *Pattern;
    };
    const InvalidHeader Headers[] = {
        {5, 32, "INK-V0003.*invalid VM object in header: address width must be 64 bits"},
        {7, 3, "INK-V0003.*invalid VM object in header: object format version is unsupported"},
        {19, 3, "INK-V0003.*invalid VM object in header: instruction-set version is unsupported"},
    };
    for (const auto &Header : Headers)
    {
      SCOPED_TRACE(Header.Offset);
      auto Bytes = emptyObjectGolden();
      Bytes[Header.Offset] = Header.Value;
      Bytes.resize(31);
      EXPECT_DEATH(static_cast<void>(ObjectFileReader::read(Bytes, core::DiagnosticEngine{})), Header.Pattern);
    }
  }

  // Legacy version 1 objects report a fatal version diagnostic before decoding their obsolete reserved-register records.
  TEST_F(VMObjectFileIOTest, RejectsLegacyObjectFormatWithReservedRegisterSlots)
  {
    auto Bytes = emptyObjectGolden();
    overwriteLittleInteger(Bytes, 7, 1, 4);
    overwriteLittleInteger(Bytes, 87, 3, 8);
    Bytes.insert(Bytes.end() - 1, 3, 0);
    ASSERT_EQ(Bytes.size(), 99U);
    EXPECT_DEATH(static_cast<void>(ObjectFileReader::read(Bytes, core::DiagnosticEngine{})), "INK-V0003.*invalid VM object in header: object format version is unsupported");
  }

  // Writer validation reports a fatal VM reference diagnostic before encoding an invalid source object.
  TEST_F(VMObjectFileIOTest, WriterRejectsInvalidObject)
  {
    auto Object = makeCompleteIOObject(ObjectByteOrder::Little);
    Object.getBaseReferences().getEntries()[0].SymbolIndex = static_cast<std::uint32_t>(Object.getSymbols().getEntries().size());
    EXPECT_DEATH(static_cast<void>(ObjectFileWriter::write(Object, core::DiagnosticEngine{})), "INK-V0003.*invalid VM object in base reference #0: symbol index is outside the symbol table");
  }

  // File wrappers preserve complete little- and big-endian objects through unique temporary paths.
  TEST_F(VMObjectFileIOTest, RoundTripsObjectsThroughFiles)
  {
    const ObjectByteOrder Orders[] = {ObjectByteOrder::Little, ObjectByteOrder::Big};
    for (const auto Order : Orders)
    {
      SCOPED_TRACE(static_cast<unsigned>(Order));
      ScopedObjectIODirectory Directory;
      ASSERT_TRUE(Directory.valid());
      const auto Object = makeCompleteIOObject(Order);
      const auto Written = ObjectFileWriter::writeFile(Object, Directory.filePath(), core::DiagnosticEngine{});
      ASSERT_TRUE(Written.has_value());
      const auto Read = ObjectFileReader::readFile(Directory.filePath(), core::DiagnosticEngine{});
      ASSERT_TRUE(Read.has_value());
      expectSameObjectContents(Object, *Read);
    }
  }

  // Missing files, absent parent directories, and directory paths fail through explicit file I/O results.
  TEST_F(VMObjectFileIOTest, ReportsFileOpenFailures)
  {
    ScopedObjectIODirectory Directory;
    ASSERT_TRUE(Directory.valid());
    const auto Missing = ObjectFileReader::readFile(Directory.filePath(), core::DiagnosticEngine{});
    ASSERT_FALSE(Missing.has_value());
    EXPECT_EQ(Missing.error().Kind, ObjectFileIOErrorKind::FileOpenFailed);
    const ObjectFile Object;
    const auto MissingParent = ObjectFileWriter::writeFile(Object, Directory.directoryPath() / "missing" / "object.inko", core::DiagnosticEngine{});
    ASSERT_FALSE(MissingParent.has_value());
    EXPECT_EQ(MissingParent.error().Kind, ObjectFileIOErrorKind::FileOpenFailed);
    EXPECT_FALSE(ObjectFileReader::readFile(Directory.directoryPath(), core::DiagnosticEngine{}).has_value());
    EXPECT_FALSE(ObjectFileWriter::writeFile(Object, Directory.directoryPath(), core::DiagnosticEngine{}).has_value());
  }

  // Fatal writer validation preserves the same existing file across death-test processes instead of opening it in truncation mode.
  TEST_F(VMObjectFileIOTest, InvalidObjectDoesNotTruncateExistingFile)
  {
    constexpr const char *PathEnvironmentName = "INK_OBJECT_IO_VALIDATION_DEATH_PATH";
    std::optional<ScopedObjectIODirectory> Directory;
    std::filesystem::path Path;
    const char *InheritedPath = std::getenv(PathEnvironmentName);
    if (InheritedPath && *InheritedPath)
    {
      Path = InheritedPath;
    }
    else
    {
      Directory.emplace();
      ASSERT_TRUE(Directory->valid());
      Path = Directory->filePath();
    }
    core::test::ScopedEnvironmentVariable PathEnvironment(PathEnvironmentName);
    const std::string PathText = Path.string();
    ASSERT_TRUE(PathEnvironment.set(PathText.c_str()));
    const std::array<char, 4> Sentinel = {'k', 'e', 'e', 'p'};
    {
      std::ofstream Output(Path, std::ios::binary | std::ios::trunc);
      ASSERT_TRUE(Output.is_open());
      Output.write(Sentinel.data(), static_cast<std::streamsize>(Sentinel.size()));
      ASSERT_TRUE(Output.good());
    }
    ObjectFile Invalid;
    Invalid.getBaseReferences().getEntries().push_back({0});
    EXPECT_DEATH(static_cast<void>(ObjectFileWriter::writeFile(Invalid, Path, core::DiagnosticEngine{})), "INK-V0003.*invalid VM object in base reference #0: symbol index is outside the symbol table");
    std::ifstream Input(Path, std::ios::binary);
    ASSERT_TRUE(Input.is_open());
    std::array<char, 4> Actual{};
    Input.read(Actual.data(), static_cast<std::streamsize>(Actual.size()));
    EXPECT_EQ(Input.gcount(), static_cast<std::streamsize>(Sentinel.size()));
    EXPECT_EQ(Actual, Sentinel);
    EXPECT_EQ(Input.peek(), std::ifstream::traits_type::eof());
  }
} // namespace ink::vm::test
