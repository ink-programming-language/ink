#ifndef INK_EXECUTION_INTERPRETER_H
#define INK_EXECUTION_INTERPRETER_H

#include "ink/execution/runtime_value.h"
#include "ink/execution/runtime_world.h"
#include "ink/ir/module.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace ink::execution
{
  enum class ExecutionStatus : std::uint8_t
  {
    Returned,
    LanguageTrap,
    LimitExceeded,
    Unsupported,
    InternalInvariantFailure,
  };

  enum class ExecutionLimitKind : std::uint8_t
  {
    None,
    Fuel,
    CallDepth,
    Stack,
  };

  struct ExecutionLimits
  {
    std::uint64_t Fuel = 1'000'000;
    std::uint32_t MaxCallDepth = 1'024;
    std::uint64_t MaxStackBytes = 16 * 1'024 * 1'024;
  };

  struct ExecutionStatistics
  {
    std::uint64_t FuelConsumed = 0;
    std::uint32_t MaximumCallDepth = 0;
    std::uint64_t PeakStackBytes = 0;
  };

  struct ExecutionResult
  {
    ExecutionStatus Status = ExecutionStatus::InternalInvariantFailure;
    std::optional<RuntimeValue> Value;
    std::optional<ir::IrTrapKind> Trap;
    ExecutionLimitKind Limit = ExecutionLimitKind::None;
    ir::IrOriginId Origin;
    std::string Message;
    ExecutionStatistics Statistics;

    bool returned() const noexcept;
  };

  ExecutionResult interpret(const ir::VerifiedClosedModule &Module, ir::IrFunctionId Entry, RuntimeWorld &World, const std::vector<RuntimeValue> &Arguments, ExecutionLimits Limits = {});
} // namespace ink::execution

#endif
