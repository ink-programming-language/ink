#include "ink/vm/object/object_file_writer.h"

#include "ink/vm/object/object_file.h"
#include "ink/vm/object/object_file_validator.h"

#include <cstddef>
#include <cstring>
#include <fstream>
#include <limits>
#include <string_view>
#include <utility>

namespace ink::vm
{
  namespace
  {
    class ObjectWriteBuffer final
    {
      public:
        explicit ObjectWriteBuffer(ObjectByteOrder ByteOrder) noexcept
            : ByteOrder(ByteOrder)
        {
        }

        bool appendBytes(const void *Source, std::size_t Count)
        {
          if (Count == 0)
          {
            return true;
          }
          const auto Start = Bytes.size();
          if (!grow(Count))
          {
            return false;
          }
          std::memcpy(Bytes.data() + Start, Source, Count);
          return true;
        }

        bool appendUnsigned(std::uint64_t Value, std::uint32_t ByteCount)
        {
          const auto Start = Bytes.size();
          if (!grow(ByteCount))
          {
            return false;
          }
          for (std::uint32_t Index = 0; Index < ByteCount; ++Index)
          {
            const auto Shift = (ByteOrder == ObjectByteOrder::Little ? Index : ByteCount - Index - 1) * 8;
            Bytes[Start + Index] = static_cast<std::uint8_t>(Value >> Shift);
          }
          return true;
        }

        bool appendSize(std::size_t Size)
        {
          if (std::cmp_greater(Size, std::numeric_limits<std::uint64_t>::max()))
          {
            return false;
          }
          return appendUnsigned(static_cast<std::uint64_t>(Size), 8);
        }

        bool appendString(std::string_view Text)
        {
          return appendSize(Text.size()) && appendBytes(Text.data(), Text.size());
        }

        std::unexpected<ObjectFileIOError> sizeOverflow() const noexcept
        {
          return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::SizeOverflow, static_cast<std::uint64_t>(Bytes.size()), std::nullopt});
        }

        std::vector<std::uint8_t> takeBytes() && noexcept
        {
          return std::move(Bytes);
        }

      private:
        bool grow(std::size_t Count)
        {
          if (Count > Bytes.max_size() - Bytes.size())
          {
            return false;
          }
          Bytes.resize(Bytes.size() + Count);
          return true;
        }

