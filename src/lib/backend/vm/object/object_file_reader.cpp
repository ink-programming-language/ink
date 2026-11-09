#include "ink/backend/vm/object/object_file_reader.h"

#include "ink/core/support/version.h"
#include "ink/core/name/name_pool.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/backend/vm/object/object_file.h"
#include "ink/backend/vm/object/object_file_io_error.h"
#include "ink/backend/vm/object/object_file_validator.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <fstream>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace ink::vm
{
  namespace
  {
    class ObjectInput final
    {
      public:
        explicit ObjectInput(std::span<const std::uint8_t> Bytes, const core::DiagnosticEngine &Diagnostics) noexcept
            : Bytes(Bytes),
              Diagnostics(Diagnostics)
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
        bool readCount(std::uint64_t &Count, std::size_t MinimumRecordSize, const Container &Values, bool Indexed, std::string_view Context) noexcept
        {
          const auto Start = Cursor;
          if (!readInteger(Count))
          {
            return false;
          }
          if (Indexed && Count > static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max()) + 1)
          {
            Diagnostics.reportICE<core::DiagnosticKind::VMObjectValidationFailed>({}, Context, "record count exceeds the u32 index range");
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
          Failure = ObjectFileIOError{Kind, static_cast<std::uint64_t>(Offset)};
          return false;
        }

        std::span<const std::uint8_t> Bytes;
        const core::DiagnosticEngine &Diagnostics;
        std::size_t Cursor = 0;
        ObjectByteOrder ByteOrder = ObjectByteOrder::Little;
        std::optional<ObjectFileIOError> Failure;
    };
  } // namespace

  std::expected<ObjectFile, ObjectFileIOError> ObjectFileReader::read(std::span<const std::uint8_t> Bytes, const core::DiagnosticEngine &Diagnostics)
  {
    ObjectInput Input(Bytes, Diagnostics);
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
      ObjectFileValidator::validate(Object, Diagnostics);
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
    ObjectFileValidator::validate(Object, Diagnostics);

    if (!Input.readBytes(Object.getCode().getBytes()) || !Input.readInteger(Object.getConstants().getAlignment()) || !Input.readBytes(Object.getConstants().getBytes()) || !Input.readInteger(Object.getGlobalStorage().getAlignment()) || !Input.readInteger(Object.getGlobalStorage().getByteSize()) || !Input.readBytes(Object.getGlobalStorage().getInitialBytes()))
    {
      return std::unexpected(Input.error());
    }

    auto &Symbols = Object.getSymbols().getEntries();
    std::uint64_t SymbolCount = 0;
    if (!Input.readCount(SymbolCount, 10, Symbols, true, "symbol table"))
    {
      return std::unexpected(Input.error());
    }
    Symbols.resize(static_cast<std::size_t>(SymbolCount));
    for (auto &Symbol : Symbols)
    {
      std::string SymbolName;
      bool HasDefinition = false;
      if (!Input.readString(SymbolName) || !Input.readEnum(Symbol.Linkage, 2) || !Input.readMarker(HasDefinition))
      {
        return std::unexpected(Input.error());
      }
      Symbol.Name = core::NamePool::instance().intern(SymbolName);
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
    if (!Input.readCount(BaseReferenceCount, 4, BaseReferences, true, "base reference table"))
    {
      return std::unexpected(Input.error());
    }
    BaseReferences.resize(static_cast<std::size_t>(BaseReferenceCount));
    for (auto &Reference : BaseReferences)
    {
      if (!Input.readInteger(Reference.SymbolIndex))
      {
        return std::unexpected(Input.error());
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
      if (!Input.readCount(SourceFileCount, 8, SourceFiles, true, "debug source file table"))
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
      if (!Input.readCount(LineCount, 20, Lines, false, "debug lines"))
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
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::TrailingData, static_cast<std::uint64_t>(Input.offset())});
    }
    ObjectFileValidator::validate(Object, Diagnostics);
    return std::move(Object);
  }

  std::expected<ObjectFile, ObjectFileIOError> ObjectFileReader::readFile(const std::filesystem::path &Path, const core::DiagnosticEngine &Diagnostics)
  {
    std::ifstream Stream(Path, std::ios::binary | std::ios::ate);
    if (!Stream.is_open())
    {
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::FileOpenFailed, 0});
    }
    const auto End = Stream.tellg();
    if (End == std::ifstream::pos_type(-1))
    {
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::FileReadFailed, 0});
    }
    const auto Length = End - std::ifstream::pos_type(0);
    if (Length < 0)
    {
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::FileReadFailed, 0});
    }
    const auto Size = static_cast<std::uint64_t>(Length);
    std::vector<std::uint8_t> FileBytes;
    if (Size > FileBytes.max_size() || Size > std::numeric_limits<std::size_t>::max())
    {
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::SizeOverflow, 0});
    }
    Stream.seekg(0, std::ios::beg);
    if (!Stream)
    {
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::FileReadFailed, 0});
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
        return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::FileReadFailed, Offset});
      }
      Cursor += static_cast<std::size_t>(Chunk);
    }
    return read(FileBytes, Diagnostics);
  }
} // namespace ink::vm
