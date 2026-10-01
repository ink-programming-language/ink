#include "ink/execution/value/execution_integer.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <span>
#include <utility>

namespace ink::execution
{
  ExecutionInteger::ExecutionInteger(const ir::IntegerBits &Bits)
      : BitWidth(Bits.bitWidth()),
        Words(Bits.words().begin(), Bits.words().end())
  {
  }

  ExecutionInteger::ExecutionInteger(std::uint32_t BitWidth, std::uint64_t LowWord)
      : BitWidth(BitWidth),
        Words(static_cast<std::size_t>(BitWidth / 64) + (BitWidth % 64 != 0), 0)
  {
    if (!Words.empty())
    {
      Words.front() = LowWord;
    }
  }

  bool ExecutionInteger::valid() const noexcept
  {
    const std::size_t ExpectedWords = static_cast<std::size_t>(BitWidth / 64) + (BitWidth % 64 != 0);
    if (BitWidth == 0 || Words.size() != ExpectedWords)
    {
      return false;
    }
    return BitWidth % 64 == 0 || (Words.back() >> (BitWidth % 64)) == 0;
  }

  bool ExecutionInteger::isZero() const noexcept
  {
    return valid() && std::all_of(Words.begin(), Words.end(), [](std::uint64_t Word)
    {
      return Word == 0;
    });
  }

  bool ExecutionInteger::isNegative() const noexcept
  {
    return valid() && ((Words.back() >> ((BitWidth - 1) % 64)) & 1) != 0;
  }

  ir::IntegerBits ExecutionInteger::bits() const
  {
    return ir::IntegerBits(BitWidth, std::span<const std::uint64_t>(Words));
  }

  bool ExecutionInteger::compatible(const ExecutionInteger &Right) const noexcept
  {
    return valid() && Right.valid() && BitWidth == Right.BitWidth;
  }

  void ExecutionInteger::maskUnusedBits() noexcept
  {
    if (BitWidth % 64 != 0)
    {
      Words.back() &= (std::uint64_t{1} << (BitWidth % 64)) - 1;
    }
  }

  ExecutionStatus ExecutionInteger::add(const ExecutionInteger &Right, ExecutionInteger &Result) const
  {
    if (!compatible(Right))
    {
      return ExecutionStatus::TypeMismatch;
    }
    ExecutionInteger Sum(BitWidth);
    std::uint64_t Carry = 0;
    for (std::size_t Index = 0; Index < Words.size(); ++Index)
    {
      const std::uint64_t Pair = Words[Index] + Right.Words[Index];
      const bool PairCarry = Pair < Words[Index];
      Sum.Words[Index] = Pair + Carry;
      Carry = PairCarry || Sum.Words[Index] < Pair;
    }
    Sum.maskUnusedBits();
    Result = std::move(Sum);
    return ExecutionStatus::Success;
  }

  void ExecutionInteger::subtractUnchecked(const ExecutionInteger &Right) noexcept
  {
    std::uint64_t Borrow = 0;
    for (std::size_t Index = 0; Index < Words.size(); ++Index)
    {
      const std::uint64_t Pair = Words[Index] - Right.Words[Index];
      const bool PairBorrow = Words[Index] < Right.Words[Index];
      Words[Index] = Pair - Borrow;
      Borrow = PairBorrow || Pair < Borrow;
    }
    maskUnusedBits();
  }

