#ifndef INK_EXECUTION_BYTECODE_EXECUTABLE_FUNCTION_H
#define INK_EXECUTION_BYTECODE_EXECUTABLE_FUNCTION_H

#include "ink/execution/bytecode/instruction.h"
#include "ink/execution/runtime/runtime_value.h"

#include <memory>
#include <vector>

namespace ink::execution
{
  struct ExecutionCallSite
  {
      FunctionId Target = InvalidFunction;
      SlotId CalleeSlot = InvalidSlot;
      std::vector<SlotId> Arguments;
      RuntimeTypeId Signature = InvalidRuntimeType;
  };

  // The executable owns its constants and shares only lowered runtime layouts.
  // Parameter slots come first; no part of this image borrows an IR object.
  struct ExecutableFunction
  {
      FunctionId Id = InvalidFunction;
      RuntimeTypeId Signature = InvalidRuntimeType;
      std::shared_ptr<const RuntimeTypeTable> Layouts;
      std::vector<BytecodeInstruction> Code;
      std::vector<RuntimeTypeId> SlotTypes;
      std::vector<RuntimeValue> InitialSlots;
      std::vector<ExecutionCallSite> Calls;
      std::vector<char> ConstantData;
      std::uint32_t LocalStorageCount = 0;
  };
} // namespace ink::execution

#endif
