namespace ink::execution
{
  namespace
  {
    FORCE_INLINE bool compareResult(int Ordering, std::uint32_t Predicate) noexcept
    {
      switch (static_cast<ExecutionPredicate>(Predicate))
      {
      case ExecutionPredicate::Equal:
        return Ordering == 0;
      case ExecutionPredicate::NotEqual:
        return Ordering != 0;
      case ExecutionPredicate::Less:
        return Ordering < 0;
      case ExecutionPredicate::LessEqual:
        return Ordering <= 0;
      case ExecutionPredicate::Greater:
        return Ordering > 0;
      case ExecutionPredicate::GreaterEqual:
        return Ordering >= 0;
      }
      return false;
    }

    FORCE_INLINE std::uint64_t signMask(BytecodeOpcode Code) noexcept
    {
      switch (Code)
      {
      case BytecodeOpcode::CompareSigned8:
        return std::uint64_t{1} << 7;
      case BytecodeOpcode::CompareSigned16:
        return std::uint64_t{1} << 15;
      case BytecodeOpcode::CompareSigned32:
        return std::uint64_t{1} << 31;
      case BytecodeOpcode::CompareSigned64:
        return std::uint64_t{1} << 63;
      default:
        return 0;
      }
    }
  } // namespace

  template <BytecodeOpcode Code>
  FORCE_INLINE ExecutionStatus ExecutionMachine::executeScalar(const BytecodeInstruction &Operation, CallFrame &Frame)
  {
    const Slot &Left = Frame.Slots[Operation.Operands[1]];
    if (!Left.Initialized)
    {
      return ExecutionStatus::RuntimeValue;
    }
    std::uint64_t Bits;
    if constexpr (Code == BytecodeOpcode::LogicalNot)
    {
      Bits = Left.Bits ^ 1;
    }
    else
    {
      const Slot &Right = Frame.Slots[Operation.Operands[2]];
      if (!Right.Initialized)
      {
        return ExecutionStatus::RuntimeValue;
      }
      if constexpr (Code == BytecodeOpcode::AddI8)
      {
        Bits = static_cast<std::uint8_t>(Left.Bits + Right.Bits);
      }
      else if constexpr (Code == BytecodeOpcode::AddI16)
      {
        Bits = static_cast<std::uint16_t>(Left.Bits + Right.Bits);
      }
      else if constexpr (Code == BytecodeOpcode::AddI32)
      {
        Bits = static_cast<std::uint32_t>(Left.Bits + Right.Bits);
      }
      else if constexpr (Code == BytecodeOpcode::AddI64)
      {
        Bits = Left.Bits + Right.Bits;
      }
      else if constexpr (Code == BytecodeOpcode::LogicalAnd)
      {
        Bits = Left.Bits & Right.Bits;
      }
      else if constexpr (Code == BytecodeOpcode::LogicalOr)
      {
        Bits = Left.Bits | Right.Bits;
      }
      else
      {
        const std::uint64_t Mask = signMask(Code);
        const std::uint64_t LeftBits = Left.Bits ^ Mask;
        const std::uint64_t RightBits = Right.Bits ^ Mask;
        Bits = compareResult(LeftBits < RightBits ? -1 : LeftBits > RightBits ? 1 : 0, Operation.Operands[3]);
      }
    }
    Slot &Destination = Frame.Slots[Operation.Operands[0]];
    Destination.Bits = Bits;
    Destination.Initialized = true;
    return ExecutionStatus::Success;
  }
} // namespace ink::execution
