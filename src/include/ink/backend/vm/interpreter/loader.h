#pragma once

#include <cstdint>
#include <vector>

namespace ink::vm
{
  class ExecutableFile;
  class Interpreter;

  class Loader
  {
    public:
      // The owning interpreter must outlive this loader.
      explicit Loader(vm::Interpreter &Interpreter) noexcept;

      // ImageBase is a byte offset in CodeSegment; false reports a recoverable loading failure.
      // Image/export bounds are checked; relocation and symbol binding are not implemented yet.
      [[nodiscard]] bool loadLibrary(std::vector<std::uint8_t> &CodeSegment, std::uint64_t ImageBase, ExecutableFile &Executable);

    private:
      vm::Interpreter &Interpreter;
  };
} // namespace ink::vm
