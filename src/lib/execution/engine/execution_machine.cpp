#include "ink/execution/engine/execution_machine.h"

#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/engine/execution_linker.h"

namespace ink::execution
{
  ExecutionMachine::ExecutionMachine(ExecutionEngine &Engine, ExecutionLinker &Linker)
      : Engine(Engine),
        Linker(Linker)
  {
  }

  void ExecutionMachine::clearNativeSymbols() noexcept
  {
    NativeCalls.clear();
  }

  void ExecutionMachine::clearCode() noexcept
  {
    Functions.clear();
    NativeCalls.clear();
  }

  ExecutionMachine::PreparedFunction *ExecutionMachine::prepare(FunctionId Function, ExecutionStatus &Status)
  {
    const auto Found = Functions.find(Function);
    if (Found != Functions.end())
    {
      return Found->second.get();
    }
    const ExecutableFunction *Image = Linker.prepare(Function, Status);
    if (!Image)
    {
      return nullptr;
    }
    auto Prepared = std::make_unique<PreparedFunction>();
    Prepared->Image = Image;
    Prepared->CallTargets.resize(Image->Calls.size(), nullptr);
    for (const ExecutionCallSite &Call : Image->Calls)
    {
      Prepared->CallDescriptors.push_back(Call.Target == InvalidFunction ? nullptr : Linker.descriptor(Call.Target));
    }
    Prepared->Operations.reserve(Image->Code.size());
    for (const BytecodeInstruction &Operation : Image->Code)
    {
      std::size_t Cost = 1;
      for (BytecodeOperandKind Kind : bytecodeInstructionMetadata(Operation.Code)->Operands)
      {
        Cost += Kind == BytecodeOperandKind::ReadSlot;
      }
      if (Operation.Code == BytecodeOpcode::Alloca || Operation.Code == BytecodeOpcode::AllocaLocal || Operation.Code == BytecodeOpcode::LoadLocal || Operation.Code == BytecodeOpcode::StoreLocal)
      {
        ++Cost;
      }
      if (Operation.Code == BytecodeOpcode::CallDirect || Operation.Code == BytecodeOpcode::CallIndirect)
      {
        Cost += 1 + Image->Calls[Operation.Operands[1]].Arguments.size();
      }
      PreparedOperation Plan;
      Plan.Cost = Cost;
      switch (Operation.Code)
      {
      case BytecodeOpcode::Alloca:
        Plan.Layout = Image->Layouts->get(Operation.Operands[1]);
        break;
      case BytecodeOpcode::AddWide:
      case BytecodeOpcode::CompareWide:
        Plan.Layout = Image->Layouts->get(Image->SlotTypes[Operation.Operands[1]]);
        break;
      case BytecodeOpcode::LoadI8:
      case BytecodeOpcode::StoreI8:
        Plan.ScalarBytes = 1;
        break;
      case BytecodeOpcode::LoadI16:
      case BytecodeOpcode::StoreI16:
        Plan.ScalarBytes = 2;
        break;
      case BytecodeOpcode::LoadI32:
      case BytecodeOpcode::StoreI32:
        Plan.ScalarBytes = 4;
        break;
      case BytecodeOpcode::LoadI64:
      case BytecodeOpcode::StoreI64:
        Plan.ScalarBytes = 8;
        break;
      default:
        break;
      }
      if (Plan.ScalarBytes || Operation.Code == BytecodeOpcode::Load || Operation.Code == BytecodeOpcode::Store)
      {
        const StorageLayout &Pointer = *Image->Layouts->get(Image->SlotTypes[Operation.Operands[1]]);
        Plan.Layout = Image->Layouts->get(Pointer.Pointee);
        Plan.Writable = Pointer.Writable;
      }
      Prepared->Operations.push_back(Plan);
    }
    PreparedFunction *Result = Prepared.get();
    Functions.emplace(Function, std::move(Prepared));
    return Result;
  }

