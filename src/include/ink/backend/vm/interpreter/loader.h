#pragma once

#include <cstdint>
#include <span>

namespace ink::vm
{
  class ExecutableFile;
  class Interpreter;

  class Loader
  {
    public:
      // The owning interpreter must outlive this loader.
      explicit Loader(vm::Interpreter &Interpreter) noexcept;

      /**
       * Bind Executable's copied image at byte offset ImageBase within the writable CodeSegment span.
       * Register exports, resolve every import once, then patch aligned eight-byte address fields.
       * Executable remains unbound. Missing imports return false; the caller owns rollback.
       * Invalid internal metadata reports an ICE. See the implementation for layout and patch details.
       */
      [[nodiscard]] bool loadLibrary(std::span<std::uint8_t> CodeSegment, std::uint64_t ImageBase, ExecutableFile &Executable);

    private:
      vm::Interpreter &Interpreter;
  };
} // namespace ink::vm
