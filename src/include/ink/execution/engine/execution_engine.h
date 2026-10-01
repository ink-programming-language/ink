#ifndef INK_EXECUTION_ENGINE_EXECUTION_ENGINE_H
#define INK_EXECUTION_ENGINE_EXECUTION_ENGINE_H

#include "ink/execution/engine/execution_frame.h"
#include "ink/execution/memory/execution_heap.h"
#include "ink/execution/support/execution_result.h"
#include "ink/execution/ffi/native_symbol_cache.h"
#include "ink/core/config_manager.h"

#include <cstddef>
#include <functional>
#include <memory>
#include <span>
#include <vector>

namespace ink::ir
{
  class IRContext;
  class Function;
  class Type;
  class Value;
} // namespace ink::ir

namespace ink::tokenizer
{
  enum class TokenKind;
} // namespace ink::tokenizer

namespace ink::execution
{
  struct ExecutionLimits
  {
      std::size_t MaxSteps = core::ConfigManager::getSize<core::ConfigKind::ExecutionMaxSteps>();
      // Counts all storage allocations, including releases; reused slots get new generations.
      std::size_t MaxObjects = core::ConfigManager::getSize<core::ConfigKind::ExecutionMaxObjects>();
      std::size_t MaxCallDepth = core::ConfigManager::getSize<core::ConfigKind::ExecutionMaxCallDepth>();
      // Bounds combined AST expression, statement and call traversal on the host stack.
      std::size_t MaxEvaluationDepth = core::ConfigManager::getSize<core::ConfigKind::ExecutionMaxEvaluationDepth>();
  };

  // Owned execution storage shared by sequential semantic evaluation requests and IR calls.
  // The IR context must outlive the engine. Binding and event identities must remain
  // stable until their frame ends. The semantic driver determines execution order
  // and active paths; this class is single-threaded. Resolved native modules must
  // remain loaded until the symbol cache is cleared or the engine is destroyed.
  class ExecutionEngine final
  {
    public:
      explicit ExecutionEngine(ir::IRContext &Context, ExecutionLimits Limits = {});
      ~ExecutionEngine();
      ExecutionEngine(const ExecutionEngine &) = delete;
      ExecutionEngine &operator=(const ExecutionEngine &) = delete;
      ExecutionEngine(ExecutionEngine &&) = delete;
      ExecutionEngine &operator=(ExecutionEngine &&) = delete;

      ir::IRContext &context() noexcept;
      const ir::IRContext &context() const noexcept;
      ExecutionHeap &heap() noexcept;
      const ExecutionHeap &heap() const noexcept;
      ExecutionStatus lastStatus() const noexcept;

      // Clear between invocations before unloading modules or rebinding symbols.
      // Outstanding native addresses and calls must finish using modules first.
      void clearNativeSymbolCache() noexcept;

      // Null indicates an invalid parent or an exhausted limit; see lastStatus().
      ExecutionFrame *createFrame(ExecutionFrameKind Kind, ExecutionFrame *Parent = nullptr);
      // Module frames persist. Ending another frame invalidates its objects and descendants.
      ExecutionStatus endFrame(ExecutionFrame &Frame);

      ExecutionPlaceResult allocate(ExecutionFrame &Frame, const void *Binding, const ir::Type &Type, bool Writable = true, const ir::Constant *Initial = nullptr);
      ExecutionPlaceResult bindRuntime(ExecutionFrame &Frame, const void *Binding, const ir::Type &Type, bool Writable = true);
      ExecutionPlaceResult lookup(const ExecutionFrame &Frame, const void *Binding);
      ExecutionResult load(ExecutionPlace Place);
      ExecutionStatus store(ExecutionPlace Place, const ir::Constant &Value);

      // Constants are frozen only at the semantic boundary. Runtime storage and
      // instruction results own their payload and do not enter the constant pool.
      ExecutionPlaceResult allocateValue(ExecutionFrame &Frame, const void *Binding, const ir::Type &Type, bool Writable = true, const ExecutionValueRef *Initial = nullptr);
      ExecutionValueResult loadValue(ExecutionPlace Place);
      ExecutionStatus storeValue(ExecutionPlace Place, const ExecutionValueRef &Value);
      ExecutionValueResult evaluate(const ir::Value &Value, ExecutionFrame &Frame);
      ExecutionValueResult execute(const ir::Function &Function, std::span<const ExecutionValueRef> Arguments = {});

      // Only semantic events use this cache. Ordinary evaluation, loop iterations and
      // actual calls execute afresh; use a distinct frame or key for each such event.
      // Cancellation and budget exhaustion are never recorded as semantic failures.
      // Either stops this engine permanently; cleanup remains available on stopped engines.
      // ReusedResult distinguishes completed cached results from a callback or a rejected request.
      ExecutionResult executeOnce(ExecutionFrame &Frame, const void *EventKey, const std::function<ExecutionResult()> &Callback, bool *ReusedResult = nullptr);

      // Every invocation owns a fresh call frame; Ink callbacks execute the AST body.
      // Ink parameters are writable objects bound by FunctionParameter identity;
      // C linkage always uses a checked host adapter and ignores the body callback.
      ExecutionResult call(const ir::Function &Function, ExecutionFrame &DefinitionFrame, std::span<const ir::Value *const> Arguments, const std::function<ExecutionResult(ExecutionFrame &)> &Body = {});

      // The semantic driver charges AST operations and loop back edges here as well.
      ExecutionStatus consumeStep();
      // Every successful entry must be paired with a leave, including during failure cleanup.
      ExecutionStatus enterEvaluation();
      void leaveEvaluation() noexcept;
      void cancel() noexcept;

      ExecutionResult evaluateUnary(tokenizer::TokenKind Operator, const ir::Constant &Operand);
      ExecutionResult evaluateBinary(tokenizer::TokenKind Operator, const ir::Constant &Left, const ir::Constant &Right);

    private:
      ExecutionPlaceResult allocateObject(ExecutionFrame &Frame, const void *Binding, const ir::Type &Type, bool Writable, const ExecutionValueRef *Initial, bool Runtime);
      ExecutionStatus validatePlace(ExecutionPlace Place) const noexcept;
      ExecutionStatus validateValue(const ExecutionValueRef &Value) const noexcept;
      ExecutionValueResult executeInvocation(const ir::Function &Function, std::span<const ExecutionValueRef> Arguments);
      ExecutionValueResult executeBody(const ir::Function &Function, ExecutionFrame &Frame);
      ExecutionValueResult executeInstruction(const ir::Value &Instruction, ExecutionFrame &Frame);
      ExecutionValueResult executeCall(const ir::Value &Instruction, ExecutionFrame &Frame);
      ExecutionValueResult loadPointer(const ExecutionValueRef &Address);
      ExecutionStatus storePointer(const ExecutionValueRef &Address, const ExecutionValueRef &Value);
      ExecutionResult freeze(const ExecutionValueResult &Result);

      ir::IRContext &Context;
      ExecutionLimits Limits;
      ExecutionHeap Heap;
      NativeSymbolCache NativeSymbols;
      ExecutionStatus LastStatus = ExecutionStatus::Success;
      ExecutionStatus StopStatus = ExecutionStatus::Success;
      std::size_t Steps = 0;
      std::size_t ActiveCalls = 0;
      std::size_t EvaluationDepth = 0;
      std::vector<std::unique_ptr<ExecutionFrame>> Frames;
  };
} // namespace ink::execution

#endif
