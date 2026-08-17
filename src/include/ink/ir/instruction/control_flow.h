#ifndef INK_IR_CONTROL_FLOW_H
#define INK_IR_CONTROL_FLOW_H

#include "ink/ir/instruction/instruction.h"

#include <cstdint>
#include <memory>
#include <vector>

namespace ink::ir
{
  enum class ComparePredicate : std::uint8_t
  {
    Equal,
    NotEqual,
    LessThan,
    LessEqual,
    GreaterThan,
    GreaterEqual,
    Count,
  };

  const char *comparePredicateName(ComparePredicate Predicate) noexcept;

  struct BlockTarget
  {
      BlockId Block;
  };

  struct PhiIncoming
  {
      ValueHandle Value;
      BlockId Predecessor;
  };

  class PhiInstruction final : public Instruction
  {
    public:
      explicit PhiInstruction(const Type &ResultType) noexcept
          : Instruction(InstructionKind::Phi),
            ResultType(&ResultType)
      {
      }

      ValueId Result;
      const Type *ResultType;
      std::vector<PhiIncoming> IncomingValues;
  };

  class CompareInstruction final : public Instruction
  {
    public:
      explicit CompareInstruction(const Type &ResultType) noexcept
          : Instruction(InstructionKind::Compare),
            ResultType(&ResultType)
      {
      }

      ValueId Result;
      const Type *ResultType;
      ComparePredicate Predicate = ComparePredicate::Equal;
      ValueHandle Left;
      ValueHandle Right;
  };

  class BranchInstruction final : public Instruction
  {
    public:
      BranchInstruction() noexcept
          : Instruction(InstructionKind::Branch)
      {
      }

      BlockTarget Target;
  };

  class ConditionalBranchInstruction final : public Instruction
  {
    public:
      ConditionalBranchInstruction() noexcept
          : Instruction(InstructionKind::ConditionalBranch)
      {
      }

      ValueHandle Condition;
      BlockTarget TrueTarget;
      BlockTarget FalseTarget;
  };
} // namespace ink::ir

#endif
