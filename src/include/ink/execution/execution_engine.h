#ifndef INK_EXECUTION_EXECUTION_ENGINE_H
#define INK_EXECUTION_EXECUTION_ENGINE_H

#include "ink/core/diagnostic.h"
#include "ink/execution/context.h"
#include "ink/execution/native_symbol_registry.h"
#include "ink/ir/ir.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

namespace ink::execution
{
  class RuntimeValue
  {
  public:
    static RuntimeValue voidValue() noexcept;
    static RuntimeValue integerValue(ir::TypeKind Type, std::uint64_t Value) noexcept;
    static RuntimeValue pointerValue(ir::TypeKind Type, const void *Value) noexcept;

    ir::TypeKind type() const noexcept;
    std::optional<std::uint64_t> integer() const noexcept;
    const void *pointer() const noexcept;

  private:
    ir::TypeKind Type = ir::TypeKind::Void;
    std::uint64_t Integer = 0;
    const void *Pointer = nullptr;
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
