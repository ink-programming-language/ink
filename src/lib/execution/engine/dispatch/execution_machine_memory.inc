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
    const RuntimeTypeId DestinationType = Destination.Type;
    if (Operation.Code == BytecodeOpcode::AllocaLocal)
    {
      const ExecutionStatus Status = Memory.chargeLocalAllocation();
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
      // Every dynamic allocation resets the slot, including loop iterations.
      Slot &Local = Frame.Locals[Operation.Operands[2]];
      Local = {};
      Local.Type = Operation.Operands[1];
      Destination = RuntimeValue::fromBits(Operation.Operands[2], DestinationType);
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
      Destination = RuntimeValue::fromPointer(ExecutionPointer::fromPlace(Allocated.Place), DestinationType);
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
      Destination = RuntimeValue::fromPointer(ExecutionPointer::fromBuffer(Buffer), DestinationType);
      return ExecutionStatus::Success;
    }
    const Slot &Address = Frame.Slots[Operation.Operands[1]];
    if (!Address.Initialized)
    {
      return ExecutionStatus::RuntimeValue;
    }
    const bool Store = isStore(Operation.Code);
    const Slot *Stored = Store ? &Frame.Slots[Operation.Operands[2]] : nullptr;
    if (Stored)
    {
      const ExecutionStatus Status = validateValue(*Stored);
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
    }
    if (Operation.Code == BytecodeOpcode::LoadLocal || Operation.Code == BytecodeOpcode::StoreLocal)
    {
      Slot &Local = Frame.Locals[Address.Bits];
      if (Store)
      {
        Local = *Stored;
        Destination = RuntimeValue::fromBits(0, DestinationType);
        return ExecutionStatus::Success;
      }
      if (!Local.Initialized)
      {
        return ExecutionStatus::Uninitialized;
      }
      const ExecutionStatus Status = validateValue(Local);
      if (Status == ExecutionStatus::Success)
      {
        Destination = Local;
      }
      return Status;
    }
    const ExecutionStatus AddressStatus = validateValue(Address);
    if (AddressStatus != ExecutionStatus::Success)
    {
      return AddressStatus;
    }
    const ExecutionPointer &Pointer = Address.pointer();
    const StorageLayout &Pointee = *Plan.Layout;
    if (Store && !Plan.Writable)
    {
      return ExecutionStatus::ReadOnly;
    }
    if (Pointer.kind() == ExecutionPointer::Kind::Place)
    {
      ExecutionCell &Cell = *Pointer.place().storage().cell();
      if (Cell.layout().Domain != Pointee.Domain)
      {
        return ExecutionStatus::ForeignContext;
      }
      if (Pointer.offset() == 0 && Cell.type() == Pointee.Type)
      {
        const ExecutionStatus StepStatus = Engine.consumeStep();
        if (StepStatus != ExecutionStatus::Success)
        {
          return StepStatus;
        }
        if (Store)
        {
          const ExecutionStatus Status = Plan.ScalarBytes && Cell.data() ? Cell.storeBits(Stored->Bits, Plan.ScalarBytes) : Cell.storeRuntime(*Stored);
          if (Status == ExecutionStatus::Success)
          {
            Destination = RuntimeValue::fromBits(0, DestinationType);
          }
          return Status;
        }
        if (Plan.ScalarBytes && Cell.data())
        {
          std::uint64_t Bits = 0;
          const ExecutionStatus Status = Cell.loadBits(Bits, Plan.ScalarBytes);
          if (Status == ExecutionStatus::Success)
          {
            Destination = RuntimeValue::fromBits(Bits, DestinationType);
          }
          return Status;
        }
        const RuntimeValueResult Loaded = Cell.loadRuntime();
        if (!Loaded)
        {
          return Loaded.Status;
        }
        const ExecutionStatus Status = validateValue(Loaded.Value);
        if (Status == ExecutionStatus::Success)
        {
          Destination = Loaded.Value;
        }
        return Status;
      }
      if (!Cell.data() || Pointee.Kind != RuntimeKind::Integer || Pointee.BitWidth != 8 || Pointee.Signed)
      {
        return ExecutionStatus::TypeMismatch;
      }
      if (Pointer.offset() >= Cell.size())
      {
        return ExecutionStatus::InvalidPlace;
      }
      if (Store && !Cell.writable())
      {
        return ExecutionStatus::ReadOnly;
      }
      if (!Cell.initialized())
      {
        return ExecutionStatus::Uninitialized;
      }
      const ExecutionStatus Status = Engine.consumeStep();
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
      auto *Bytes = static_cast<unsigned char *>(Cell.data());
      if (Store)
      {
        Bytes[Pointer.offset()] = static_cast<unsigned char>(Stored->Bits);
        Destination = RuntimeValue::fromBits(0, DestinationType);
      }
      else
      {
        Destination = RuntimeValue::fromBits(Bytes[Pointer.offset()], DestinationType);
      }
      return ExecutionStatus::Success;
    }
    if (Pointer.kind() == ExecutionPointer::Kind::Buffer)
    {
      if (Pointer.offset() >= Pointer.buffer()->size())
      {
        return ExecutionStatus::InvalidPlace;
      }
      if (Pointee.Kind != RuntimeKind::Integer || Pointee.BitWidth != 8)
      {
        return ExecutionStatus::UnsupportedOperation;
      }
      char *Bytes = Pointer.buffer()->data();
      if (Store)
      {
        Bytes[Pointer.offset()] = static_cast<char>(Stored->Bits);
        Destination = RuntimeValue::fromBits(0, DestinationType);
      }
      else
      {
        Destination = RuntimeValue::fromBits(static_cast<unsigned char>(Bytes[Pointer.offset()]), DestinationType);
      }
      return ExecutionStatus::Success;
    }
    // Native addresses can cross the FFI boundary but never enable arbitrary host loads.
    return Pointer.kind() == ExecutionPointer::Kind::Null ? ExecutionStatus::InvalidPlace : ExecutionStatus::UnsupportedOperation;
  }
} // namespace ink::execution
