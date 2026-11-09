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
      // Checks image bounds, import identities and data-relocation fields without resolving host symbols or decoding instructions.
      // A valid image is nonempty and contains its entry offset; validity does not establish a legal function entry or executable bytecode.
      // Success returns normally; the first failure reports an ICE through Diagnostics and terminates in every build mode.
      static void validate(const ExecutableFile &Executable, const core::DiagnosticEngine &Diagnostics) noexcept;
  };
} // namespace ink::vm
