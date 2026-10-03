#include "ink/execution/engine/execution_machine.h"

#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/engine/execution_linker.h"

#include <string_view>
#include <utility>

// Keep every instruction handler visible to the compiler at its dispatch call site.
#include "dispatch/execution_machine_scalar.inc"
#include "dispatch/execution_machine_wide.inc"
#include "dispatch/execution_machine_memory.inc"
#include "dispatch/execution_machine_call.inc"
#include "dispatch/execution_machine_control.inc"

namespace ink::execution
{
  RuntimeValueResult ExecutionMachine::run(PreparedFunction &Function, std::span<const RuntimeValue> Arguments)
  {
    const StorageLayout &Signature = *Function.Image->Layouts->get(Function.Image->Signature);
    if (Arguments.size() != Signature.Parameters.size())
    {
      return {ExecutionStatus::InvalidArguments};
    }
    for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
    {
      if (Arguments[Index].Type != Signature.Parameters[Index])
      {
        return {ExecutionStatus::TypeMismatch};
      }
      const ExecutionStatus Status = validateArgument(Arguments[Index], *Function.Image->Layouts->get(Signature.Parameters[Index]));
      if (Status != ExecutionStatus::Success)
      {
        return {Status};
      }
    }
    ExecutionStatus Status = beginCall(false);
    if (Status != ExecutionStatus::Success)
    {
      return {Status};
    }
    std::vector<CallFrame> Stack;
    CallFrame Entry;
    Entry.Function = &Function;
    Entry.Slots = Function.Image->InitialSlots;
    Entry.Locals.resize(Function.Image->LocalStorageCount);
    for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
    {
      Entry.Slots[Index] = Arguments[Index];
    }
    Stack.push_back(std::move(Entry));
    RuntimeValueResult Result{ExecutionStatus::MissingBody};
    while (!Stack.empty() && Status == ExecutionStatus::Success)
    {
      CallFrame &Frame = Stack.back();
      const ExecutableFunction &Image = *Frame.Function->Image;
      if (Frame.PC >= Image.Code.size())
      {
        Status = ExecutionStatus::MissingBody;
        break;
      }
      Status = Engine.consumeSteps(Frame.Function->Operations[Frame.PC].Cost);
      if (Status != ExecutionStatus::Success)
      {
        break;
      }
      const BytecodeInstruction Operation = Image.Code[Frame.PC++];
      switch (Operation.Code)
      {
      case BytecodeOpcode::Alloca:
      case BytecodeOpcode::AllocaLocal:
      case BytecodeOpcode::Load:
      case BytecodeOpcode::LoadI8:
      case BytecodeOpcode::LoadI16:
      case BytecodeOpcode::LoadI32:
      case BytecodeOpcode::LoadI64:
      case BytecodeOpcode::LoadLocal:
      case BytecodeOpcode::Store:
      case BytecodeOpcode::StoreI8:
      case BytecodeOpcode::StoreI16:
      case BytecodeOpcode::StoreI32:
      case BytecodeOpcode::StoreI64:
      case BytecodeOpcode::StoreLocal:
      case BytecodeOpcode::CString:
        Status = executeMemory(Operation, Frame);
        break;
#define INK_SCALAR_CASE(Name) case BytecodeOpcode::Name: Status = executeScalar<BytecodeOpcode::Name>(Operation, Frame); break;
      INK_SCALAR_CASE(AddI8)
      INK_SCALAR_CASE(AddI16)
      INK_SCALAR_CASE(AddI32)
      INK_SCALAR_CASE(AddI64)
      INK_SCALAR_CASE(LogicalNot)
      INK_SCALAR_CASE(LogicalAnd)
      INK_SCALAR_CASE(LogicalOr)
      INK_SCALAR_CASE(CompareSigned8)
      INK_SCALAR_CASE(CompareSigned16)
      INK_SCALAR_CASE(CompareSigned32)
      INK_SCALAR_CASE(CompareSigned64)
      INK_SCALAR_CASE(CompareUnsigned8)
      INK_SCALAR_CASE(CompareUnsigned16)
      INK_SCALAR_CASE(CompareUnsigned32)
      INK_SCALAR_CASE(CompareUnsigned64)
      INK_SCALAR_CASE(CompareBool)
#undef INK_SCALAR_CASE
      case BytecodeOpcode::AddWide:
      case BytecodeOpcode::CompareWide:
        Status = executeWide(Operation, Frame);
        break;
      case BytecodeOpcode::Function:
        Status = executeFunction(Operation, Frame);
        break;
      case BytecodeOpcode::Jump:
        Status = executeJump(Operation, Frame);
        break;
      case BytecodeOpcode::JumpIf:
        Status = executeJumpIf(Operation, Frame);
        break;
      case BytecodeOpcode::CallDirect:
      case BytecodeOpcode::CallIndirect:
        Status = executeCall(Operation, Stack);
        break;
      case BytecodeOpcode::Return:
      case BytecodeOpcode::ReturnVoid:
        Status = executeReturn(Operation, Stack, Result);
        break;
      case BytecodeOpcode::Failure:
        Status = executeFailure(Operation);
        break;
      default:
        Status = ExecutionStatus::UnsupportedOperation;
        break;
      }
    }
    // Normal returns already pop their current frame. An execution failure leaves
    // active callers on the stack; release them from the innermost call outward.
    for (auto Frame = Stack.rbegin(); Frame != Stack.rend(); ++Frame)
    {
      endCall(*Frame);
    }
    return Status == ExecutionStatus::Success ? Result : RuntimeValueResult{Status};
  }
} // namespace ink::execution
