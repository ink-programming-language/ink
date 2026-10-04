namespace ink::execution
{
  FORCE_INLINE ExecutionStatus ExecutionMachine::executeClass(const BytecodeInstruction &Operation, CallFrame &Frame)
  {
    const ExecutableFunction &Image = *Frame.Function->Image;
    Slot &Destination = Frame.Slots[Operation.Operands[0]];
    if (Operation.Code == BytecodeOpcode::Class)
    {
      const TypeDesc &Layout = *Destination.Layout;
      const std::size_t Count = Layout.classDesc().Fields.size();
      if (Count > Engine.Limits.MaxStorageBytes / sizeof(RuntimeValue))
      {
        return ExecutionStatus::BudgetExceeded;
      }
      const ExecutionStatus Status = Engine.consumeSteps(Count);
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
      std::memset(Destination.Data, 0, Layout.Size);
      for (std::size_t Index = 0; Index < Count; ++Index)
      {
        const Slot &Field = Frame.Slots[arraySourceSlot(Image, Operation.Operands[1] + Index * 4)];
        const ExecutionStatus FieldStatus = validateValue(Field);
        if (FieldStatus != ExecutionStatus::Success)
        {
          return FieldStatus;
        }
        std::memcpy(Destination.Data + Layout.classDesc().Fields[Index].Offset, Field.Data, Field.Layout->Size);
      }
      Destination.Initialized = true;
      return ExecutionStatus::Success;
    }
    const Slot &Object = Frame.Slots[Operation.Operands[1]];
    const ExecutionStatus Status = validateValue(Object);
    if (Status != ExecutionStatus::Success)
    {
      return Status;
    }
    const std::size_t Field = Operation.Operands[2];
    if (Operation.Code == BytecodeOpcode::FieldExtract)
    {
      Destination.copyFrom(Object.Data + Object.Layout->classDesc().Fields[Field].Offset);
      return ExecutionStatus::Success;
    }
    const TypeDesc &PointerLayout = *Object.Layout;
    const TypeDesc &Class = *Image.Layouts->get(PointerLayout.pointerDesc().Pointee);
    Destination.setPointer(static_cast<std::byte *>(Object.pointer()) + Class.classDesc().Fields[Field].Offset);
    return ExecutionStatus::Success;
  }
} // namespace ink::execution
