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
      // Checks image bounds, byte order, dynamic import identities, address relocations, and named export offsets without resolving host symbols or decoding instructions.
      // Entry and export offsets lie in the image; validity does not establish legal function entries, call contracts, or executable bytecode.
      // Success returns normally; the first failure reports an ICE through Diagnostics and terminates in every build mode.
      static void validate(const ExecutableFile &Executable, const core::DiagnosticEngine &Diagnostics) noexcept;
  };
} // namespace ink::vm
