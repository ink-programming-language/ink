#include "../../core/support/environment_test_support.h"

#include "ink/vm/object/object_file_reader.h"
#include "ink/vm/object/object_file_writer.h"

#include <gtest/gtest.h>

#include <array>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
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
00 00 00 00
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
00 00 00 00
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
00 00 00 00
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
    Symbol.Name = "x";
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
    Entry.Name = "entry";
    Entry.Linkage = ObjectSymbolLinkage::Export;
    Entry.Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, 0, 4};
    Object.getSymbols().getEntries().push_back(Entry);
    ObjectSymbol Constant;
    Constant.Name = std::string("local\0constant", 14);
    Constant.Definition = ObjectSymbolDefinition{ObjectSectionKind::Constants, 0, 3};
    Object.getSymbols().getEntries().push_back(Constant);
    ObjectSymbol Counter;
    Counter.Name = "counter";
    Counter.Linkage = ObjectSymbolLinkage::Export;
    Counter.Definition = ObjectSymbolDefinition{ObjectSectionKind::GlobalStorage, 0, 4};
    Object.getSymbols().getEntries().push_back(Counter);
    ObjectSymbol Imported;
    Imported.Name = "external";
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
    const auto Encoded = ObjectFileWriter::write(Original);
    ASSERT_TRUE(Encoded.has_value());
    const auto Decoded = ObjectFileReader::read(*Encoded);
    ASSERT_TRUE(Decoded.has_value());
    expectSameObjectContents(Original, *Decoded);
    const auto Rewritten = ObjectFileWriter::write(*Decoded);
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

  // Independent little-endian bytes fix all field widths, order, flags, and multibyte debug values without padding.
  TEST_F(VMObjectFileIOTest, MatchesIndependentLittleEndianGolden)
  {
    const auto Golden = littleEndianObjectGolden();
    ASSERT_EQ(Golden.size(), 178U);
    const auto Expected = makeGoldenObject(ObjectByteOrder::Little);
    const auto Encoded = ObjectFileWriter::write(Expected);
    ASSERT_TRUE(Encoded.has_value());
    EXPECT_EQ(*Encoded, Golden);
    const auto Decoded = ObjectFileReader::read(Golden);
    ASSERT_TRUE(Decoded.has_value());
    expectSameObjectContents(Expected, *Decoded);
  }

  // Independent big-endian bytes verify byte order across u16, u32, u64, symbol definitions, and debug locations.
  TEST_F(VMObjectFileIOTest, MatchesIndependentBigEndianGolden)
  {
    const auto Golden = bigEndianObjectGolden();
    ASSERT_EQ(Golden.size(), 178U);
    const auto Expected = makeGoldenObject(ObjectByteOrder::Big);
    const auto Encoded = ObjectFileWriter::write(Expected);
    ASSERT_TRUE(Encoded.has_value());
    EXPECT_EQ(*Encoded, Golden);
    const auto Decoded = ObjectFileReader::read(Golden);
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
        Symbol.Name = "x";
        Symbol.Definition = ObjectSymbolDefinition{ObjectSectionKind::Code, 0, 0};
      }
      Object.getBaseReferences().getEntries().push_back({256});
      const auto Encoded = ObjectFileWriter::write(Object);
      ASSERT_TRUE(Encoded.has_value());
      ASSERT_EQ(Encoded->size(), 96U + 257U * 28U + 4U);
      const auto ReferenceOffset = Encoded->size() - 5;
      for (std::size_t ByteIndex = 0; ByteIndex < 4; ++ByteIndex)
      {
        EXPECT_EQ((*Encoded)[ReferenceOffset + ByteIndex], ReferenceBytes[OrderIndex][ByteIndex]);
      }
      const auto Decoded = ObjectFileReader::read(*Encoded);
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
    const auto Encoded = ObjectFileWriter::write(Empty);
    ASSERT_TRUE(Encoded.has_value());
    EXPECT_EQ(*Encoded, Golden);
    const auto Decoded = ObjectFileReader::read(Golden);
    ASSERT_TRUE(Decoded.has_value());
    expectSameObjectContents(Empty, *Decoded);
  }

  // A present but empty debug record remains distinct from an absent optional debug record after decoding.
  TEST_F(VMObjectFileIOTest, PreservesPresentEmptyDebugInfo)
  {
    ObjectFile Object;
    Object.getDebugInfo().emplace();
    const auto Encoded = ObjectFileWriter::write(Object);
    ASSERT_TRUE(Encoded.has_value());
    EXPECT_EQ(Encoded->size(), 112U);
    const auto Decoded = ObjectFileReader::read(*Encoded);
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
      EXPECT_FALSE(ObjectFileReader::read(std::span<const std::uint8_t>(Golden.data(), Length)).has_value());
    }
  }

  // A complete object followed by an extra byte reports trailing data at the first unconsumed byte.
  TEST_F(VMObjectFileIOTest, RejectsTrailingData)
  {
    auto Bytes = littleEndianObjectGolden();
    Bytes.push_back(0xA5);
    const auto Decoded = ObjectFileReader::read(Bytes);
    ASSERT_FALSE(Decoded.has_value());
    EXPECT_EQ(Decoded.error().Kind, ObjectFileIOErrorKind::TrailingData);
    EXPECT_EQ(Decoded.error().ByteOffset, 178U);
  }

  // Corrupted magic and unknown byte-order markers are rejected before an object can be returned.
  TEST_F(VMObjectFileIOTest, RejectsCorruptMagicAndByteOrder)
  {
    auto Bytes = emptyObjectGolden();
    Bytes[0] = 'X';
    EXPECT_FALSE(ObjectFileReader::read(Bytes).has_value());
    Bytes = emptyObjectGolden();
    Bytes[4] = 2;
    EXPECT_FALSE(ObjectFileReader::read(Bytes).has_value());
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
      const auto Decoded = ObjectFileReader::read(Bytes);
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
      EXPECT_FALSE(ObjectFileReader::read(Bytes).has_value());
    }
  }

  // Huge logical global capacity needs no payload allocation, while oversized byte and string lengths cannot be read.
  TEST_F(VMObjectFileIOTest, RejectsOversizedPayloadsWithoutMaterializingGlobalCapacity)
  {
    auto CapacityBytes = emptyObjectGolden();
    overwriteLittleInteger(CapacityBytes, 63, std::numeric_limits<std::uint64_t>::max(), 8);
    const auto CapacityObject = ObjectFileReader::read(CapacityBytes);
    ASSERT_TRUE(CapacityObject.has_value());
    EXPECT_EQ(CapacityObject->getGlobalStorage().getByteSize(), std::numeric_limits<std::uint64_t>::max());
    EXPECT_TRUE(CapacityObject->getGlobalStorage().getInitialBytes().empty());
    const std::size_t EmptyPayloadOffsets[] = {31, 47, 71};
    for (const auto Offset : EmptyPayloadOffsets)
    {
      SCOPED_TRACE(Offset);
      auto Bytes = emptyObjectGolden();
      overwriteLittleInteger(Bytes, Offset, std::numeric_limits<std::uint64_t>::max(), 8);
      EXPECT_FALSE(ObjectFileReader::read(Bytes).has_value());
    }
    const std::size_t StringLengthOffsets[] = {92, 141};
    for (const auto Offset : StringLengthOffsets)
    {
      SCOPED_TRACE(Offset);
      auto Bytes = littleEndianObjectGolden();
      overwriteLittleInteger(Bytes, Offset, std::numeric_limits<std::uint64_t>::max(), 8);
      EXPECT_FALSE(ObjectFileReader::read(Bytes).has_value());
    }
  }

  // Record counts beyond available bytes and index capacity fail before allocating or iterating the claimed records.
  TEST_F(VMObjectFileIOTest, RejectsOversizedRecordCounts)
  {
    const std::size_t EmptyCountOffsets[] = {79, 87};
    for (const auto Offset : EmptyCountOffsets)
    {
      SCOPED_TRACE(Offset);
      auto Bytes = emptyObjectGolden();
      overwriteLittleInteger(Bytes, Offset, std::numeric_limits<std::uint64_t>::max(), 8);
      EXPECT_FALSE(ObjectFileReader::read(Bytes).has_value());
    }
    const std::size_t DebugCountOffsets[] = {133, 150};
    for (const auto Offset : DebugCountOffsets)
    {
      SCOPED_TRACE(Offset);
      auto Bytes = littleEndianObjectGolden();
      overwriteLittleInteger(Bytes, Offset, std::numeric_limits<std::uint64_t>::max(), 8);
      EXPECT_FALSE(ObjectFileReader::read(Bytes).has_value());
    }
  }

  // Reference counts require four available bytes per record before the reader allocates the table.
  TEST_F(VMObjectFileIOTest, RejectsReferenceCountsWithoutCompleteIntegerPayloads)
  {
    auto Bytes = emptyObjectGolden();
    overwriteLittleInteger(Bytes, 87, 1, 8);
    Bytes.resize(98, 0);
    const auto Decoded = ObjectFileReader::read(Bytes);
    ASSERT_FALSE(Decoded.has_value());
    EXPECT_EQ(Decoded.error().Kind, ObjectFileIOErrorKind::UnexpectedEnd);
    EXPECT_EQ(Decoded.error().ByteOffset, 87U);
  }

  // The u32 index space allows 2^32 records, while a larger count fails as an unrepresentable table.
  TEST_F(VMObjectFileIOTest, ChecksReferenceCountIndexCapacityBeforeAllocation)
  {
    auto Bytes = emptyObjectGolden();
    const auto MaximumCount = static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max()) + 1;
    overwriteLittleInteger(Bytes, 87, MaximumCount, 8);
    const auto MissingPayload = ObjectFileReader::read(Bytes);
    ASSERT_FALSE(MissingPayload.has_value());
    EXPECT_EQ(MissingPayload.error().Kind, ObjectFileIOErrorKind::UnexpectedEnd);
    EXPECT_EQ(MissingPayload.error().ByteOffset, 87U);
    overwriteLittleInteger(Bytes, 87, MaximumCount + 1, 8);
    const auto Overflow = ObjectFileReader::read(Bytes);
    ASSERT_FALSE(Overflow.has_value());
    EXPECT_EQ(Overflow.error().Kind, ObjectFileIOErrorKind::InvalidObject);
    ASSERT_TRUE(Overflow.error().ValidationError.has_value());
    EXPECT_EQ(Overflow.error().ValidationError->Kind, ObjectValidationErrorKind::TooManyRecords);
  }

  // Each missing byte of a symbol-index payload is rejected rather than treated as an absent reference.
  TEST_F(VMObjectFileIOTest, RejectsTruncatedSymbolReferenceIndices)
  {
    const auto Golden = littleEndianObjectGolden();
    for (std::size_t Length = 128; Length < 132; ++Length)
    {
      SCOPED_TRACE(Length);
      const auto Decoded = ObjectFileReader::read(std::span<const std::uint8_t>(Golden.data(), Length));
      ASSERT_FALSE(Decoded.has_value());
      EXPECT_EQ(Decoded.error().Kind, ObjectFileIOErrorKind::UnexpectedEnd);
      EXPECT_EQ(Decoded.error().ByteOffset, 120U);
    }
  }

  // Decoded references and alignments are structurally validated and preserve the validator's record diagnosis.
  TEST_F(VMObjectFileIOTest, RejectsSemanticallyInvalidReferencesAndAlignments)
  {
    struct Case
    {
        std::size_t Offset;
        std::uint64_t Value;
        std::size_t ByteCount;
        ObjectValidationErrorKind Kind;
        std::uint64_t RecordIndex;
    };
    const Case Cases[] = {
        {41, 3, 8, ObjectValidationErrorKind::InvalidConstantSection, 0},
        {58, 3, 8, ObjectValidationErrorKind::InvalidGlobalStorage, 0},
        {128, 1, 4, ObjectValidationErrorKind::InvalidBaseReference, 0},
    };
    for (const auto &Entry : Cases)
    {
      SCOPED_TRACE(Entry.Offset);
      auto Bytes = littleEndianObjectGolden();
      overwriteLittleInteger(Bytes, Entry.Offset, Entry.Value, Entry.ByteCount);
      const auto Decoded = ObjectFileReader::read(Bytes);
      ASSERT_FALSE(Decoded.has_value());
      EXPECT_EQ(Decoded.error().Kind, ObjectFileIOErrorKind::InvalidObject);
      ASSERT_TRUE(Decoded.error().ValidationError.has_value());
      EXPECT_EQ(Decoded.error().ValidationError->Kind, Entry.Kind);
      EXPECT_EQ(Decoded.error().ValidationError->RecordIndex, Entry.RecordIndex);
    }
  }

  // Address width and both configured version triplets are validated after wire decoding.
  TEST_F(VMObjectFileIOTest, RejectsSemanticallyInvalidHeaderVersionsAndAddressWidth)
  {
    const std::size_t HeaderOffsets[] = {5, 7, 19};
    for (const auto Offset : HeaderOffsets)
    {
      SCOPED_TRACE(Offset);
      auto Bytes = emptyObjectGolden();
      Bytes[Offset] = Offset == 5 ? 32 : 3;
      const auto Decoded = ObjectFileReader::read(Bytes);
      ASSERT_FALSE(Decoded.has_value());
      EXPECT_EQ(Decoded.error().Kind, ObjectFileIOErrorKind::InvalidObject);
      ASSERT_TRUE(Decoded.error().ValidationError.has_value());
      EXPECT_EQ(Decoded.error().ValidationError->Kind, ObjectValidationErrorKind::InvalidHeader);
    }
  }

  // Version 1 objects carrying reserved register slots are rejected before their obsolete records are decoded.
  TEST_F(VMObjectFileIOTest, RejectsLegacyObjectFormatWithReservedRegisterSlots)
  {
    auto Bytes = emptyObjectGolden();
    overwriteLittleInteger(Bytes, 7, 1, 4);
    overwriteLittleInteger(Bytes, 87, 3, 8);
    Bytes.insert(Bytes.end() - 1, 3, 0);
    ASSERT_EQ(Bytes.size(), 99U);
    const auto Decoded = ObjectFileReader::read(Bytes);
    ASSERT_FALSE(Decoded.has_value());
    EXPECT_EQ(Decoded.error().Kind, ObjectFileIOErrorKind::InvalidObject);
    ASSERT_TRUE(Decoded.error().ValidationError.has_value());
    EXPECT_EQ(Decoded.error().ValidationError->Kind, ObjectValidationErrorKind::InvalidHeader);
  }

  // Invalid source objects are rejected with their validation error before binary encoding begins.
  TEST_F(VMObjectFileIOTest, WriterRejectsInvalidObject)
  {
    auto Object = makeCompleteIOObject(ObjectByteOrder::Little);
    Object.getBaseReferences().getEntries()[0].SymbolIndex = static_cast<std::uint32_t>(Object.getSymbols().getEntries().size());
    const auto Encoded = ObjectFileWriter::write(Object);
    ASSERT_FALSE(Encoded.has_value());
    EXPECT_EQ(Encoded.error().Kind, ObjectFileIOErrorKind::InvalidObject);
    ASSERT_TRUE(Encoded.error().ValidationError.has_value());
    EXPECT_EQ(Encoded.error().ValidationError->Kind, ObjectValidationErrorKind::InvalidBaseReference);
    EXPECT_EQ(Encoded.error().ValidationError->RecordIndex, 0U);
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
      const auto Written = ObjectFileWriter::writeFile(Object, Directory.filePath());
      ASSERT_TRUE(Written.has_value());
      const auto Read = ObjectFileReader::readFile(Directory.filePath());
      ASSERT_TRUE(Read.has_value());
      expectSameObjectContents(Object, *Read);
    }
  }

  // Missing files, absent parent directories, and directory paths fail through explicit file I/O results.
  TEST_F(VMObjectFileIOTest, ReportsFileOpenFailures)
  {
    ScopedObjectIODirectory Directory;
    ASSERT_TRUE(Directory.valid());
    const auto Missing = ObjectFileReader::readFile(Directory.filePath());
    ASSERT_FALSE(Missing.has_value());
    EXPECT_EQ(Missing.error().Kind, ObjectFileIOErrorKind::FileOpenFailed);
    const ObjectFile Object;
    const auto MissingParent = ObjectFileWriter::writeFile(Object, Directory.directoryPath() / "missing" / "object.inko");
    ASSERT_FALSE(MissingParent.has_value());
    EXPECT_EQ(MissingParent.error().Kind, ObjectFileIOErrorKind::FileOpenFailed);
    EXPECT_FALSE(ObjectFileReader::readFile(Directory.directoryPath()).has_value());
    EXPECT_FALSE(ObjectFileWriter::writeFile(Object, Directory.directoryPath()).has_value());
  }

  // Rejected objects leave an existing file's bytes intact instead of opening it in truncation mode first.
  TEST_F(VMObjectFileIOTest, InvalidObjectDoesNotTruncateExistingFile)
  {
    ScopedObjectIODirectory Directory;
    ASSERT_TRUE(Directory.valid());
    const std::array<char, 4> Sentinel = {'k', 'e', 'e', 'p'};
    {
      std::ofstream Output(Directory.filePath(), std::ios::binary | std::ios::trunc);
      ASSERT_TRUE(Output.is_open());
      Output.write(Sentinel.data(), static_cast<std::streamsize>(Sentinel.size()));
      ASSERT_TRUE(Output.good());
    }
    ObjectFile Invalid;
    Invalid.getBaseReferences().getEntries().push_back({0});
    const auto Written = ObjectFileWriter::writeFile(Invalid, Directory.filePath());
    ASSERT_FALSE(Written.has_value());
    EXPECT_EQ(Written.error().Kind, ObjectFileIOErrorKind::InvalidObject);
    std::ifstream Input(Directory.filePath(), std::ios::binary);
    ASSERT_TRUE(Input.is_open());
    std::array<char, 4> Actual{};
    Input.read(Actual.data(), static_cast<std::streamsize>(Actual.size()));
    EXPECT_EQ(Input.gcount(), static_cast<std::streamsize>(Sentinel.size()));
    EXPECT_EQ(Actual, Sentinel);
    EXPECT_EQ(Input.peek(), std::ifstream::traits_type::eof());
  }
} // namespace ink::vm::test
