#include "ink/ir/serialization/serialization.h"
#include "model.h"

#include <array>
#include <limits>
#include <unordered_map>
#include <utility>

namespace ink::ir
{
  namespace
  {
    constexpr std::array<unsigned char, 8> Magic = {'I', 'N', 'K', 'I', 'R', 'B', '\r', '\n'};
    constexpr std::uint32_t Version = 1;
    constexpr std::size_t HeaderSize = 32;
    constexpr std::size_t DirectoryEntrySize = 32;
    constexpr std::size_t SectionCount = 4;
    constexpr std::size_t TargetSize = 56;
    constexpr std::size_t RecordSize = 32;
    constexpr std::size_t StringEntrySize = 16;

    struct Section
    {
        std::uint64_t Offset = 0;
        std::uint64_t Size = 0;
        std::uint64_t Count = 0;
    };

    IRSerializationError failure(IRSerializationErrorCode Code, std::size_t Offset, std::string Message)
    {
      return {Code, Offset, std::move(Message)};
    }

    void append32(std::vector<std::byte> &Bytes, std::uint32_t Value)
    {
      for (unsigned Shift = 0; Shift < 32; Shift += 8)
      {
        Bytes.push_back(static_cast<std::byte>((Value >> Shift) & 255U));
      }
    }

    void append64(std::vector<std::byte> &Bytes, std::uint64_t Value)
    {
      for (unsigned Shift = 0; Shift < 64; Shift += 8)
      {
        Bytes.push_back(static_cast<std::byte>((Value >> Shift) & 255U));
      }
    }

    // All callers establish the containing span's bounds before reading fixed-width fields.
    std::uint32_t read32(std::span<const std::byte> Bytes, std::size_t Offset)
    {
      std::uint32_t Value = 0;
      for (unsigned Index = 0; Index < 4; ++Index)
      {
        Value |= std::uint32_t(std::to_integer<unsigned char>(Bytes[Offset + Index])) << (Index * 8U);
      }
      return Value;
    }

    std::uint64_t read64(std::span<const std::byte> Bytes, std::size_t Offset)
    {
      std::uint64_t Value = 0;
      for (unsigned Index = 0; Index < 8; ++Index)
      {
        Value |= std::uint64_t(std::to_integer<unsigned char>(Bytes[Offset + Index])) << (Index * 8U);
      }
      return Value;
    }

    bool addWithin(std::uint64_t &Total, std::uint64_t Amount, std::uint64_t Limit)
    {
      if (Total > Limit || Amount > Limit - Total)
      {
        return false;
      }
      Total += Amount;
      return true;
    }

