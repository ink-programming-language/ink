namespace ink::execution
{
  namespace
  {
    ExecutionStatus arrayIndex(const RuntimeSlot &Value, const TypeDesc &Layout, std::uint64_t Count, std::size_t &Index)
    {
      std::uint64_t Bits = 0;
      if (Layout.bitWidth() > 64)
      {
        const ExecutionInteger Wide = Value.integer();
        if (Layout.isSigned() && Wide.isNegative())
        {
          return ExecutionStatus::IndexOutOfBounds;
        }
        const auto Integer = Wide.bits();
        for (std::size_t Word = 1; Word < Integer.words().size(); ++Word)
        {
          if (Integer.words()[Word])
          {
            return ExecutionStatus::IndexOutOfBounds;
          }
        }
        Bits = Wide.lowWord();
      }
      else
      {
        Bits = Value.bits();
        if (Layout.isSigned() && (Bits & (std::uint64_t{1} << (Layout.bitWidth() - 1))))
        {
          return ExecutionStatus::IndexOutOfBounds;
        }
      }
      if (Bits >= Count || Bits > std::numeric_limits<std::size_t>::max())
      {
        return ExecutionStatus::IndexOutOfBounds;
      }
      Index = static_cast<std::size_t>(Bits);
      return ExecutionStatus::Success;
    }
  } // namespace

  FORCE_INLINE ExecutionStatus ExecutionMachine::executeArray(const BytecodeInstruction &Operation, CallFrame &Frame)
  {
    const ExecutableFunction &Image = *Frame.Function->Image;
    Slot &Destination = Frame.Slots[Operation.Operands[0]];
    if (Operation.Code == BytecodeOpcode::Array || Operation.Code == BytecodeOpcode::ArrayRepeat)
    {
      const TypeDesc &Layout = *Destination.Layout;
      if (Layout.arrayDesc().ElementCount > Engine.Limits.MaxStorageBytes / sizeof(RuntimeValue))
      {
        return ExecutionStatus::BudgetExceeded;
      }
      const std::size_t Count = static_cast<std::size_t>(Layout.arrayDesc().ElementCount);
      const ExecutionStatus Status = Engine.consumeSteps(Count);
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
      // Repeat evaluates and validates its source even when the array is empty.
      if (Operation.Code == BytecodeOpcode::ArrayRepeat)
      {
        const Slot &Element = Frame.Slots[Operation.Operands[1]];
        const ExecutionStatus ElementStatus = validateValue(Element);
        if (ElementStatus != ExecutionStatus::Success)
        {
          return ElementStatus;
        }
        for (std::size_t Index = 0; Index < Count; ++Index)
        {
          std::memcpy(Destination.Data + Index * Element.Layout->Size, Element.Data, Element.Layout->Size);
        }
      }
      else
      {
        for (std::size_t Index = 0; Index < Count; ++Index)
        {
          const Slot &Element = Frame.Slots[arraySourceSlot(Image, Operation.Operands[1] + Index * 4)];
          const ExecutionStatus ElementStatus = validateValue(Element);
          if (ElementStatus != ExecutionStatus::Success)
          {
            return ElementStatus;
          }
          std::memcpy(Destination.Data + Index * Element.Layout->Size, Element.Data, Element.Layout->Size);
        }
      }
      Destination.Initialized = true;
      return ExecutionStatus::Success;
    }
    const Slot &Object = Frame.Slots[Operation.Operands[1]];
    const Slot &Subscript = Frame.Slots[Operation.Operands[2]];
    for (const Slot *Value : {&Object, &Subscript})
    {
      const ExecutionStatus Status = validateValue(*Value);
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
    }
    const bool Address = Operation.Code == BytecodeOpcode::ArrayElementPointer;
    const TypeDesc &ObjectType = *Object.Layout;
    const TypeDesc &Array = Address ? *Image.Layouts->get(ObjectType.pointerDesc().Pointee) : ObjectType;
    std::size_t Index = 0;
    const ExecutionStatus Status = arrayIndex(Subscript, *Subscript.Layout, Array.arrayDesc().ElementCount, Index);
    if (Status != ExecutionStatus::Success)
    {
      return Status;
    }
    if (!Address)
    {
      Destination.copyFrom(Object.Data + Index * Array.arrayDesc().ElementLayout->Size);
      return ExecutionStatus::Success;
    }
    Destination.setPointer(static_cast<std::byte *>(Object.pointer()) + Index * Array.arrayDesc().ElementLayout->Size);
    return ExecutionStatus::Success;
  }
} // namespace ink::execution
