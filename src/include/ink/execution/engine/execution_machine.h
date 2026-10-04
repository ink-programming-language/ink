#ifndef INK_EXECUTION_ENGINE_EXECUTION_MACHINE_H
#define INK_EXECUTION_ENGINE_EXECUTION_MACHINE_H

#include "ink/core/core_define.h"
#include "ink/execution/bytecode/executable_function.h"
#include "ink/execution/ffi/native_call_cache.h"
#include "ink/execution/runtime/runtime_bytes.h"

#include <memory>
#include <span>
#include <unordered_map>
#include <vector>

namespace ink::execution
{
  class ExecutionEngine;
  class ExecutionLinker;

  // Semantic conversion belongs to the engine boundary. The VM consumes lowered
  // identities and values; first-use compilation belongs to ExecutionLinker.
  class ExecutionMachine final
  {
    public:
      ExecutionMachine(ExecutionEngine &Engine, ExecutionLinker &Linker);
      RuntimeValueResult execute(FunctionId Function, std::span<const RuntimeValue> Arguments);
      void clearNativeSymbols() noexcept;
      void clearCode() noexcept;
      const RuntimeTypeTable *types() const noexcept;

    private:
      using Slot = RuntimeSlot;

      struct PreparedOperation
      {
          std::size_t Cost = 0;
          const TypeDesc *Layout = nullptr;
      };

      struct PreparedFunction
      {
          const ExecutableFunction *Image = nullptr;
          std::vector<PreparedOperation> Operations;
          std::vector<PreparedFunction *> CallTargets;
          std::vector<const RuntimeFunctionDescriptor *> CallDescriptors;
          std::vector<std::size_t> SlotOffsets;
          std::vector<std::size_t> LocalOffsets;
          std::vector<const TypeDesc *> LocalLayouts;
          RuntimeBytes InitialBytes{nullptr, RuntimeBytesDeleter{}};
          std::size_t FrameSize = 0;
          std::size_t FrameAlignment = 1;
      };

      struct CallFrame
      {
          PreparedFunction *Function = nullptr;
          RuntimeBytes Bytes{nullptr, RuntimeBytesDeleter{}};
          std::vector<Slot> Slots;
          std::vector<Slot> Locals;
          std::vector<ExecutionStorageRef> Storage;
          std::size_t PC = 0;
          SlotId ReturnSlot = InvalidSlot;
          bool Nested = false;
      };

      PreparedFunction *prepare(FunctionId Function, ExecutionStatus &Status);
      ExecutionStatus initializeFrame(CallFrame &Frame, PreparedFunction &Function);
      ExecutionStatus importValue(const TypeDesc &Layout, void *Destination, const RuntimeValue &Value);
      ExecutionStatus validateValue(const Slot &Value) const noexcept;
      ExecutionStatus validateValue(const RuntimeValue &Value) const noexcept;
      ExecutionStatus validateArgument(const RuntimeValue &Value, const TypeDesc &Layout);
      ExecutionStatus beginCall(bool Nested);
      void endCall(CallFrame &Frame) noexcept;
      RuntimeValueResult callNative(const RuntimeFunctionDescriptor &Function, std::span<const RuntimeValue> Arguments, bool Nested);
      RuntimeValueResult run(PreparedFunction &Function, std::span<const RuntimeValue> Arguments);
      // Instruction handlers are defined in the .inc files included by the dispatch translation unit.
      FORCE_INLINE ExecutionStatus executeCall(const BytecodeInstruction &InstructionValue, std::vector<CallFrame> &Stack);
      FORCE_INLINE ExecutionStatus executeMemory(const BytecodeInstruction &InstructionValue, CallFrame &Frame);
      FORCE_INLINE ExecutionStatus executeClass(const BytecodeInstruction &InstructionValue, CallFrame &Frame);
      FORCE_INLINE ExecutionStatus executeArray(const BytecodeInstruction &InstructionValue, CallFrame &Frame);
      FORCE_INLINE ExecutionStatus executeWide(const BytecodeInstruction &InstructionValue, CallFrame &Frame);
      template <BytecodeOpcode Code>
      FORCE_INLINE ExecutionStatus executeScalar(const BytecodeInstruction &InstructionValue, CallFrame &Frame);
      FORCE_INLINE ExecutionStatus executeFunction(const BytecodeInstruction &InstructionValue, CallFrame &Frame);
      FORCE_INLINE ExecutionStatus executeJump(const BytecodeInstruction &InstructionValue, CallFrame &Frame);
      FORCE_INLINE ExecutionStatus executeJumpIf(const BytecodeInstruction &InstructionValue, CallFrame &Frame);
      FORCE_INLINE ExecutionStatus executeReturn(const BytecodeInstruction &InstructionValue, std::vector<CallFrame> &Stack, RuntimeValueResult &Result);
      FORCE_INLINE ExecutionStatus executeFailure(const BytecodeInstruction &InstructionValue);

      ExecutionEngine &Engine;
      ExecutionLinker &Linker;
      std::unordered_map<FunctionId, std::unique_ptr<PreparedFunction>> Functions;
      NativeCallCache NativeCalls;
      std::size_t PreparedBytes = 0;
      std::size_t ActiveFrameBytes = 0;
  };
} // namespace ink::execution

#endif
