#include "ink/execution/engine/execution_machine.h"

#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/engine/execution_linker.h"
#include "ink/core/object_layout.h"

namespace ink::execution
{
  const RuntimeTypeTable *ExecutionMachine::types() const noexcept
  {
    return Linker.layouts();
  }

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
    PreparedBytes = 0;
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
    core::ObjectLayoutBuilder FrameLayout(Engine.Limits.MaxStorageBytes);
    for (RuntimeTypeId Type : Image->SlotTypes)
    {
      const TypeDesc &Layout = *Image->Layouts->get(Type);
      const auto Offset = FrameLayout.append(Layout.Size, Layout.Alignment);
      if (!Offset)
      {
        Status = ExecutionStatus::BudgetExceeded;
        return nullptr;
      }
      Prepared->SlotOffsets.push_back(static_cast<std::size_t>(*Offset));
    }
    Prepared->LocalLayouts.resize(Image->LocalStorageCount);
    Prepared->LocalOffsets.resize(Image->LocalStorageCount);
    for (const BytecodeInstruction &Operation : Image->Code)
    {
      if (Operation.Code == BytecodeOpcode::AllocaLocal)
      {
        const TypeDesc &Layout = *Image->Layouts->get(Operation.Operands[1]);
        const auto Offset = FrameLayout.append(Layout.Size, Layout.Alignment);
        if (!Offset)
        {
          Status = ExecutionStatus::BudgetExceeded;
          return nullptr;
        }
        Prepared->LocalLayouts[Operation.Operands[2]] = &Layout;
        Prepared->LocalOffsets[Operation.Operands[2]] = static_cast<std::size_t>(*Offset);
      }
    }
    const auto FrameSize = FrameLayout.size();
    if (!FrameSize || *FrameSize > Engine.Limits.MaxStorageBytes - PreparedBytes)
    {
      Status = ExecutionStatus::BudgetExceeded;
      return nullptr;
    }
    Prepared->FrameSize = static_cast<std::size_t>(*FrameSize);
    Prepared->FrameAlignment = static_cast<std::size_t>(FrameLayout.alignment());
    Prepared->InitialBytes = allocateRuntimeBytes(Prepared->FrameSize, Prepared->FrameAlignment);
    for (std::size_t Index = 0; Index < Image->InitialSlots.size(); ++Index)
    {
      const RuntimeValue &Value = Image->InitialSlots[Index];
      if (Value.Initialized)
      {
        Status = importValue(*Image->Layouts->get(Value.Type), Prepared->InitialBytes.get() + Prepared->SlotOffsets[Index], Value);
        if (Status != ExecutionStatus::Success)
        {
          return nullptr;
        }
      }
    }
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
      case BytecodeOpcode::LoadI16:
      case BytecodeOpcode::StoreI16:
      case BytecodeOpcode::LoadI32:
      case BytecodeOpcode::StoreI32:
      case BytecodeOpcode::LoadI64:
      case BytecodeOpcode::StoreI64:
      case BytecodeOpcode::Load:
      case BytecodeOpcode::Store:
        Plan.Layout = Image->Layouts->get(Image->Layouts->get(Image->SlotTypes[Operation.Operands[1]])->pointerDesc().Pointee);
        break;
      default:
        break;
      }
      Prepared->Operations.push_back(Plan);
    }
    PreparedFunction *Result = Prepared.get();
    PreparedBytes += Prepared->FrameSize;
    Functions.emplace(Function, std::move(Prepared));
    return Result;
  }

  ExecutionStatus ExecutionMachine::initializeFrame(CallFrame &Frame, PreparedFunction &Function)
  {
    if (Function.FrameSize > Engine.Limits.MaxStorageBytes - ActiveFrameBytes)
    {
      return ExecutionStatus::BudgetExceeded;
    }
    Frame.Function = &Function;
    Frame.Bytes = allocateRuntimeBytes(Function.FrameSize, Function.FrameAlignment);
    ActiveFrameBytes += Function.FrameSize;
    std::memcpy(Frame.Bytes.get(), Function.InitialBytes.get(), Function.FrameSize);
    Frame.Slots.reserve(Function.SlotOffsets.size());
    for (std::size_t Index = 0; Index < Function.SlotOffsets.size(); ++Index)
    {
      Frame.Slots.push_back({Frame.Bytes.get() + Function.SlotOffsets[Index], Function.Image->Layouts->get(Function.Image->SlotTypes[Index]), Function.Image->InitialSlots[Index].Initialized});
    }
    Frame.Locals.reserve(Function.LocalOffsets.size());
    for (std::size_t Index = 0; Index < Function.LocalOffsets.size(); ++Index)
    {
      Frame.Locals.push_back({Frame.Bytes.get() + Function.LocalOffsets[Index], Function.LocalLayouts[Index], false});
    }
    return ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionMachine::importValue(const TypeDesc &Layout, void *Destination, const RuntimeValue &Value)
  {
    return Engine.Heap.memoryManager().writeValueBytes(Layout, Destination, Value);
  }
  ExecutionStatus ExecutionMachine::validateValue(const Slot &Value) const noexcept
  {
    return Value.Initialized ? ExecutionStatus::Success : ExecutionStatus::RuntimeValue;
  }

  ExecutionStatus ExecutionMachine::validateValue(const RuntimeValue &Value) const noexcept
  {
    if (!Value.Initialized)
    {
      return ExecutionStatus::RuntimeValue;
    }
    if (Value.kind() == RuntimeKind::Array || Value.kind() == RuntimeKind::Class)
    {
      for (const RuntimeValue &Element : Value.aggregate())
      {
        const ExecutionStatus Status = validateValue(Element);
        if (Status != ExecutionStatus::Success)
        {
          return Status;
        }
      }
    }
    return Value.kind() == RuntimeKind::Pointer ? Engine.Heap.memoryManager().validatePointer(Value.pointer()) : ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionMachine::validateArgument(const RuntimeValue &Value, const TypeDesc &Layout)
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
      Valid = Layout.bitWidth() > 64 ? Value.kind() == RuntimeKind::Integer && Value.integer().valid() && Value.integer().bitWidth() == Layout.bitWidth() : Layout.bitWidth() != 0 && !Value.Object && (Layout.bitWidth() == 64 || (Value.Bits >> Layout.bitWidth()) == 0);
      break;
    case RuntimeKind::Float:
      Valid = !Value.Object && ((Layout.bitWidth() == 16 || Layout.bitWidth() == 32) ? (Value.Bits >> Layout.bitWidth()) == 0 : Layout.bitWidth() == 64);
      break;
    case RuntimeKind::String:
      Valid = Value.kind() == RuntimeKind::String;
      break;
    case RuntimeKind::Array:
      if (Value.kind() != RuntimeKind::Array || Value.array().size() != Layout.arrayDesc().ElementCount || !Layout.arrayDesc().ElementLayout)
      {
        break;
      }
      for (const RuntimeValue &Element : Value.array())
      {
        if (Element.Type != Layout.arrayDesc().ElementType)
        {
          return ExecutionStatus::TypeMismatch;
        }
        const ExecutionStatus ElementStatus = validateArgument(Element, *Layout.arrayDesc().ElementLayout);
        if (ElementStatus != ExecutionStatus::Success)
        {
          return ElementStatus;
        }
      }
      Valid = true;
      break;
    case RuntimeKind::Class:
      if (Value.kind() != RuntimeKind::Class || Value.fields().size() != Layout.classDesc().Fields.size())
      {
        break;
      }
      for (std::size_t Index = 0; Index < Value.fields().size(); ++Index)
      {
        if (Value.fields()[Index].Type != Layout.classDesc().Fields[Index].Type)
        {
          return ExecutionStatus::TypeMismatch;
        }
        const ExecutionStatus FieldStatus = validateArgument(Value.fields()[Index], *Layout.classDesc().Fields[Index].Layout);
        if (FieldStatus != ExecutionStatus::Success)
        {
          return FieldStatus;
        }
      }
      Valid = true;
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
    if (Frame.Bytes)
    {
      ActiveFrameBytes -= Frame.Function->FrameSize;
    }
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
    const TypeDesc *Signature = Layouts->get(Descriptor->Signature);
    if (!Signature || Signature->Kind != RuntimeKind::Function || !Layouts->get(Signature->functionDesc().ReturnType))
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
