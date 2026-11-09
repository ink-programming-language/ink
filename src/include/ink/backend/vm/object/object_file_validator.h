#pragma once

namespace ink::core
{
  class DiagnosticEngine;
} // namespace ink::core

namespace ink::vm
{
  class ObjectFile;

  class ObjectFileValidator final
  {
    public:
      // Checks record structure and references, not bytecode executability.
      // Unresolved imports are valid. Success returns normally; the first failure reports an ICE and terminates in every build mode.
      static void validate(const ObjectFile &Object, const core::DiagnosticEngine &Diagnostics) noexcept;
  };
} // namespace ink::vm
