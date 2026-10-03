#ifndef INK_EXECUTION_SUPPORT_EXECUTION_RESULT_H
#define INK_EXECUTION_SUPPORT_EXECUTION_RESULT_H

#include "ink/execution/memory/execution_storage_ref.h"

#include <cstddef>
#include <utility>

namespace ink::ir
{
  class Constant;
} // namespace ink::ir

namespace ink::execution
{
  class ExecutionEngine;

  enum class ExecutionStatus
  {
    Success,
    InvalidFrame,
    InvalidBinding,
    DuplicateBinding,
    UnknownBinding,
    InvalidPlace,
    ExpiredPlace,
    Uninitialized,
    RuntimeValue,
    ReadOnly,
    TypeMismatch,
    ForeignContext,
    InvalidArguments,
    MissingBody,
    SymbolNotFound,
    UnsupportedExternalSignature,
    HostAbiMismatch,
    UnsupportedOperation,
    DivisionByZero,
    InvalidShift,
    Overflow,
    BudgetExceeded,
    Cancelled,
    IndexOutOfBounds,
  };

  // Frozen result at the compile-time semantic boundary; IR execution returns
  // ExecutionValueResult and never interns intermediate runtime payloads.
  struct ExecutionResult
  {
      ExecutionStatus Status = ExecutionStatus::Success;
      const ir::Constant *Value = nullptr;

      bool succeeded() const noexcept
      {
        return Status == ExecutionStatus::Success;
      }

      explicit operator bool() const noexcept
      {
        return succeeded();
      }
  };

  // A place borrows a managed cell identity without extending its lifetime.
  class ExecutionPlace final
  {
    public:
      ExecutionPlace() = default;

      explicit ExecutionPlace(ExecutionStorageRef Storage) noexcept
          : Storage(std::move(Storage))
      {
      }

      bool valid() const noexcept
      {
        return Storage.cell() != nullptr;
      }

      const ExecutionStorageRef &storage() const noexcept
      {
        return Storage;
      }

      ExecutionStatus status() const noexcept
      {
        const ExecutionStatus Status = Storage.status();
        return Status == ExecutionStatus::Success && !Storage.cell() ? ExecutionStatus::InvalidPlace : Status;
      }

      bool operator==(const ExecutionPlace &) const noexcept = default;

    private:
      ExecutionStorageRef Storage;
  };

  struct ExecutionPlaceResult
  {
      ExecutionStatus Status = ExecutionStatus::Success;
      ExecutionPlace Place = {};

      bool succeeded() const noexcept
      {
        return Status == ExecutionStatus::Success;
      }

      explicit operator bool() const noexcept
      {
        return succeeded();
      }
  };
} // namespace ink::execution

#endif
