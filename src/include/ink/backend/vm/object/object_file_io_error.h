#pragma once

#include "ink/backend/vm/object/object_file_validator.h"

#include <cstdint>
#include <optional>

namespace ink::vm
{
  enum class ObjectFileIOErrorKind : std::uint8_t
  {
    InvalidObject,
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
      // Byte position for encoding or I/O failures; zero for object-wide validation errors.
      std::uint64_t ByteOffset = 0;
      std::optional<ObjectValidationError> ValidationError;
  };
} // namespace ink::vm
