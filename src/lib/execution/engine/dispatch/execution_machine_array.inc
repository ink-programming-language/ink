namespace ink::execution
{
  namespace
  {
    ExecutionStatus arrayIndex(const RuntimeValue &Value, const StorageLayout &Layout, std::uint64_t Count, std::size_t &Index)
    {
      std::uint64_t Bits = Value.Bits;
      if (Layout.BitWidth > 64)
      {
        if (Layout.Signed && Value.integer().isNegative())
        {
          return ExecutionStatus::IndexOutOfBounds;
        }
        const auto Integer = Value.integer().bits();
        for (std::size_t Word = 1; Word < Integer.words().size(); ++Word)
        {
          if (Integer.words()[Word])
          {
            return ExecutionStatus::IndexOutOfBounds;
          }
        }
        Bits = Value.integer().lowWord();
      }
      else if (Layout.Signed && (Bits & (std::uint64_t{1} << (Layout.BitWidth - 1))))
      {
        return ExecutionStatus::IndexOutOfBounds;
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
    const RuntimeTypeId DestinationType = Destination.Type;
    if (Operation.Code == BytecodeOpcode::Array || Operation.Code == BytecodeOpcode::ArrayRepeat)
    {
      const StorageLayout &Layout = *Image.Layouts->get(DestinationType);
      if (Layout.ElementCount > Engine.Limits.MaxStorageBytes / sizeof(RuntimeValue))
      {
        return ExecutionStatus::BudgetExceeded;
      }
      const std::size_t Count = static_cast<std::size_t>(Layout.ElementCount);
      const ExecutionStatus Status = Engine.consumeSteps(Count);
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
      std::vector<RuntimeValue> Elements;
      Elements.reserve(Count);
      // Repeat evaluates and validates its source even when the array is empty.
      if (Operation.Code == BytecodeOpcode::ArrayRepeat)
      {
        const RuntimeValue &Element = Frame.Slots[Operation.Operands[1]];
        const ExecutionStatus ElementStatus = validateValue(Element);
        if (ElementStatus != ExecutionStatus::Success)
        {
          return ElementStatus;
        }
        Elements.assign(Count, Element);
      }
      else
      {
        for (std::size_t Index = 0; Index < Count; ++Index)
        {
          const RuntimeValue &Element = Frame.Slots[arraySourceSlot(Image, Operation.Operands[1] + Index * 4)];
          const ExecutionStatus ElementStatus = validateValue(Element);
          if (ElementStatus != ExecutionStatus::Success)
          {
            return ElementStatus;
          }
          Elements.push_back(Element);
        }
      }
      Destination = RuntimeValue::fromArray(std::move(Elements), DestinationType);
      return ExecutionStatus::Success;
    }
    const RuntimeValue &Object = Frame.Slots[Operation.Operands[1]];
    const RuntimeValue &Subscript = Frame.Slots[Operation.Operands[2]];
    for (const RuntimeValue *Value : {&Object, &Subscript})
    {
      const ExecutionStatus Status = validateValue(*Value);
      if (Status != ExecutionStatus::Success)
      {
        return Status;
      }
    }
    const bool Address = Operation.Code == BytecodeOpcode::ArrayElementPointer;
    const StorageLayout &ObjectType = *Image.Layouts->get(Object.Type);
    const StorageLayout &Array = Address ? *Image.Layouts->get(ObjectType.Pointee) : ObjectType;
    std::size_t Index = 0;
    const ExecutionStatus Status = arrayIndex(Subscript, *Image.Layouts->get(Subscript.Type), Array.ElementCount, Index);
    if (Status != ExecutionStatus::Success)
    {
      return Status;
    }
    if (!Address)
    {
      if (Object.kind() != RuntimeKind::Array || Object.array().size() != Array.ElementCount)
      {
        return ExecutionStatus::TypeMismatch;
      }
      Destination = Object.array()[Index];
      return validateValue(Destination);
    }
    const ExecutionPointer &Pointer = Object.pointer();
    if (Pointer.kind() != ExecutionPointer::Kind::Place)
    {
      return Pointer.kind() == ExecutionPointer::Kind::Null ? ExecutionStatus::InvalidPlace : ExecutionStatus::UnsupportedOperation;
    }
    const ExecutionCell &Cell = *Pointer.place().storage().cell();
    if (Cell.layout().Domain != Array.Domain)
    {
      return ExecutionStatus::ForeignContext;
    }
    if (!Cell.elementLayout(Pointer.offset(), Array.Type) || !Array.ElementLayout || (Array.ElementLayout->Size && Index > (std::numeric_limits<std::size_t>::max() - Pointer.offset()) / Array.ElementLayout->Size))
    {
      return ExecutionStatus::InvalidPlace;
    }
    const std::size_t Offset = Pointer.offset() + Index * Array.ElementLayout->Size;
    Destination = RuntimeValue::fromPointer(ExecutionPointer::fromPlace(Pointer.place(), Offset), DestinationType);
    return ExecutionStatus::Success;
  }
} // namespace ink::execution
