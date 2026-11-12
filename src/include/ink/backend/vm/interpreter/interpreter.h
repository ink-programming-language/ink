#pragma once

#include "ink/backend/vm/executable/executable_file.h"
#include "ink/backend/vm/interpreter/loader.h"

#include <cstdint>
#include <unordered_map>

namespace ink::vm
{
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

    private:
      std::unordered_map<std::uint32_t, ExecutableFile> Libraries;
      vm::Loader Loader;
      std::uint32_t NextLibraryHandle = 1;
  };
} // namespace ink::vm
