namespace ink::execution
{
  FORCE_INLINE ExecutionStatus ExecutionMachine::executeWide(const BytecodeInstruction &Operation, CallFrame &Frame)
  {
    if (!Frame.Slots[Operation.Operands[1]].Initialized || !Frame.Slots[Operation.Operands[2]].Initialized)
    {
      return ExecutionStatus::RuntimeValue;
    }
    const Slot &LeftValue = Frame.Slots[Operation.Operands[1]];
    const Slot &RightValue = Frame.Slots[Operation.Operands[2]];
    const TypeDesc &Layout = *Frame.Function->Operations[Frame.PC - 1].Layout;
    const ExecutionInteger Left = LeftValue.integer();
    const ExecutionInteger Right = RightValue.integer();
    if (Operation.Code == BytecodeOpcode::AddWide)
    {
      ExecutionInteger Sum(Layout.bitWidth());
      const ExecutionStatus Status = Left.add(Right, Sum);
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
      Frame.Slots[Operation.Operands[0]].setInteger(Sum);
    }
    else
    {
      int Ordering = 0;
      const ExecutionStatus Status = Left.compare(Right, Layout.isSigned(), Ordering);
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
      Frame.Slots[Operation.Operands[0]].setBits(compareResult(Ordering, Operation.Operands[3]));
    }
    return ExecutionStatus::Success;
  }
} // namespace ink::execution
