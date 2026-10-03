namespace ink::execution
{
  FORCE_INLINE ExecutionStatus ExecutionMachine::executeFunction(const BytecodeInstruction &Operation, CallFrame &Frame)
  {
    Frame.Slots[Operation.Operands[0]] = Frame.Function->Image->InitialSlots[Operation.Operands[0]];
    return ExecutionStatus::Success;
  }

  FORCE_INLINE ExecutionStatus ExecutionMachine::executeJump(const BytecodeInstruction &Operation, CallFrame &Frame)
  {
    Frame.PC = Operation.Operands[1];
    return ExecutionStatus::Success;
  }

  FORCE_INLINE ExecutionStatus ExecutionMachine::executeJumpIf(const BytecodeInstruction &Operation, CallFrame &Frame)
  {
    if (!Frame.Slots[Operation.Operands[1]].Initialized)
    {
      return ExecutionStatus::RuntimeValue;
    }
    Frame.PC = Frame.Slots[Operation.Operands[1]].Bits ? Operation.Operands[2] : Operation.Operands[3];
    return ExecutionStatus::Success;
  }

  FORCE_INLINE ExecutionStatus ExecutionMachine::executeReturn(const BytecodeInstruction &Operation, std::vector<CallFrame> &Stack, RuntimeValueResult &Result)
  {
    CallFrame &Frame = Stack.back();
    const ExecutableFunction &Image = *Frame.Function->Image;
    Slot Returned = RuntimeValue::fromBits(0, Image.Layouts->get(Image.Signature)->ReturnType);
    if (Operation.Code == BytecodeOpcode::Return)
    {
      Returned = Frame.Slots[Operation.Operands[1]];
      const ExecutionStatus Status = validateValue(Returned);
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
    }
    const SlotId ReturnSlot = Frame.ReturnSlot;
    if (Stack.size() == 1)
    {
      Result = {ExecutionStatus::Success, Returned};
    }
    endCall(Frame);
    Stack.pop_back();
    if (!Stack.empty())
    {
      Stack.back().Slots[ReturnSlot] = std::move(Returned);
    }
    return ExecutionStatus::Success;
  }

  FORCE_INLINE ExecutionStatus ExecutionMachine::executeFailure(const BytecodeInstruction &Operation)
  {
    return static_cast<ExecutionStatus>(Operation.Operands[1]);
  }
} // namespace ink::execution
