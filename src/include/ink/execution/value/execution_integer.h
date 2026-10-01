#ifndef INK_EXECUTION_VALUE_EXECUTION_INTEGER_H
#define INK_EXECUTION_VALUE_EXECUTION_INTEGER_H

#include "ink/execution/support/execution_result.h"
#include "ink/ir/constant/integer_constant.h"

#include <cstdint>
#include <vector>

namespace ink::execution
{
  // Owned fixed-width two's-complement bits. Signedness is supplied by the
  // operation, and every arithmetic result wraps to the declared bit width.
  class ExecutionInteger final
  {
    public:
      explicit ExecutionInteger(const ir::IntegerBits &Bits);
      explicit ExecutionInteger(std::uint32_t BitWidth, std::uint64_t LowWord = 0);

      std::uint32_t bitWidth() const noexcept
      {
        return BitWidth;
      }

      bool valid() const noexcept;
      bool isZero() const noexcept;
      bool isNegative() const noexcept;
      ir::IntegerBits bits() const;

      // Invalid inputs or mismatched widths fail without changing the output.
      // Result may alias either input; signed division truncates toward zero.
      ExecutionStatus add(const ExecutionInteger &Right, ExecutionInteger &Result) const;
      ExecutionStatus subtract(const ExecutionInteger &Right, ExecutionInteger &Result) const;
      ExecutionStatus multiply(const ExecutionInteger &Right, ExecutionInteger &Result) const;
      ExecutionStatus divide(const ExecutionInteger &Right, bool Signed, ExecutionInteger &Result) const;
      ExecutionStatus remainder(const ExecutionInteger &Right, bool Signed, ExecutionInteger &Result) const;
      ExecutionStatus negate(ExecutionInteger &Result) const;
      ExecutionStatus invert(ExecutionInteger &Result) const;
      ExecutionStatus bitwiseAnd(const ExecutionInteger &Right, ExecutionInteger &Result) const;
      ExecutionStatus bitwiseOr(const ExecutionInteger &Right, ExecutionInteger &Result) const;
      ExecutionStatus bitwiseXor(const ExecutionInteger &Right, ExecutionInteger &Result) const;
      ExecutionStatus shiftLeft(std::uint32_t Amount, ExecutionInteger &Result) const;
      ExecutionStatus shiftRight(std::uint32_t Amount, bool Arithmetic, ExecutionInteger &Result) const;
      ExecutionStatus compare(const ExecutionInteger &Right, bool Signed, int &Result) const noexcept;
      ExecutionStatus shiftAmount(std::uint32_t Limit, bool Signed, std::uint32_t &Result) const noexcept;

    private:
      enum class BitwiseOperation
      {
        And,
        Or,
        Xor,
      };

      bool compatible(const ExecutionInteger &Right) const noexcept;
      void maskUnusedBits() noexcept;
      int compareUnsigned(const ExecutionInteger &Right) const noexcept;
      void subtractUnchecked(const ExecutionInteger &Right) noexcept;
      void negateUnchecked() noexcept;
      void divideUnsigned(const ExecutionInteger &Right, ExecutionInteger &Quotient, ExecutionInteger &Remainder) const;
      ExecutionStatus quotientOrRemainder(const ExecutionInteger &Right, bool Signed, bool WantRemainder, ExecutionInteger &Result) const;
      ExecutionStatus bitwise(const ExecutionInteger &Right, BitwiseOperation Operation, ExecutionInteger &Result) const;

      std::uint32_t BitWidth;
      std::vector<std::uint64_t> Words;
  };
} // namespace ink::execution

#endif
