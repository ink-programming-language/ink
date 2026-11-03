#pragma once

#include "ink/ir/object/object_file_io_error.h"

#include <cstdint>
#include <expected>
#include <filesystem>
#include <vector>

namespace ink::ir
{
  class ObjectFile;

  class ObjectFileWriter final
  {
    public:
      // Encodes validated fields explicitly; code and data payload bytes are preserved verbatim.
      [[nodiscard]] static std::expected<std::vector<std::uint8_t>, ObjectFileIOError> write(const ObjectFile &Object);

      // Validation and encoding finish before the destination file is opened for writing.
      [[nodiscard]] static std::expected<void, ObjectFileIOError> writeFile(const ObjectFile &Object, const std::filesystem::path &Path);
  };
} // namespace ink::ir
