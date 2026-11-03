#pragma once

#include <cstdint>
#include <optional>

namespace ink::ir
{
  class ObjectFile;

  enum class ObjectValidationErrorKind : std::uint8_t
  {
    InvalidHeader,
    TooManyRecords,
    InvalidConstantSection,
    InvalidGlobalStorage,
    InvalidSymbol,
    InvalidBaseReference,
    InvalidDebugInfo,
  };

  struct ObjectValidationError
  {
      ObjectValidationErrorKind Kind;
      // Index in the collection selected by Kind; zero for singleton records or a collection-size error.
      std::uint64_t RecordIndex = 0;
  };

  class ObjectFileValidator final
  {
    public:
      // Checks record structure and references, not bytecode executability.
      // Returns the first error; unresolved symbol imports are valid object contents.
      static std::optional<ObjectValidationError> validate(const ObjectFile &Object) noexcept;
  };
} // namespace ink::ir
