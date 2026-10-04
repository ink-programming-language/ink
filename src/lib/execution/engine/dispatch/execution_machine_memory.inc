namespace ink::execution
{
  namespace
  {
    FORCE_INLINE bool isStore(BytecodeOpcode Code) noexcept
    {
      return Code == BytecodeOpcode::Store || Code == BytecodeOpcode::StoreLocal || Code == BytecodeOpcode::StoreI8 || Code == BytecodeOpcode::StoreI16 || Code == BytecodeOpcode::StoreI32 || Code == BytecodeOpcode::StoreI64;
    }
  } // namespace

  FORCE_INLINE ExecutionStatus ExecutionMachine::executeMemory(const BytecodeInstruction &Operation, CallFrame &Frame)
  {
    const ExecutableFunction &Image = *Frame.Function->Image;
    const PreparedOperation &Plan = Frame.Function->Operations[Frame.PC - 1];
    ExecutionMemoryManager &Memory = Engine.Heap.memoryManager();
    Slot &Destination = Frame.Slots[Operation.Operands[0]];
    if (Operation.Code == BytecodeOpcode::AllocaLocal)
    {
      const ExecutionStatus Status = Memory.chargeLocalAllocation();
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
      // Every dynamic allocation resets the slot, including loop iterations.
      Slot &Local = Frame.Locals[Operation.Operands[2]];
      Local.Initialized = false;
      Destination.setBits(Operation.Operands[2]);
      return ExecutionStatus::Success;
    }
    if (Operation.Code == BytecodeOpcode::Alloca)
    {
      const ExecutionPlaceResult Allocated = Memory.allocateCell(*Plan.Layout);
      if (!Allocated)
      {
        return Allocated.Status;
      }
      Frame.Storage.push_back(Allocated.Place.storage());
      Destination.setPointer(Allocated.Place.storage().cell()->data());
      return ExecutionStatus::Success;
    }
    if (Operation.Code == BytecodeOpcode::CString)
    {
      const std::size_t Length = Operation.Operands[2];
      const char *Bytes = Length ? Image.ConstantData.data() + Operation.Operands[1] : "";
      const ExecutionStorageRef Buffer = Memory.allocateBuffer(std::string_view(Bytes, Length));
      if (!Buffer.valid())
      {
        return Memory.lastStatus();
      }
      Frame.Storage.push_back(Buffer);
      Destination.setPointer(Buffer.buffer()->data());
      return ExecutionStatus::Success;
    }
    const Slot &Address = Frame.Slots[Operation.Operands[1]];
    if (!Address.Initialized)
    {
      return ExecutionStatus::RuntimeValue;
    }
    const bool Store = isStore(Operation.Code);
    const Slot *Stored = Store ? &Frame.Slots[Operation.Operands[2]] : nullptr;
    if (Stored && !Stored->Initialized)
    {
      return ExecutionStatus::RuntimeValue;
    }
    if (Operation.Code == BytecodeOpcode::LoadLocal || Operation.Code == BytecodeOpcode::StoreLocal)
    {
      Slot &Local = Frame.Locals[Address.bits()];
      if (Store)
      {
        Local.copyFrom(Stored->Data);
        Destination.Initialized = true;
        return ExecutionStatus::Success;
      }
      if (!Local.Initialized)
      {
        return ExecutionStatus::Uninitialized;
      }
      Destination.copyFrom(Local.Data);
      return ExecutionStatus::Success;
    }
    const ExecutionStatus StepStatus = Engine.consumeStep();
    if (StepStatus != ExecutionStatus::Success)
    {
      return StepStatus;
    }
    const ExecutionStatus Status = Store ? Memory.storeBytes(Address.pointer(), *Plan.Layout, Stored->Data) : Memory.loadBytes(Address.pointer(), *Plan.Layout, Destination.Data);
    Destination.Initialized = Status == ExecutionStatus::Success;
    return Status;
  }
} // namespace ink::execution
