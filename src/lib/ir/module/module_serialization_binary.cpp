#include "module_serialization_internal.h"
#include <llvm/Support/Endian.h>
#include <cstring>

namespace ink::ir::archive
{
  namespace
  {
    constexpr std::size_t HeaderBytes = 16;
    constexpr std::size_t RecordBytes = 24;
  } // namespace

  bool readBinary(std::string_view Bytes, State &Data)
  {
    if (Bytes.size() < HeaderBytes || Bytes.substr(0, 4) != "IIRB")
    {
      return Data.fail("Invalid module binary signature or truncated header");
    }
    if (llvm::support::endian::read32le(Bytes.data() + 4) != ModuleBinaryVersion)
    {
      return Data.fail("Unsupported module binary version", ModuleArchiveStatus::UnsupportedVersion);
    }
    const auto Count = llvm::support::endian::read32le(Bytes.data() + 8);
    if (llvm::support::endian::read32le(Bytes.data() + 12) != 0)
    {
      return Data.fail("Unknown module binary flags");
    }
    Bytes.remove_prefix(HeaderBytes);
    if (Count > Data.Limits.MaxObjects)
    {
      return Data.fail("Module archive object limit exceeded", ModuleArchiveStatus::LimitExceeded);
    }
    if (Count > Bytes.size() / RecordBytes)
    {
      return Data.fail("Truncated module record table");
    }
    if (Count > Data.Limits.MaxAllocationBytes / 1024)
    {
      return Data.fail("Module archive allocation limit exceeded", ModuleArchiveStatus::LimitExceeded);
    }
    Data.Records.reserve(Count);
    for (std::uint32_t I = 0; I < Count; ++I)
    {
      if (Bytes.size() < RecordBytes)
      {
        return Data.fail("Truncated module record header");
      }
      const auto *Info = tagInfo(llvm::support::endian::read32le(Bytes.data()));
      const auto Type = llvm::support::endian::read32le(Bytes.data() + 4);
      const auto Parent = llvm::support::endian::read32le(Bytes.data() + 8);
      const auto Fields = llvm::support::endian::read32le(Bytes.data() + 12);
      const auto Length = llvm::support::endian::read64le(Bytes.data() + 16);
      Bytes.remove_prefix(RecordBytes);
      if (!Info || Fields < Info->MinFields || Fields > Info->MaxFields || Fields > Bytes.size() / 8 || Length > Bytes.size() - static_cast<std::size_t>(Fields) * 8 || (!Info->HasText && Length))
      {
        return Data.fail("Invalid module record kind, fields or payload length");
      }
      if (!Data.addRecord() || !Data.fields(Fields) || !Data.string(static_cast<std::size_t>(Length)))
      {
        return false;
      }
      auto &Entry = Data.Records.back();
      Entry.Kind = Info->Kind;
      Entry.Type = Type;
      Entry.Parent = Parent;
      Entry.Fields.resize(Fields);
      for (std::size_t Field = 0; Field < Fields; ++Field)
      {
        Entry.Fields[Field] = llvm::support::endian::read64le(Bytes.data() + Field * 8);
      }
      Bytes.remove_prefix(static_cast<std::size_t>(Fields) * 8);
      Entry.Text.assign(Bytes.data(), static_cast<std::size_t>(Length));
      Bytes.remove_prefix(static_cast<std::size_t>(Length));
    }
    return Bytes.empty() || Data.fail("Trailing module binary data");
  }

  std::string writeBinary(State &Data)
  {
    std::size_t Size = HeaderBytes;
    if (Data.Records.size() > UINT32_MAX)
    {
      Data.fail("Module binary object index exceeds 32 bits", ModuleArchiveStatus::LimitExceeded);
      return {};
    }
    for (const auto &Entry : Data.Records)
    {
      if (Entry.Type > UINT32_MAX || Entry.Parent > UINT32_MAX || Entry.Fields.size() > UINT32_MAX || Size > Data.Limits.MaxArchiveBytes || RecordBytes > Data.Limits.MaxArchiveBytes - Size)
      {
        Data.fail("Module binary size or index limit exceeded", ModuleArchiveStatus::LimitExceeded);
        return {};
      }
      Size += RecordBytes;
      if (Entry.Fields.size() > (Data.Limits.MaxArchiveBytes - Size) / 8)
      {
        Data.fail("Module archive byte limit exceeded", ModuleArchiveStatus::LimitExceeded);
        return {};
      }
      Size += Entry.Fields.size() * 8;
      if (Entry.Text.size() > Data.Limits.MaxArchiveBytes - Size)
      {
        Data.fail("Module archive byte limit exceeded", ModuleArchiveStatus::LimitExceeded);
        return {};
      }
      Size += Entry.Text.size();
    }
    if (Size > Data.Limits.MaxArchiveBytes || !Data.charge(Size, 1))
    {
      Data.fail("Module archive byte limit exceeded", ModuleArchiveStatus::LimitExceeded);
      return {};
    }
    // One allocation, fixed-width scalars, and one bulk copy per string or AST payload.
    std::string Output(Size, '\0');
    char *Cursor = Output.data();
    std::memcpy(Cursor, "IIRB", 4);
    llvm::support::endian::write32le(Cursor + 4, ModuleBinaryVersion);
    llvm::support::endian::write32le(Cursor + 8, static_cast<std::uint32_t>(Data.Records.size()));
    Cursor += HeaderBytes;
    for (const auto &Entry : Data.Records)
    {
      llvm::support::endian::write32le(Cursor, static_cast<unsigned>(Entry.Kind));
      llvm::support::endian::write32le(Cursor + 4, static_cast<std::uint32_t>(Entry.Type));
      llvm::support::endian::write32le(Cursor + 8, static_cast<std::uint32_t>(Entry.Parent));
      llvm::support::endian::write32le(Cursor + 12, static_cast<std::uint32_t>(Entry.Fields.size()));
      llvm::support::endian::write64le(Cursor + 16, Entry.Text.size());
      Cursor += RecordBytes;
      for (auto Field : Entry.Fields)
      {
        llvm::support::endian::write64le(Cursor, Field);
        Cursor += 8;
      }
      std::memcpy(Cursor, Entry.Text.data(), Entry.Text.size());
      Cursor += Entry.Text.size();
    }
    return Output;
  }
} // namespace ink::ir::archive