        ObjectByteOrder ByteOrder;
        std::vector<std::uint8_t> Bytes;
    };
  } // namespace

  std::expected<std::vector<std::uint8_t>, ObjectFileIOError> ObjectFileWriter::write(const ObjectFile &Object)
  {
    if (const auto Error = ObjectFileValidator::validate(Object))
    {
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::InvalidObject, 0, Error});
    }

    const auto &Header = Object.getHeader();
    ObjectWriteBuffer Buffer(Header.getByteOrder());
    const auto &Magic = Header.getMagic();
    const auto &FormatVersion = Header.getFormatVersion();
    const auto &InstructionSetVersion = Header.getInstructionSetVersion();
    if (!Buffer.appendBytes(Magic.data(), Magic.size()) || !Buffer.appendUnsigned(static_cast<std::uint8_t>(Header.getByteOrder()), 1) || !Buffer.appendUnsigned(Header.getAddressBitWidth(), 2) || !Buffer.appendUnsigned(FormatVersion.getMajor(), 4) || !Buffer.appendUnsigned(FormatVersion.getMinor(), 4) || !Buffer.appendUnsigned(FormatVersion.getPatch(), 4) || !Buffer.appendUnsigned(InstructionSetVersion.getMajor(), 4) || !Buffer.appendUnsigned(InstructionSetVersion.getMinor(), 4) || !Buffer.appendUnsigned(InstructionSetVersion.getPatch(), 4))
    {
      return Buffer.sizeOverflow();
    }

    const auto &Code = Object.getCode().getBytes();
    if (!Buffer.appendSize(Code.size()) || !Buffer.appendBytes(Code.data(), Code.size()))
    {
      return Buffer.sizeOverflow();
    }

    const auto &Constants = Object.getConstants();
    const auto &ConstantBytes = Constants.getBytes();
    if (!Buffer.appendUnsigned(Constants.getAlignment(), 8) || !Buffer.appendSize(ConstantBytes.size()) || !Buffer.appendBytes(ConstantBytes.data(), ConstantBytes.size()))
    {
      return Buffer.sizeOverflow();
    }

    const auto &GlobalStorage = Object.getGlobalStorage();
    const auto &InitialBytes = GlobalStorage.getInitialBytes();
    if (!Buffer.appendUnsigned(GlobalStorage.getAlignment(), 8) || !Buffer.appendUnsigned(GlobalStorage.getByteSize(), 8) || !Buffer.appendSize(InitialBytes.size()) || !Buffer.appendBytes(InitialBytes.data(), InitialBytes.size()))
    {
      return Buffer.sizeOverflow();
    }

    const auto &Symbols = Object.getSymbols().getEntries();
    if (!Buffer.appendSize(Symbols.size()))
    {
      return Buffer.sizeOverflow();
    }
    for (const auto &Symbol : Symbols)
    {
      if (!Buffer.appendString(Symbol.Name) || !Buffer.appendUnsigned(static_cast<std::uint8_t>(Symbol.Linkage), 1) || !Buffer.appendUnsigned(Symbol.Definition.has_value() ? 1 : 0, 1))
      {
        return Buffer.sizeOverflow();
      }
      if (Symbol.Definition)
      {
        const auto &Definition = *Symbol.Definition;
        if (!Buffer.appendUnsigned(static_cast<std::uint8_t>(Definition.Section), 1) || !Buffer.appendUnsigned(Definition.ByteOffset, 8) || !Buffer.appendUnsigned(Definition.ByteSize, 8))
        {
          return Buffer.sizeOverflow();
        }
      }
    }

    const auto &BaseReferences = Object.getBaseReferences().getEntries();
    if (!Buffer.appendSize(BaseReferences.size()))
    {
      return Buffer.sizeOverflow();
    }
    for (const auto &Reference : BaseReferences)
    {
      if (!Buffer.appendUnsigned(Reference.SymbolIndex, 4))
      {
        return Buffer.sizeOverflow();
      }
    }

    const auto &DebugInfo = Object.getDebugInfo();
    if (!Buffer.appendUnsigned(DebugInfo.has_value() ? 1 : 0, 1))
    {
      return Buffer.sizeOverflow();
    }
    if (DebugInfo)
    {
      const auto &SourceFiles = DebugInfo->getSourceFiles();
      if (!Buffer.appendSize(SourceFiles.size()))
      {
        return Buffer.sizeOverflow();
      }
      for (const auto &SourceFile : SourceFiles)
      {
        if (!Buffer.appendString(SourceFile))
        {
          return Buffer.sizeOverflow();
        }
      }
      const auto &Lines = DebugInfo->getLines();
      if (!Buffer.appendSize(Lines.size()))
      {
        return Buffer.sizeOverflow();
      }
      for (const auto &Line : Lines)
      {
        if (!Buffer.appendUnsigned(Line.CodeOffset, 8) || !Buffer.appendUnsigned(Line.FileIndex, 4) || !Buffer.appendUnsigned(Line.Line, 4) || !Buffer.appendUnsigned(Line.Column, 4))
        {
          return Buffer.sizeOverflow();
        }
      }
    }
    return std::move(Buffer).takeBytes();
  }

  std::expected<void, ObjectFileIOError> ObjectFileWriter::writeFile(const ObjectFile &Object, const std::filesystem::path &Path)
  {
    auto Encoded = write(Object);
    if (!Encoded)
    {
      return std::unexpected(Encoded.error());
    }

    std::ofstream Stream(Path, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!Stream.is_open() || !Stream)
    {
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::FileOpenFailed, 0, std::nullopt});
    }
    const auto &Bytes = *Encoded;
    std::size_t Offset = 0;
    while (Offset < Bytes.size())
    {
      const auto Remaining = Bytes.size() - Offset;
      const auto Chunk = std::cmp_greater(Remaining, std::numeric_limits<std::streamsize>::max()) ? static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max()) : Remaining;
      Stream.write(reinterpret_cast<const char *>(Bytes.data() + Offset), static_cast<std::streamsize>(Chunk));
      if (!Stream)
      {
        Stream.close();
        return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::FileWriteFailed, static_cast<std::uint64_t>(Offset), std::nullopt});
      }
      Offset += Chunk;
    }
    Stream.close();
    if (!Stream)
    {
      return std::unexpected(ObjectFileIOError{ObjectFileIOErrorKind::FileWriteFailed, static_cast<std::uint64_t>(Offset), std::nullopt});
    }
    return {};
  }
} // namespace ink::vm