    IRSerializationResult<std::vector<std::byte>> encodeBinary(const detail::Model &Data, const IRBinaryWriteOptions &Options)
    {
      std::vector<std::string_view> Strings;
      std::unordered_map<std::string_view, std::uint32_t> StringIds;
      std::uint64_t StringBytes = 0;
      std::uint64_t OperandBytes = 0;
      const auto Limit = static_cast<std::uint64_t>(Options.MaxBytes);
      if (Data.Records.size() >= detail::NullId)
      {
        return std::unexpected(failure(IRSerializationErrorCode::ResourceLimit, 0, "too many IR records"));
      }
      for (const auto &Record : Data.Records)
      {
        if (Record.Numbers.size() > std::numeric_limits<std::uint32_t>::max() || Record.References.size() > std::numeric_limits<std::uint32_t>::max() || Record.Strings.size() > std::numeric_limits<std::uint32_t>::max())
        {
          return std::unexpected(failure(IRSerializationErrorCode::ResourceLimit, 0, "too many record operands"));
        }
        if (!addWithin(OperandBytes, std::uint64_t(Record.Numbers.size()) * 8, Limit) || !addWithin(OperandBytes, std::uint64_t(Record.References.size()) * 4, Limit) || !addWithin(OperandBytes, std::uint64_t(Record.Strings.size()) * 4, Limit))
        {
          return std::unexpected(failure(IRSerializationErrorCode::ResourceLimit, 0, "binary output exceeds MaxBytes"));
        }
        for (const auto &String : Record.Strings)
        {
          if (StringIds.contains(String))
          {
            continue;
          }
          if (Strings.size() >= detail::NullId || !addWithin(StringBytes, String.size(), Limit))
          {
            return std::unexpected(failure(IRSerializationErrorCode::ResourceLimit, 0, "binary string table exceeds limits"));
          }
          StringIds.emplace(String, static_cast<std::uint32_t>(Strings.size()));
          Strings.push_back(String);
        }
      }

      const std::array<std::uint64_t, SectionCount> Sizes = {TargetSize, std::uint64_t(Strings.size()) * StringEntrySize + StringBytes, std::uint64_t(Data.Records.size()) * RecordSize, OperandBytes};
      const std::array<std::uint64_t, SectionCount> Counts = {1, Strings.size(), Data.Records.size(), OperandBytes};
      std::array<Section, SectionCount> Sections;
      std::uint64_t Total = HeaderSize + SectionCount * DirectoryEntrySize;
      for (std::size_t Index = 0; Index < SectionCount; ++Index)
      {
        Sections[Index] = {Total, Sizes[Index], Counts[Index]};
        if (!addWithin(Total, Sizes[Index], Limit))
        {
          return std::unexpected(failure(IRSerializationErrorCode::ResourceLimit, 0, "binary output exceeds MaxBytes"));
        }
      }

      std::vector<std::byte> Bytes;
      Bytes.reserve(static_cast<std::size_t>(Total));
      for (unsigned char Character : Magic)
      {
        Bytes.push_back(static_cast<std::byte>(Character));
      }
      append32(Bytes, Version);
      append32(Bytes, 0);
      append64(Bytes, Total);
      append32(Bytes, SectionCount);
      append32(Bytes, 0);
      for (std::size_t Index = 0; Index < SectionCount; ++Index)
      {
        append32(Bytes, static_cast<std::uint32_t>(Index + 1));
        append32(Bytes, 0);
        append64(Bytes, Sections[Index].Offset);
        append64(Bytes, Sections[Index].Size);
        append64(Bytes, Sections[Index].Count);
      }
      append32(Bytes, Data.PointerBits);
      append32(Bytes, Data.ByteOrder);
      append32(Bytes, Data.NativeAbi ? 1U : 0U);
      append32(Bytes, 0);
      for (std::uint64_t Alignment : Data.Alignments)
      {
        append64(Bytes, Alignment);
      }

      std::uint64_t StringOffset = Strings.size() * StringEntrySize;
      for (std::string_view String : Strings)
      {
        append64(Bytes, StringOffset);
        append64(Bytes, String.size());
        StringOffset += String.size();
      }
      for (std::string_view String : Strings)
      {
        for (unsigned char Character : String)
        {
          Bytes.push_back(static_cast<std::byte>(Character));
        }
      }

      std::uint64_t OperandOffset = 0;
      for (const auto &Record : Data.Records)
      {
        const std::uint64_t Length = Record.Numbers.size() * 8ULL + Record.References.size() * 4ULL + Record.Strings.size() * 4ULL;
        append32(Bytes, static_cast<std::uint32_t>(Record.Kind));
        append32(Bytes, static_cast<std::uint32_t>(Record.Numbers.size()));
        append32(Bytes, static_cast<std::uint32_t>(Record.References.size()));
        append32(Bytes, static_cast<std::uint32_t>(Record.Strings.size()));
        append64(Bytes, OperandOffset);
        append64(Bytes, Length);
        OperandOffset += Length;
      }
      for (const auto &Record : Data.Records)
      {
        for (std::uint64_t Number : Record.Numbers)
        {
          append64(Bytes, Number);
        }
        for (std::uint32_t Reference : Record.References)
        {
          append32(Bytes, Reference);
        }
        for (const auto &String : Record.Strings)
        {
          append32(Bytes, StringIds.find(String)->second);
        }
      }
      return Bytes;
    }

