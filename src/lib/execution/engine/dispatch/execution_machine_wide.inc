namespace ink::execution
{
  FORCE_INLINE ExecutionStatus ExecutionMachine::executeWide(const BytecodeInstruction &Operation, CallFrame &Frame)
  {
    if (!Frame.Slots[Operation.Operands[1]].Initialized || !Frame.Slots[Operation.Operands[2]].Initialized)
    {
      return ExecutionStatus::RuntimeValue;
    }
    const RuntimeValue &LeftValue = Frame.Slots[Operation.Operands[1]];
    const RuntimeValue &RightValue = Frame.Slots[Operation.Operands[2]];
    const StorageLayout &Layout = *Frame.Function->Operations[Frame.PC - 1].Layout;
    ExecutionInteger LeftScalar(Layout.BitWidth <= 64 ? Layout.BitWidth : 1, LeftValue.Bits);
    ExecutionInteger RightScalar(Layout.BitWidth <= 64 ? Layout.BitWidth : 1, RightValue.Bits);
    const ExecutionInteger &Left = Layout.BitWidth <= 64 ? LeftScalar : LeftValue.integer();
    const ExecutionInteger &Right = Layout.BitWidth <= 64 ? RightScalar : RightValue.integer();
    const RuntimeTypeId ResultType = Frame.Slots[Operation.Operands[0]].Type;
    if (Operation.Code == BytecodeOpcode::AddWide)
    {
      ExecutionInteger Sum(Layout.BitWidth);
      const ExecutionStatus Status = Left.add(Right, Sum);
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
      Frame.Slots[Operation.Operands[0]] = RuntimeValue::fromInteger(std::move(Sum), ResultType);
    }
    else
    {
      int Ordering = 0;
      const ExecutionStatus Status = Left.compare(Right, Layout.Signed, Ordering);
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
      Frame.Slots[Operation.Operands[0]] = RuntimeValue::fromBits(compareResult(Ordering, Operation.Operands[3]), ResultType);
    }
    return ExecutionStatus::Success;
  }
} // namespace ink::execution
