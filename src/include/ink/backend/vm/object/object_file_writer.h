#pragma once

#include "ink/backend/vm/object/object_file_io_error.h"

#include <cstdint>
#include <expected>
#include <filesystem>
#include <vector>

namespace ink::core
{
  class DiagnosticEngine;
} // namespace ink::core

namespace ink::vm
{
  class ObjectFile;

  class ObjectFileWriter final
  {
    public:
      // Structural validation failures report an ICE; valid code and data payload bytes are encoded verbatim.
      [[nodiscard]] static std::expected<std::vector<std::uint8_t>, ObjectFileIOError> write(const ObjectFile &Object, const core::DiagnosticEngine &Diagnostics);

      // Validation and encoding finish before the destination file is opened for writing.
      [[nodiscard]] static std::expected<void, ObjectFileIOError> writeFile(const ObjectFile &Object, const std::filesystem::path &Path, const core::DiagnosticEngine &Diagnostics);
  };
} // namespace ink::vm