    IRSerializationResult<detail::Model> parseBinary(std::span<const std::byte> Bytes, const IRReadOptions &Options)
    {
      if (Bytes.size() > Options.MaxBytes)
      {
        return std::unexpected(failure(IRSerializationErrorCode::ResourceLimit, 0, "binary input exceeds MaxBytes"));
      }
      if (Bytes.size() < HeaderSize)
      {
        return std::unexpected(failure(IRSerializationErrorCode::TruncatedInput, Bytes.size(), "incomplete binary header"));
      }
      for (std::size_t Index = 0; Index < Magic.size(); ++Index)
      {
        if (std::to_integer<unsigned char>(Bytes[Index]) != Magic[Index])
        {
          return std::unexpected(failure(IRSerializationErrorCode::InvalidMagic, Index, "invalid Ink IR binary magic"));
        }
      }
      if (read32(Bytes, 8) != Version)
      {
        return std::unexpected(failure(IRSerializationErrorCode::UnsupportedVersion, 8, "unsupported Ink IR binary version"));
      }
      if (read32(Bytes, 12) != 0 || read32(Bytes, 28) != 0)
      {
        return std::unexpected(failure(IRSerializationErrorCode::UnsupportedFeature, 12, "unknown binary header flags"));
      }
      if (read64(Bytes, 16) != Bytes.size())
      {
        return std::unexpected(failure(IRSerializationErrorCode::TruncatedInput, 16, "binary file size does not match its header"));
      }
      if (read32(Bytes, 24) != SectionCount || Bytes.size() < HeaderSize + SectionCount * DirectoryEntrySize)
      {
        return std::unexpected(failure(IRSerializationErrorCode::InvalidSyntax, 24, "invalid binary section directory"));
      }

      std::array<Section, SectionCount> Sections;
      std::uint64_t NextOffset = HeaderSize + SectionCount * DirectoryEntrySize;
      for (std::size_t Index = 0; Index < SectionCount; ++Index)
      {
        const std::size_t EntryOffset = HeaderSize + Index * DirectoryEntrySize;
        if (read32(Bytes, EntryOffset) != Index + 1 || read32(Bytes, EntryOffset + 4) != 0)
        {
          return std::unexpected(failure(IRSerializationErrorCode::UnsupportedFeature, EntryOffset, "unknown or reordered binary section"));
        }
        auto &Entry = Sections[Index];
        Entry = {read64(Bytes, EntryOffset + 8), read64(Bytes, EntryOffset + 16), read64(Bytes, EntryOffset + 24)};
        if (Entry.Offset != NextOffset || !addWithin(NextOffset, Entry.Size, Bytes.size()))
        {
          return std::unexpected(failure(IRSerializationErrorCode::InvalidSyntax, EntryOffset, "overlapping or out-of-bounds binary section"));
        }
      }
      if (NextOffset != Bytes.size() || Sections[0].Size != TargetSize || Sections[0].Count != 1 || Sections[3].Count != Sections[3].Size)
      {
        return std::unexpected(failure(IRSerializationErrorCode::InvalidSyntax, HeaderSize, "inconsistent binary section sizes"));
      }
      const auto &StringSection = Sections[1];
      const auto &RecordSection = Sections[2];
      if (StringSection.Count >= detail::NullId || StringSection.Count > Options.MaxOperands || RecordSection.Count >= detail::NullId || RecordSection.Count > Options.MaxRecords)
      {
        return std::unexpected(failure(IRSerializationErrorCode::ResourceLimit, HeaderSize, "binary table count exceeds limits"));
      }
      if (StringSection.Count > StringSection.Size / StringEntrySize || RecordSection.Count > RecordSection.Size / RecordSize || RecordSection.Count * RecordSize != RecordSection.Size)
      {
        return std::unexpected(failure(IRSerializationErrorCode::InvalidSyntax, HeaderSize, "truncated binary index table"));
      }

      detail::Model Data;
      const auto TargetOffset = static_cast<std::size_t>(Sections[0].Offset);
      Data.PointerBits = read32(Bytes, TargetOffset);
      Data.ByteOrder = read32(Bytes, TargetOffset + 4);
      const auto NativeAbi = read32(Bytes, TargetOffset + 8);
      if (NativeAbi > 1 || read32(Bytes, TargetOffset + 12) != 0)
      {
        return std::unexpected(failure(IRSerializationErrorCode::InvalidSyntax, TargetOffset + 8, "invalid target flags"));
      }
      Data.NativeAbi = NativeAbi != 0;
      for (std::size_t Index = 0; Index < Data.Alignments.size(); ++Index)
      {
        Data.Alignments[Index] = read64(Bytes, TargetOffset + 16 + Index * 8);
      }

      std::vector<std::string_view> Strings;
      Strings.reserve(static_cast<std::size_t>(StringSection.Count));
      std::uint64_t NextString = StringSection.Count * StringEntrySize;
      std::uint64_t TotalStringBytes = 0;
      for (std::uint64_t Index = 0; Index < StringSection.Count; ++Index)
      {
        const auto Offset = static_cast<std::size_t>(StringSection.Offset + Index * StringEntrySize);
        const auto Start = read64(Bytes, Offset);
        const auto Length = read64(Bytes, Offset + 8);
        if (Start != NextString || !addWithin(NextString, Length, StringSection.Size))
        {
          return std::unexpected(failure(IRSerializationErrorCode::InvalidSyntax, Offset, "invalid binary string span"));
        }
        if (!addWithin(TotalStringBytes, Length, Options.MaxStringBytes))
        {
          return std::unexpected(failure(IRSerializationErrorCode::ResourceLimit, Offset, "binary strings exceed MaxStringBytes"));
        }
        const auto *Characters = reinterpret_cast<const char *>(Bytes.data() + static_cast<std::size_t>(StringSection.Offset + Start));
        Strings.emplace_back(Characters, static_cast<std::size_t>(Length));
      }
      if (NextString != StringSection.Size)
      {
        return std::unexpected(failure(IRSerializationErrorCode::InvalidSyntax, static_cast<std::size_t>(StringSection.Offset), "unindexed binary string bytes"));
      }

      Data.Records.reserve(static_cast<std::size_t>(RecordSection.Count));
      std::uint64_t NextOperand = 0;
      std::uint64_t TotalOperands = 0;
      std::uint64_t ExpandedStringBytes = 0;
      for (std::uint64_t Index = 0; Index < RecordSection.Count; ++Index)
      {
        const auto Offset = static_cast<std::size_t>(RecordSection.Offset + Index * RecordSize);
        detail::Record Record;
        Record.Kind = static_cast<detail::RecordKind>(read32(Bytes, Offset));
        if (!detail::isValidRecordKind(Record.Kind))
        {
          return std::unexpected(failure(IRSerializationErrorCode::UnsupportedKind, Offset, "unknown binary IR record kind"));
        }
        const auto NumberCount = read32(Bytes, Offset + 4);
        const auto ReferenceCount = read32(Bytes, Offset + 8);
        const auto StringCount = read32(Bytes, Offset + 12);
        const auto PayloadOffset = read64(Bytes, Offset + 16);
        const auto Length = read64(Bytes, Offset + 24);
        const std::uint64_t ExpectedLength = std::uint64_t(NumberCount) * 8 + std::uint64_t(ReferenceCount) * 4 + std::uint64_t(StringCount) * 4;
        if (PayloadOffset != NextOperand || Length != ExpectedLength || !addWithin(NextOperand, Length, Sections[3].Size))
        {
          return std::unexpected(failure(IRSerializationErrorCode::InvalidSyntax, Offset, "invalid binary record payload"));
        }
        if (!addWithin(TotalOperands, std::uint64_t(NumberCount) + ReferenceCount + StringCount, Options.MaxOperands))
        {
          return std::unexpected(failure(IRSerializationErrorCode::ResourceLimit, Offset, "binary operands exceed MaxOperands"));
        }
        Record.Numbers.reserve(NumberCount);
        Record.References.reserve(ReferenceCount);
        Record.Strings.reserve(StringCount);
        auto Cursor = static_cast<std::size_t>(Sections[3].Offset + PayloadOffset);
        for (std::uint32_t Item = 0; Item < NumberCount; ++Item)
        {
          Record.Numbers.push_back(read64(Bytes, Cursor));
          Cursor += 8;
        }
        for (std::uint32_t Item = 0; Item < ReferenceCount; ++Item)
        {
          Record.References.push_back(read32(Bytes, Cursor));
          Cursor += 4;
        }
        for (std::uint32_t Item = 0; Item < StringCount; ++Item)
        {
          const auto StringId = read32(Bytes, Cursor);
          if (StringId >= Strings.size())
          {
            return std::unexpected(failure(IRSerializationErrorCode::InvalidReference, Cursor, "invalid binary string ID"));
          }
          if (!addWithin(ExpandedStringBytes, Strings[StringId].size(), Options.MaxStringBytes))
          {
            return std::unexpected(failure(IRSerializationErrorCode::ResourceLimit, Cursor, "expanded binary strings exceed MaxStringBytes"));
          }
          Record.Strings.emplace_back(Strings[StringId]);
          Cursor += 4;
        }
        Data.Records.push_back(std::move(Record));
      }
      if (NextOperand != Sections[3].Size)
      {
        return std::unexpected(failure(IRSerializationErrorCode::InvalidSyntax, static_cast<std::size_t>(Sections[3].Offset), "unindexed binary operand bytes"));
      }
      return Data;
    }
  } // namespace

