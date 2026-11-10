#pragma once

#include "ink/backend/vm/executable/executable_file.h"
#include "ink/backend/vm/module/machine_module.h"
#include "ink/core/diagnostic/diagnostic_engine.h"

#include <optional>

namespace ink::vm
{
  // Emits one validated, fully linked logical module without borrowing its nodes in the result.
  class ExecutableEmitter final
  {
    public:
      // The caller guarantees module, operand, symbol, and entry validity; emission does not validate them.
      // VM addresses are image-relative byte offsets. Layout/encoding failures report VMExecutableEmissionFailed
      // and return nullopt without changing Module. Output validation is a separate caller-controlled step.
      [[nodiscard]] static std::optional<ExecutableFile> emit(const MachineModule &Module, const MachineFunction &Entry, const core::DiagnosticEngine &Diagnostics, ByteOrder Order = ByteOrder::Little);
  };
} // namespace ink::vm
