#pragma once

#include <cstdint>

namespace ink::vm
{
  enum class ObjectFileIOErrorKind : std::uint8_t
  {
    UnexpectedEnd,
    InvalidEncoding,
    SizeOverflow,
    TrailingData,
    FileOpenFailed,
    FileReadFailed,
    FileWriteFailed,
  };

  struct ObjectFileIOError
  {
      ObjectFileIOErrorKind Kind;
      // Byte position for decoding, encoding, or file I/O failures.
      std::uint64_t ByteOffset = 0;
  };
} // namespace ink::vm
