#pragma once

#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/support/core_define.h"

#include <cstdint>
#include <limits>
#include <string_view>

namespace ink::vm::detail
{
  inline std::uint64_t addAddressOffset(std::uint64_t Address, std::int64_t Addend, const core::DiagnosticEngine &Diagnostics, std::string_view Context = "dynamic import relocation") noexcept
  {
    if (Addend >= 0)
    {
      const auto Offset = static_cast<std::uint64_t>(Addend);
      REPORT_ICE_IF(Offset > std::numeric_limits<std::uint64_t>::max() - Address, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, Context, "resolved address plus addend overflows");
      return Address + Offset;
    }
    const auto Offset = std::uint64_t{0} - static_cast<std::uint64_t>(Addend);
    REPORT_ICE_IF(Offset > Address, Diagnostics, core::DiagnosticKind::VMExecutableValidationFailed, {}, Context, "resolved address plus addend underflows");
    return Address - Offset;
  }
} // namespace ink::vm::detail
