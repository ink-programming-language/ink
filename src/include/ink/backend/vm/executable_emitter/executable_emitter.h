#pragma once

#include "ink/backend/vm/executable/executable_file.h"
#include "ink/core/name/name.h"

#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace ink::core
{
  class DiagnosticEngine;
} // namespace ink::core

namespace ink::vm
{
  class MachineModule;

  class ExecutableEmitter
  {
    public:
      // The borrowed diagnostic engine must outlive this emitter.
      explicit ExecutableEmitter(const core::DiagnosticEngine &Diagnostics) noexcept;

      [[nodiscard]] ExecutableFile emit(const MachineModule &Module);

      // Rebuilds the symbol map from all functions and globals, using zero offsets until image layout is known.
      void scanUnResolvedSymbol(const MachineModule &Module);

      // Appends Count bytes from a readable source outside the image; zero Count permits a null source.
      void pad(const void *Src, std::size_t Count);

      // Appends Count zero bytes to the image.
      void padZero(std::size_t Count);

      // Appends aligned global storage, zero-fills padding and uninitialized tails, and resolves each global to its image offset.
      void padGlobals(const MachineModule &Module);

    private:
      void padInteger(std::uint64_t Bits, std::uint32_t Width);

      const core::DiagnosticEngine &Diagnostics;
      ExecutableFile Executable;
      std::unordered_map<core::Name, std::uint64_t> UnresolvedSymbol;
      std::unordered_map<core::Name, std::uint64_t> ResolvedSymbol;
  };
} // namespace ink::vm
