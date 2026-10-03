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

  // The verifier checks the byte span before any caller decodes its slot IDs.
  inline SlotId arraySourceSlot(const ExecutableFunction &Function, std::size_t Offset) noexcept
  {
    SlotId Result = 0;
    for (unsigned Byte = 0; Byte < 4; ++Byte)
    {
      Result |= static_cast<SlotId>(static_cast<unsigned char>(Function.ConstantData[Offset + Byte])) << (Byte * 8);
    }
    return Result;
  }
} // namespace ink::execution

#endif
