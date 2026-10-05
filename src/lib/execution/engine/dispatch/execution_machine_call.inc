namespace ink::execution
{
  FORCE_INLINE ExecutionStatus ExecutionMachine::executeCall(const BytecodeInstruction &Operation, std::vector<CallFrame> &Stack)
  {
    CallFrame &Frame = Stack.back();
    const ExecutionCallSite &Site = Frame.Function->Image->Calls[Operation.Operands[1]];
    FunctionId Target = Site.Target;
    if (Target == InvalidFunction)
    {
      const Slot &Callee = Frame.Slots[Site.CalleeSlot];
      if (!Callee.Initialized)
      {
        return ExecutionStatus::RuntimeValue;
      }
      Target = Linker.resolveFunction(static_cast<FunctionId>(Callee.bits()));
    }
    const RuntimeFunctionDescriptor *Descriptor = Site.Target != InvalidFunction ? Frame.Function->CallDescriptors[Operation.Operands[1]] : Linker.descriptor(Target);
    if (!Descriptor || Descriptor->Signature != Site.Signature)
    {
      return ExecutionStatus::TypeMismatch;
    }
    for (SlotId Argument : Site.Arguments)
    {
      const ExecutionStatus Status = validateValue(Frame.Slots[Argument]);
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
    }
    const bool Registered = Linker.nativeBinding(Target) != nullptr;
    if (Descriptor->External || Registered)
    {
      std::vector<RuntimeValue> NativeArguments;
      NativeArguments.reserve(Site.Arguments.size());
      for (SlotId Argument : Site.Arguments)
      {
        const Slot &Source = Frame.Slots[Argument];
        const RuntimeValueResult Value = readStorage(*Source.Layout, Source.Data);
        if (!Value)
        {
          return Value.Status;
        }
        NativeArguments.push_back(Value.Value);
      }
      const RuntimeValueResult NativeResult = Registered ? callRegistered(Target, NativeArguments, true) : callNative(*Descriptor, NativeArguments, true);
      if (NativeResult)
      {
        Slot &Destination = Frame.Slots[Operation.Operands[0]];
        const ExecutionStatus Status = importValue(*Destination.Layout, Destination.Data, NativeResult.Value);
        Destination.Initialized = Status == ExecutionStatus::Success;
        return Status;
      }
      return NativeResult.Status;
    }
    ExecutionStatus Status = ExecutionStatus::Success;
    PreparedFunction *Callee = Site.Target != InvalidFunction ? Frame.Function->CallTargets[Operation.Operands[1]] : nullptr;
    if (!Callee)
    {
      Callee = prepare(Target, Status);
      if (!Callee)
      {
        return Status;
      }
      if (Site.Target != InvalidFunction)
      {
        Frame.Function->CallTargets[Operation.Operands[1]] = Callee;
      }
    }
    Status = beginCall(true);
    if (Status != ExecutionStatus::Success)
    {
      return Status;
    }
    CallFrame Next;
    Next.ReturnSlot = Operation.Operands[0];
    Next.Nested = true;
    Status = initializeFrame(Next, *Callee);
    if (Status != ExecutionStatus::Success)
    {
      endCall(Next);
      return Status;
    }
    for (std::size_t Index = 0; Index < Site.Arguments.size(); ++Index)
    {
      Next.Slots[Index].copyFrom(Frame.Slots[Site.Arguments[Index]].Data);
    }
    // Growing the stack can invalidate Frame; finish every caller access first.
    Stack.push_back(std::move(Next));
    return ExecutionStatus::Success;
  }
} // namespace ink::execution
