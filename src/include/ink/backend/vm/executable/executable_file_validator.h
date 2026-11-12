#pragma once

namespace ink::core
{
  class DiagnosticEngine;
} // namespace ink::core

namespace ink::vm
{
  class ExecutableFile;

  class ExecutableFileValidator final
  {
    public:
      // Checks image bounds, dynamic import identities, image-base and import address relocations, and named export offsets without resolving addresses or decoding instructions.
      // Image-base relocation targets may identify the image's one-past end; loading checks the final address plus Addend.
      // Entry and export offsets lie in the image; validity does not establish legal function entries, call contracts, or executable bytecode.
      // Success returns normally; the first failure reports an ICE through Diagnostics and terminates in every build mode.
      static void validate(const ExecutableFile &Executable, const core::DiagnosticEngine &Diagnostics) noexcept;
  };
} // namespace ink::vm