  ExecutionStatus ExecutionInteger::subtract(const ExecutionInteger &Right, ExecutionInteger &Result) const
  {
    if (!compatible(Right))
    {
      return ExecutionStatus::TypeMismatch;
    }
    ExecutionInteger Difference(*this);
    Difference.subtractUnchecked(Right);
    Result = std::move(Difference);
    return ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionInteger::multiply(const ExecutionInteger &Right, ExecutionInteger &Result) const
  {
    if (!compatible(Right))
    {
      return ExecutionStatus::TypeMismatch;
    }
    const std::size_t LimbCount = static_cast<std::size_t>(BitWidth / 32) + (BitWidth % 32 != 0);
    std::vector<std::uint32_t> Product(LimbCount, 0);
    for (std::size_t LeftIndex = 0; LeftIndex < LimbCount; ++LeftIndex)
    {
      const std::uint32_t LeftLimb = static_cast<std::uint32_t>(Words[LeftIndex / 2] >> ((LeftIndex % 2) * 32));
      std::uint64_t Carry = 0;
      for (std::size_t RightIndex = 0; RightIndex < LimbCount - LeftIndex; ++RightIndex)
      {
        const std::uint32_t RightLimb = static_cast<std::uint32_t>(Right.Words[RightIndex / 2] >> ((RightIndex % 2) * 32));
        const std::size_t Index = LeftIndex + RightIndex;
        // A 32-bit product plus one result limb and one carry fits uint64_t.
        const std::uint64_t Accumulated = std::uint64_t{LeftLimb} * RightLimb + Product[Index] + Carry;
        Product[Index] = static_cast<std::uint32_t>(Accumulated);
        Carry = Accumulated >> 32;
      }
    }
    ExecutionInteger Value(BitWidth);
    for (std::size_t Index = 0; Index < LimbCount; ++Index)
    {
      Value.Words[Index / 2] |= std::uint64_t{Product[Index]} << ((Index % 2) * 32);
    }
    Value.maskUnusedBits();
    Result = std::move(Value);
    return ExecutionStatus::Success;
  }

  void ExecutionInteger::negateUnchecked() noexcept
  {
    std::uint64_t Carry = 1;
    for (std::uint64_t &Word : Words)
    {
      Word = ~Word + Carry;
      Carry = Carry && Word == 0;
    }
    maskUnusedBits();
  }

  ExecutionStatus ExecutionInteger::negate(ExecutionInteger &Result) const
  {
    if (!valid())
    {
      return ExecutionStatus::TypeMismatch;
    }
    ExecutionInteger Value(*this);
    Value.negateUnchecked();
    Result = std::move(Value);
    return ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionInteger::invert(ExecutionInteger &Result) const
  {
    if (!valid())
    {
      return ExecutionStatus::TypeMismatch;
    }
    ExecutionInteger Value(*this);
    for (std::uint64_t &Word : Value.Words)
    {
      Word = ~Word;
    }
    Value.maskUnusedBits();
    Result = std::move(Value);
    return ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionInteger::bitwise(const ExecutionInteger &Right, BitwiseOperation Operation, ExecutionInteger &Result) const
  {
    if (!compatible(Right))
    {
      return ExecutionStatus::TypeMismatch;
    }
    ExecutionInteger Value(BitWidth);
    for (std::size_t Index = 0; Index < Words.size(); ++Index)
    {
      switch (Operation)
      {
      case BitwiseOperation::And:
        Value.Words[Index] = Words[Index] & Right.Words[Index];
        break;
      case BitwiseOperation::Or:
        Value.Words[Index] = Words[Index] | Right.Words[Index];
        break;
      case BitwiseOperation::Xor:
        Value.Words[Index] = Words[Index] ^ Right.Words[Index];
        break;
      }
    }
    Result = std::move(Value);
    return ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionInteger::bitwiseAnd(const ExecutionInteger &Right, ExecutionInteger &Result) const
  {
    return bitwise(Right, BitwiseOperation::And, Result);
  }

  ExecutionStatus ExecutionInteger::bitwiseOr(const ExecutionInteger &Right, ExecutionInteger &Result) const
  {
    return bitwise(Right, BitwiseOperation::Or, Result);
  }

  ExecutionStatus ExecutionInteger::bitwiseXor(const ExecutionInteger &Right, ExecutionInteger &Result) const
  {
    return bitwise(Right, BitwiseOperation::Xor, Result);
  }

  ExecutionStatus ExecutionInteger::shiftLeft(std::uint32_t Amount, ExecutionInteger &Result) const
  {
    if (!valid())
    {
      return ExecutionStatus::TypeMismatch;
    }
    if (Amount >= BitWidth)
    {
      return ExecutionStatus::InvalidShift;
    }
    ExecutionInteger Value(BitWidth);
    const std::size_t WordShift = Amount / 64;
    const unsigned BitShift = Amount % 64;
    for (std::size_t Index = WordShift; Index < Words.size(); ++Index)
    {
      Value.Words[Index] = Words[Index - WordShift] << BitShift;
      if (BitShift != 0 && Index > WordShift)
      {
        Value.Words[Index] |= Words[Index - WordShift - 1] >> (64 - BitShift);
      }
    }
    Value.maskUnusedBits();
    Result = std::move(Value);
    return ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionInteger::shiftRight(std::uint32_t Amount, bool Arithmetic, ExecutionInteger &Result) const
  {
    if (!valid())
    {
      return ExecutionStatus::TypeMismatch;
    }
    if (Amount >= BitWidth)
    {
      return ExecutionStatus::InvalidShift;
    }
    ExecutionInteger Value(BitWidth);
    const std::size_t WordShift = Amount / 64;
    const unsigned BitShift = Amount % 64;
    for (std::size_t Index = 0; Index < Words.size() - WordShift; ++Index)
    {
      Value.Words[Index] = Words[Index + WordShift] >> BitShift;
      if (BitShift != 0 && Index + WordShift + 1 < Words.size())
      {
        Value.Words[Index] |= Words[Index + WordShift + 1] << (64 - BitShift);
      }
    }
    if (Arithmetic && Amount != 0 && isNegative())
    {
      const std::uint32_t FirstSignBit = BitWidth - Amount;
      Value.Words[FirstSignBit / 64] |= std::numeric_limits<std::uint64_t>::max() << (FirstSignBit % 64);
      for (std::size_t Index = static_cast<std::size_t>(FirstSignBit / 64) + 1; Index < Words.size(); ++Index)
      {
        Value.Words[Index] = std::numeric_limits<std::uint64_t>::max();
      }
    }
    Value.maskUnusedBits();
    Result = std::move(Value);
    return ExecutionStatus::Success;
  }

  int ExecutionInteger::compareUnsigned(const ExecutionInteger &Right) const noexcept
  {
    for (std::size_t Index = Words.size(); Index-- > 0;)
    {
      if (Words[Index] != Right.Words[Index])
      {
        return Words[Index] < Right.Words[Index] ? -1 : 1;
      }
    }
    return 0;
  }

  ExecutionStatus ExecutionInteger::compare(const ExecutionInteger &Right, bool Signed, int &Result) const noexcept
  {
    if (!compatible(Right))
    {
      return ExecutionStatus::TypeMismatch;
    }
    if (Signed && isNegative() != Right.isNegative())
    {
      Result = isNegative() ? -1 : 1;
    }
    else
    {
      Result = compareUnsigned(Right);
    }
    return ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionInteger::shiftAmount(std::uint32_t Limit, bool Signed, std::uint32_t &Result) const noexcept
  {
    if (!valid())
    {
      return ExecutionStatus::TypeMismatch;
    }
    if ((Signed && isNegative()) || Words.front() >= Limit)
    {
      return ExecutionStatus::InvalidShift;
    }
    for (std::size_t Index = 1; Index < Words.size(); ++Index)
    {
      if (Words[Index] != 0)
      {
        return ExecutionStatus::InvalidShift;
      }
    }
    Result = static_cast<std::uint32_t>(Words.front());
    return ExecutionStatus::Success;
  }

  void ExecutionInteger::divideUnsigned(const ExecutionInteger &Right, ExecutionInteger &Quotient, ExecutionInteger &Remainder) const
  {
    // Restore division one dividend bit at a time. Keep the shifted-out bit
    // separately so a divisor with its highest bit set cannot overflow the trial remainder.
    for (std::uint32_t Bit = BitWidth; Bit-- > 0;)
    {
      const bool Overflow = Remainder.isNegative();
      std::uint64_t Carry = (Words[Bit / 64] >> (Bit % 64)) & 1;
      for (std::uint64_t &Word : Remainder.Words)
      {
        const std::uint64_t NextCarry = Word >> 63;
        Word = (Word << 1) | Carry;
        Carry = NextCarry;
      }
      Remainder.maskUnusedBits();
      if (Overflow || Remainder.compareUnsigned(Right) >= 0)
      {
        Remainder.subtractUnchecked(Right);
        Quotient.Words[Bit / 64] |= std::uint64_t{1} << (Bit % 64);
      }
    }
  }

  ExecutionStatus ExecutionInteger::quotientOrRemainder(const ExecutionInteger &Right, bool Signed, bool WantRemainder, ExecutionInteger &Result) const
  {
    if (!compatible(Right))
    {
      return ExecutionStatus::TypeMismatch;
    }
    if (Right.isZero())
    {
      return ExecutionStatus::DivisionByZero;
    }
    const bool NegativeLeft = Signed && isNegative();
    const bool NegativeRight = Signed && Right.isNegative();
    ExecutionInteger Dividend(*this);
    ExecutionInteger Divisor(Right);
    if (NegativeLeft)
    {
      Dividend.negateUnchecked();
    }
    if (NegativeRight)
    {
      Divisor.negateUnchecked();
    }
    ExecutionInteger Quotient(BitWidth);
    ExecutionInteger Remainder(BitWidth);
    Dividend.divideUnsigned(Divisor, Quotient, Remainder);
    if (WantRemainder)
    {
      if (NegativeLeft)
      {
        Remainder.negateUnchecked();
      }
      Result = std::move(Remainder);
    }
    else
    {
      if (NegativeLeft != NegativeRight)
      {
        Quotient.negateUnchecked();
      }
      Result = std::move(Quotient);
    }
    return ExecutionStatus::Success;
  }

  ExecutionStatus ExecutionInteger::divide(const ExecutionInteger &Right, bool Signed, ExecutionInteger &Result) const
  {
    return quotientOrRemainder(Right, Signed, false, Result);
  }

  ExecutionStatus ExecutionInteger::remainder(const ExecutionInteger &Right, bool Signed, ExecutionInteger &Result) const
  {
    return quotientOrRemainder(Right, Signed, true, Result);
  }
} // namespace ink::execution