  ExecutionStatus ExecutionMachine::validateValue(const RuntimeValue &Value) const noexcept
  {
    if (!Value.Initialized)
    {
      return ExecutionStatus::RuntimeValue;
    }
    return Value.kind() == RuntimeKind::Pointer ? Engine.Heap.memoryManager().validatePointer(Value.pointer()) : ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionMachine::validateArgument(const RuntimeValue &Value, const StorageLayout &Layout)
  {
    const ExecutionStatus Status = validateValue(Value);
    if (Status != ExecutionStatus::Success)
    {
      return Status;
    }
    bool Valid = false;
    switch (Layout.Kind)
    {
    case RuntimeKind::Void:
      Valid = !Value.Object && Value.Bits == 0;
      break;
    case RuntimeKind::Boolean:
      Valid = !Value.Object && Value.Bits <= 1;
      break;
    case RuntimeKind::Integer:
      Valid = Layout.BitWidth > 64 ? Value.kind() == RuntimeKind::Integer && Value.integer().valid() && Value.integer().bitWidth() == Layout.BitWidth : Layout.BitWidth != 0 && !Value.Object && (Layout.BitWidth == 64 || (Value.Bits >> Layout.BitWidth) == 0);
      break;
    case RuntimeKind::Float:
      Valid = !Value.Object && ((Layout.BitWidth == 16 || Layout.BitWidth == 32) ? (Value.Bits >> Layout.BitWidth) == 0 : Layout.BitWidth == 64);
      break;
    case RuntimeKind::String:
      Valid = Value.kind() == RuntimeKind::String;
      break;
    case RuntimeKind::Pointer:
      Valid = Value.kind() == RuntimeKind::Pointer;
      break;
    case RuntimeKind::Function:
    {
      const auto *Function = !Value.Object && Value.Bits < InvalidFunction ? Linker.descriptor(static_cast<FunctionId>(Value.Bits)) : nullptr;
      Valid = Function && Function->Signature == Layout.Type;
      break;
    }
    case RuntimeKind::Invalid:
      break;
    }
    return Valid ? ExecutionStatus::Success : ExecutionStatus::TypeMismatch;
  }

  ExecutionStatus ExecutionMachine::beginCall(bool Nested)
  {
    if (Nested && Engine.enterEvaluation() != ExecutionStatus::Success)
    {
      return Engine.LastStatus;
    }
    ExecutionStatus Status = Engine.consumeStep();
    if (Status == ExecutionStatus::Success && Engine.ActiveCalls >= Engine.Limits.MaxCallDepth)
    {
      Status = Engine.finishStatus(ExecutionStatus::BudgetExceeded);
    }
    if (Status != ExecutionStatus::Success)
    {
      if (Nested)
      {
        Engine.leaveEvaluation();
      }
      return Status;
    }
    ++Engine.ActiveCalls;
    return ExecutionStatus::Success;
  }

  void ExecutionMachine::endCall(CallFrame &Frame) noexcept
  {
    for (const ExecutionStorageRef &Storage : Frame.Storage)
    {
      Engine.Heap.memoryManager().release(Storage);
    }
    --Engine.ActiveCalls;
    if (Frame.Nested)
    {
      Engine.leaveEvaluation();
    }
  }

  RuntimeValueResult ExecutionMachine::callNative(const RuntimeFunctionDescriptor &Function, std::span<const RuntimeValue> Arguments, bool Nested)
  {
    const ExecutionStatus Status = beginCall(Nested);
    if (Status != ExecutionStatus::Success)
    {
      return {Status};
    }
    RuntimeValueResult Result = NativeCalls.invoke(Engine.Heap.memoryManager(), *Linker.layouts(), Engine.NativeSymbols, Function, Arguments);
    if (Result)
    {
      Result.Status = validateValue(Result.Value);
    }
    --Engine.ActiveCalls;
    if (Nested)
    {
      Engine.leaveEvaluation();
    }
    return Result;
  }

  RuntimeValueResult ExecutionMachine::execute(FunctionId Function, std::span<const RuntimeValue> Arguments)
  {
    Function = Linker.resolveFunction(Function);
    const RuntimeFunctionDescriptor *Descriptor = Linker.descriptor(Function);
    const RuntimeTypeTable *Layouts = Linker.layouts();
    if (!Descriptor || Descriptor->Id != Function || !Layouts)
    {
      return {ExecutionStatus::InvalidArguments};
    }
    const StorageLayout *Signature = Layouts->get(Descriptor->Signature);
    if (!Signature || Signature->Kind != RuntimeKind::Function || !Layouts->get(Signature->ReturnType))
    {
      return {ExecutionStatus::TypeMismatch};
    }
    if (Descriptor->External)
    {
      return callNative(*Descriptor, Arguments, false);
    }
    ExecutionStatus Status = ExecutionStatus::Success;
    PreparedFunction *Prepared = prepare(Function, Status);
    return Prepared ? run(*Prepared, Arguments) : RuntimeValueResult{Status};
  }
} // namespace ink::execution