  IRSerializationResult<std::vector<std::byte>> IRBinaryWriter::write(const Module &ModuleValue, const IRBinaryWriteOptions &Options)
  {
    auto Verified = IRVerifier::verify(ModuleValue);
    if (!Verified)
    {
      return std::unexpected(std::move(Verified.error()));
    }
    auto Data = detail::collectModel(ModuleValue);
    if (!Data)
    {
      return std::unexpected(std::move(Data.error()));
    }
    return encodeBinary(*Data, Options);
  }

  IRSerializationResult<std::unique_ptr<Module>> IRBinaryReader::read(IRContext &Context, std::span<const std::byte> Bytes, const IRReadOptions &Options)
  {
    auto Data = parseBinary(Bytes, Options);
    if (!Data)
    {
      return std::unexpected(std::move(Data.error()));
    }
    return detail::decodeModel(Context, *Data, Options);
  }

  IRSerializationResult<std::vector<std::byte>> writeBinary(const Module &ModuleValue, const IRBinaryWriteOptions &Options)
  {
    return IRBinaryWriter::write(ModuleValue, Options);
  }

  IRSerializationResult<std::unique_ptr<Module>> readBinary(IRContext &Context, std::span<const std::byte> Bytes, const IRReadOptions &Options)
  {
    return IRBinaryReader::read(Context, Bytes, Options);
  }
} // namespace ink::ir
