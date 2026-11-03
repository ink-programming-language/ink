#pragma once

#include "ink/ir/object/object_file.h"
#include "ink/ir/object/object_file_io_error.h"

#include <cstdint>
#include <expected>
#include <filesystem>
#include <span>

namespace ink::ir
{
  class ObjectFileReader final
  {
    public:
      // Reads exactly one complete object and validates it before returning an owned value.
      [[nodiscard]] static std::expected<ObjectFile, ObjectFileIOError> read(std::span<const std::uint8_t> Bytes);

      [[nodiscard]] static std::expected<ObjectFile, ObjectFileIOError> readFile(const std::filesystem::path &Path);
  };
} // namespace ink::ir
