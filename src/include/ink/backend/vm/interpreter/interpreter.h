#pragma once

#include "ink/backend/vm/executable/executable_file.h"
#include "ink/backend/vm/interpreter/loader.h"
#include "ink/core/name/name.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace ink::vm
{
  struct LibraryDescriptor
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

      // Owns Executable and invokes the loader; failed loads restore interpreter state and return InvalidLibraryHandle.
      // Successful handles are never reused; handle exhaustion also returns InvalidLibraryHandle.
      [[nodiscard]] std::uint32_t loadLibrary(ExecutableFile Executable);

      // Registers CodeSegment-relative addresses; later entries replace existing names.
      void loadExportSymbol(const LibraryDescriptor &Library);

      // The borrowed file remains valid until this interpreter is destroyed; an unknown handle returns nullptr.
      const ExecutableFile *findExecutable(std::uint32_t Handle) const noexcept;

      const std::vector<std::uint8_t> &getCodeSegment() const noexcept
      {
        return CodeSegment;
      }

      const std::unordered_map<core::Name, std::uint64_t> &getExportSymbolMap() const noexcept
      {
        return ExportSymbolMap;
      }

    private:
      class LoadTransaction;

      std::unordered_map<std::uint32_t, LibraryDescriptor> Libraries;
      std::vector<std::uint8_t> CodeSegment;
      std::unordered_map<core::Name, std::uint64_t> ExportSymbolMap;
      vm::Loader Loader;
      std::uint32_t NextLibraryHandle = 1;
  };
} // namespace ink::vm
