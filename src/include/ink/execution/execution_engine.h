#ifndef INK_EXECUTION_EXECUTION_ENGINE_H
#define INK_EXECUTION_EXECUTION_ENGINE_H

#include "ink/core/diagnostic.h"
#include "ink/execution/context.h"
#include "ink/execution/native_symbol_registry.h"
#include "ink/ir/module.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace ink::execution
{
  class RuntimeAggregateStorage;

  class RuntimeValue
  {
  public:
    static RuntimeValue voidValue(const ir::Type &ValueType) noexcept;
    static RuntimeValue integerValue(const ir::Type &ValueType, std::uint64_t Value) noexcept;
    static RuntimeValue pointerValue(const ir::Type &ValueType, const void *Value) noexcept;
    static RuntimeValue aggregateValue(const ir::StructType &ValueType, std::vector<RuntimeValue> Fields);

    const ir::Type &type() const noexcept;
    std::optional<std::uint64_t> integer() const noexcept;
    const void *pointer() const noexcept;
    std::size_t fieldCount() const noexcept;
    const RuntimeValue *field(std::size_t FieldIndex) const noexcept;

  private:
    explicit RuntimeValue(const ir::Type &ValueType) noexcept;

    const ir::Type *ValueType;
    std::uint64_t Integer = 0;
    const void *Pointer = nullptr;
    std::shared_ptr<const RuntimeAggregateStorage> Aggregate;
  };

  class InitializationResult
  {
  public:
    bool succeeded() const noexcept;
    const std::vector<core::Diagnostic> &diagnostics() const noexcept;

  private:
    bool Succeeded = false;
    std::vector<core::Diagnostic> Diagnostics;

    friend class ExecutionEngine;
  };

  class ExecutionResult
  {
  public:
    bool succeeded() const noexcept;
    const std::optional<RuntimeValue> &returnValue() const noexcept;
    const std::vector<core::Diagnostic> &diagnostics() const noexcept;

  private:
    std::optional<RuntimeValue> ReturnValue;
    std::vector<core::Diagnostic> Diagnostics;

    friend class ExecutionEngine;
  };

  class ExecutionEngine
  {
  public:
    ExecutionEngine(ExecutionContext &Context, const ir::Module &ModuleValue);
    ~ExecutionEngine();
    ExecutionEngine(const ExecutionEngine &) = delete;
    ExecutionEngine &operator=(const ExecutionEngine &) = delete;
    ExecutionEngine(ExecutionEngine &&) = delete;
    ExecutionEngine &operator=(ExecutionEngine &&) = delete;

    NativeSymbolRegistry &nativeSymbols() noexcept;
    const NativeSymbolRegistry &nativeSymbols() const noexcept;
    InitializationResult initialize();
    ExecutionResult execute(std::string_view EntryName, const std::vector<RuntimeValue> &Arguments = {});

  private:
    class Impl;
    std::unique_ptr<Impl> Implementation;
  };
} // namespace ink::execution

#endif
