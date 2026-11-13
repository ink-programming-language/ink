#pragma once

#include "ink/backend/vm/executable/executable_file.h"
#include "ink/backend/vm/interpreter/loader.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace ink::vm
{
  struct LibraryDesciptor
  {
      // The image's byte offset in CodeSegment, preserved when the vector reallocates.
      std::uint64_t ImageBase = 0;
      vm::ExecutableFile ExecutableFile;
  };

  class Interpreter
  {
    public:
      static constexpr std::uint32_t InvalidLibraryHandle = 0;

      Interpreter() noexcept;

      Interpreter(const Interpreter &) = delete;
      Interpreter &operator=(const Interpreter &) = delete;
      Interpreter(Interpreter &&) = delete;
      Interpreter &operator=(Interpreter &&) = delete;

      // Owns Executable and invokes the loader; handles are never reused, and exhaustion returns InvalidLibraryHandle.
      [[nodiscard]] std::uint32_t loadLibrary(ExecutableFile Executable);

      // The borrowed file remains valid until this interpreter is destroyed; an unknown handle returns nullptr.
      const ExecutableFile *findExecutable(std::uint32_t Handle) const noexcept;

      const std::vector<std::uint8_t> &getCodeSegment() const noexcept
      {
        return CodeSegment;
      }

    private:
      std::unordered_map<std::uint32_t, LibraryDesciptor> Libraries;
      std::vector<std::uint8_t> CodeSegment;
      vm::Loader Loader;
      std::uint32_t NextLibraryHandle = 1;
  };
} // namespace ink::vm
