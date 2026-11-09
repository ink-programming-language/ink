#pragma once

#include "ink/backend/vm/object/object_file.h"
#include "ink/backend/vm/object/object_file_io_error.h"

#include <cstdint>
#include <expected>
#include <filesystem>
#include <span>

namespace ink::core
{
  class DiagnosticEngine;
} // namespace ink::core

namespace ink::vm
{
  class ObjectFileReader final
  {
    public:
      // Reads exactly one complete object; structural validation failures report an ICE before returning an owned value.
      [[nodiscard]] static std::expected<ObjectFile, ObjectFileIOError> read(std::span<const std::uint8_t> Bytes, const core::DiagnosticEngine &Diagnostics);

      [[nodiscard]] static std::expected<ObjectFile, ObjectFileIOError> readFile(const std::filesystem::path &Path, const core::DiagnosticEngine &Diagnostics);
  };
} // namespace ink::vm
