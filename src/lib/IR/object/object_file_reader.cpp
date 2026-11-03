#include "ink/ir/object/object_file_reader.h"

#include "ink/core/support/version.h"
#include "ink/ir/object/object_file.h"
#include "ink/ir/object/object_file_io_error.h"
#include "ink/ir/object/object_file_validator.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <fstream>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace ink::ir
{
  namespace
  {
    class ObjectInput final
    {
      public:
        explicit ObjectInput(std::span<const std::uint8_t> Bytes) noexcept
            : Bytes(Bytes)
        {
        }

        std::size_t offset() const noexcept
        {
          return Cursor;
        }

        std::size_t remaining() const noexcept
        {
          return Bytes.size() - Cursor;
        }

        const ObjectFileIOError &error() const noexcept
        {
          return *Failure;
        }

        void setByteOrder(ObjectByteOrder Value) noexcept
        {
          ByteOrder = Value;
        }

        template <typename Integer>
        bool readInteger(Integer &Value) noexcept
        {
          const auto Start = Cursor;
          if (remaining() < sizeof(Integer))
          {
            return fail(ObjectFileIOErrorKind::UnexpectedEnd, Start);
          }
          std::uint64_t Bits = 0;
          for (std::size_t Index = 0; Index < sizeof(Integer); ++Index)
          {
            const auto Shift = (ByteOrder == ObjectByteOrder::Little ? Index : sizeof(Integer) - Index - 1) * 8;
            Bits |= static_cast<std::uint64_t>(Bytes[Cursor + Index]) << Shift;
          }
          Cursor += sizeof(Integer);
          Value = static_cast<Integer>(Bits);
          return true;
        }

        template <typename Enum>
        bool readEnum(Enum &Value, std::uint8_t Maximum) noexcept
        {
          const auto Start = Cursor;
          std::uint8_t Encoded = 0;
          if (!readInteger(Encoded))
          {
            return false;
          }
          if (Encoded > Maximum)
          {
            return fail(ObjectFileIOErrorKind::InvalidEncoding, Start);
          }
          Value = static_cast<Enum>(Encoded);
          return true;
        }

        bool readMarker(bool &Value) noexcept
        {
          const auto Start = Cursor;
          std::uint8_t Encoded = 0;
          if (!readInteger(Encoded))
          {
            return false;
          }
          if (Encoded > 1)
          {
            return fail(ObjectFileIOErrorKind::InvalidEncoding, Start);
          }
          Value = Encoded != 0;
          return true;
        }

        bool readVersion(core::Version &Value) noexcept
        {
          return readInteger(Value.getMajor()) && readInteger(Value.getMinor()) && readInteger(Value.getPatch());
        }

        bool readBytes(std::vector<std::uint8_t> &Value)
        {
          const auto Start = Cursor;
          std::uint64_t Length = 0;
          if (!readInteger(Length))
          {
            return false;
          }
          if (Length > remaining())
          {
            return fail(ObjectFileIOErrorKind::UnexpectedEnd, Start);
          }
          if (Length > Value.max_size())
          {
            return fail(ObjectFileIOErrorKind::SizeOverflow, Start);
          }
          const auto Size = static_cast<std::size_t>(Length);
          const auto Payload = Bytes.subspan(Cursor, Size);
          Value.assign(Payload.begin(), Payload.end());
          Cursor += Size;
          return true;
        }

        bool readString(std::string &Value)
        {
          const auto Start = Cursor;
          std::uint64_t Length = 0;
          if (!readInteger(Length))
          {
            return false;
          }
          if (Length > remaining())
          {
            return fail(ObjectFileIOErrorKind::UnexpectedEnd, Start);
          }
          if (Length > Value.max_size())
          {
            return fail(ObjectFileIOErrorKind::SizeOverflow, Start);
          }
          if (Length == 0)
          {
            Value.clear();
            return true;
          }
          const auto Size = static_cast<std::size_t>(Length);
          Value.assign(reinterpret_cast<const char *>(Bytes.data() + Cursor), Size);
          Cursor += Size;
          return true;
        }

        template <typename Container>
        bool readCount(std::uint64_t &Count, std::size_t MinimumRecordSize, const Container &Values, bool Indexed) noexcept
        {
          const auto Start = Cursor;
          if (!readInteger(Count))
          {
            return false;
          }
          if (Indexed && Count > static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max()) + 1)
          {
            Failure = ObjectFileIOError{ObjectFileIOErrorKind::InvalidObject, 0, ObjectValidationError{ObjectValidationErrorKind::TooManyRecords, 0}};
            return false;
          }
          if (Count > remaining() / MinimumRecordSize)
          {
            return fail(ObjectFileIOErrorKind::UnexpectedEnd, Start);
          }
          if (Count > Values.max_size())
          {
            return fail(ObjectFileIOErrorKind::SizeOverflow, Start);
          }
          return true;
        }

      private:
        bool fail(ObjectFileIOErrorKind Kind, std::size_t Offset) noexcept
        {
          Failure = ObjectFileIOError{Kind, static_cast<std::uint64_t>(Offset), std::nullopt};
          return false;
        }

        std::span<const std::uint8_t> Bytes;
        std::size_t Cursor = 0;
        ObjectByteOrder ByteOrder = ObjectByteOrder::Little;
        std::optional<ObjectFileIOError> Failure;
    };
  } // namespace

  std::expected<ObjectFile, ObjectFileIOError> ObjectFileReader::read(std::span<const std::uint8_t> Bytes)
  {
    ObjectInput Input(Bytes);
    ObjectFile Object;
    auto &Header = Object.getHeader();
    for (auto &Byte : Header.getMagic())
    {
      if (!Input.readInteger(Byte))
      {
        return std::unexpected(Input.error());
      }
    }
    if (Header.getMagic() != ObjectFileMagic)
    {
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::InvalidObject, 0, ObjectValidationError{ObjectValidationErrorKind::InvalidHeader, 0}});
    }
    if (!Input.readEnum(Header.getByteOrder(), 1))
    {
      return std::unexpected(Input.error());
    }
    Input.setByteOrder(Header.getByteOrder());
    if (!Input.readInteger(Header.getAddressBitWidth()) || !Input.readVersion(Header.getFormatVersion()) || !Input.readVersion(Header.getInstructionSetVersion()))
    {
      return std::unexpected(Input.error());
    }
    if (const auto Error = ObjectFileValidator::validate(Object))
    {
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::InvalidObject, 0, Error});
    }

    if (!Input.readBytes(Object.getCode().getBytes()) || !Input.readInteger(Object.getConstants().getAlignment()) || !Input.readBytes(Object.getConstants().getBytes()) || !Input.readInteger(Object.getGlobalStorage().getAlignment()) || !Input.readInteger(Object.getGlobalStorage().getByteSize()) || !Input.readBytes(Object.getGlobalStorage().getInitialBytes()))
    {
      return std::unexpected(Input.error());
    }

    auto &Symbols = Object.getSymbols().getEntries();
    std::uint64_t SymbolCount = 0;
    if (!Input.readCount(SymbolCount, 10, Symbols, true))
    {
      return std::unexpected(Input.error());
    }
    Symbols.resize(static_cast<std::size_t>(SymbolCount));
    for (auto &Symbol : Symbols)
    {
      bool HasDefinition = false;
      if (!Input.readString(Symbol.Name) || !Input.readEnum(Symbol.Linkage, 2) || !Input.readMarker(HasDefinition))
      {
        return std::unexpected(Input.error());
      }
      if (HasDefinition)
      {
        ObjectSymbolDefinition Definition;
        if (!Input.readEnum(Definition.Section, 2) || !Input.readInteger(Definition.ByteOffset) || !Input.readInteger(Definition.ByteSize))
        {
          return std::unexpected(Input.error());
        }
        Symbol.Definition = Definition;
      }
    }

    auto &BaseReferences = Object.getBaseReferences().getEntries();
    std::uint64_t BaseReferenceCount = 0;
    if (!Input.readCount(BaseReferenceCount, 1, BaseReferences, true))
    {
      return std::unexpected(Input.error());
    }
    BaseReferences.clear();
    BaseReferences.resize(static_cast<std::size_t>(BaseReferenceCount));
    for (auto &Reference : BaseReferences)
    {
      bool HasSymbol = false;
      if (!Input.readMarker(HasSymbol))
      {
        return std::unexpected(Input.error());
      }
      if (HasSymbol)
      {
        std::uint32_t SymbolIndex = 0;
        if (!Input.readInteger(SymbolIndex))
        {
          return std::unexpected(Input.error());
        }
        Reference.SymbolIndex = SymbolIndex;
      }
    }

    bool HasDebugInfo = false;
    if (!Input.readMarker(HasDebugInfo))
    {
      return std::unexpected(Input.error());
    }
    if (HasDebugInfo)
    {
      auto &DebugInfo = Object.getDebugInfo().emplace();
      auto &SourceFiles = DebugInfo.getSourceFiles();
      std::uint64_t SourceFileCount = 0;
      if (!Input.readCount(SourceFileCount, 8, SourceFiles, true))
      {
        return std::unexpected(Input.error());
      }
      SourceFiles.resize(static_cast<std::size_t>(SourceFileCount));
      for (auto &SourceFile : SourceFiles)
      {
        if (!Input.readString(SourceFile))
        {
          return std::unexpected(Input.error());
        }
      }

      auto &Lines = DebugInfo.getLines();
      std::uint64_t LineCount = 0;
      if (!Input.readCount(LineCount, 20, Lines, false))
      {
        return std::unexpected(Input.error());
      }
      Lines.resize(static_cast<std::size_t>(LineCount));
      for (auto &Line : Lines)
      {
        if (!Input.readInteger(Line.CodeOffset) || !Input.readInteger(Line.FileIndex) || !Input.readInteger(Line.Line) || !Input.readInteger(Line.Column))
        {
          return std::unexpected(Input.error());
        }
      }
    }
    if (Input.remaining() != 0)
    {
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::TrailingData, static_cast<std::uint64_t>(Input.offset()), std::nullopt});
    }
    if (const auto Error = ObjectFileValidator::validate(Object))
    {
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::InvalidObject, 0, Error});
    }
    return std::move(Object);
  }

  std::expected<ObjectFile, ObjectFileIOError> ObjectFileReader::readFile(const std::filesystem::path &Path)
  {
    std::ifstream Stream(Path, std::ios::binary | std::ios::ate);
    if (!Stream.is_open())
    {
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::FileOpenFailed, 0, std::nullopt});
    }
    const auto End = Stream.tellg();
    if (End == std::ifstream::pos_type(-1))
    {
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::FileReadFailed, 0, std::nullopt});
    }
    const auto Length = End - std::ifstream::pos_type(0);
    if (Length < 0)
    {
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::FileReadFailed, 0, std::nullopt});
    }
    const auto Size = static_cast<std::uint64_t>(Length);
    std::vector<std::uint8_t> FileBytes;
    if (Size > FileBytes.max_size() || Size > std::numeric_limits<std::size_t>::max())
    {
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::SizeOverflow, 0, std::nullopt});
    }
    Stream.seekg(0, std::ios::beg);
    if (!Stream)
    {
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::FileReadFailed, 0, std::nullopt});
    }
    FileBytes.resize(static_cast<std::size_t>(Size));
    std::size_t Cursor = 0;
    while (Cursor < FileBytes.size())
    {
      const auto Chunk = std::min<std::uint64_t>(FileBytes.size() - Cursor, static_cast<std::uint64_t>(std::numeric_limits<std::streamsize>::max()));
      Stream.read(reinterpret_cast<char *>(FileBytes.data() + Cursor), static_cast<std::streamsize>(Chunk));
      const auto ReadCount = Stream.gcount();
      if (!Stream || ReadCount != static_cast<std::streamsize>(Chunk))
      {
        const auto Offset = static_cast<std::uint64_t>(Cursor) + (ReadCount > 0 ? static_cast<std::uint64_t>(ReadCount) : 0);
        return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::FileReadFailed, Offset, std::nullopt});
      }
      Cursor += static_cast<std::size_t>(Chunk);
    }
    return read(FileBytes);
  }
} // namespace ink::ir
