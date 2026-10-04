#ifndef INK_EXECUTION_VALUE_EXECUTION_VALUE_H
#define INK_EXECUTION_VALUE_EXECUTION_VALUE_H

#include "ink/execution/support/runtime_kind.h"

#include "ink/execution/memory/execution_pointer.h"
#include "ink/execution/support/execution_object.h"
#include "ink/execution/support/execution_result.h"
#include "ink/execution/value/execution_integer.h"
#include "ink/ir/constant/float_constant.h"

#include <memory>
#include <span>
#include <string_view>

namespace ink::ir
{
  class Function;
  class IRContext;
} // namespace ink::ir

namespace ink::execution
{
  class ExecutionHeap;

  // Immutable execution payloads are shared through ExecutionValueRef. Their
  // types and any referenced IR functions must outlive the values themselves.
  class ExecutionValue : public ExecutionObject
  {
    public:
      ~ExecutionValue() override = 0;

      RuntimeKind kind() const noexcept;

      const ir::Type *type() const noexcept
      {
        return &ValueType;
      }

      bool valid() const noexcept;
      // Local scalar constants and aggregates of those constants can be frozen into IR.
      const ir::Constant *toConstant(ir::IRContext &Context) const;

    protected:
      ExecutionValue(ExecutionObjectKind Kind, const ir::Type &Type) noexcept;

    private:
      const ir::Type &ValueType;
  };

  // Copying a reference shares an immutable payload. Pointer payloads borrow
  // their storage; keeping a value alive never extends that storage's lifetime.
  class ExecutionValueRef final
  {
    public:
      ExecutionValueRef() = default;

      const ExecutionValue *get() const noexcept
      {
        return Value.get();
      }

      const ExecutionValue *operator->() const noexcept;

      explicit operator bool() const noexcept
      {
        return static_cast<bool>(Value);
      }

      RuntimeKind kind() const noexcept
      {
        return Value ? Value->kind() : RuntimeKind::Invalid;
      }

      const ir::Type *type() const noexcept
      {
        return Value ? Value->type() : nullptr;
      }

      bool boolean() const noexcept;
      const ExecutionInteger &integer() const noexcept;
      ir::FloatBits floating() const noexcept;
      std::string_view string() const noexcept;
      std::span<const ExecutionValueRef> array() const noexcept;
      std::span<const ExecutionValueRef> fields() const noexcept;
      const ExecutionPointer &pointer() const noexcept;
      const ir::Function *function() const noexcept;

      bool valid() const noexcept
      {
        return Value && Value->valid();
      }

      const ir::Constant *toConstant(ir::IRContext &Context) const
      {
        return Value ? Value->toConstant(Context) : nullptr;
      }

    private:
      explicit ExecutionValueRef(std::shared_ptr<const ExecutionValue> Value) noexcept;

      std::shared_ptr<const ExecutionValue> Value;

      friend class ExecutionHeap;
  };

  struct ExecutionValueResult
  {
      ExecutionValueResult() = default;
      ExecutionValueResult(ExecutionStatus Status) noexcept;
      ExecutionValueResult(ExecutionStatus Status, ExecutionValueRef Value) noexcept;

      ExecutionStatus Status = ExecutionStatus::InvalidArguments;
      ExecutionValueRef Value;

      bool succeeded() const noexcept
      {
        return Status == ExecutionStatus::Success && static_cast<bool>(Value);
      }

      explicit operator bool() const noexcept
      {
        return succeeded();
      }
  };
} // namespace ink::execution

#endif
