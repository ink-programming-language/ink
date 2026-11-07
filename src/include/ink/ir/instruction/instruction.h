#pragma once

#include "ink/ir/instruction/instruction_kind.h"
#include "ink/ir/value/value.h"

#include <cstddef>
#include <utility>

namespace ink::ir
{
  class BasicBlock;
  class InstructionBuilder;

  namespace detail
  {
    // No default layout: every registered instruction must explicitly provide its operand implementation.
    template <typename InstructionType>
    struct InstructionOperands;
  } // namespace detail

  // Common base for IR instructions; the borrowed type value must outlive the concrete instruction.
  class Instruction : public Value
  {
    public:
      constexpr InstructionKind getKind() const noexcept
      {
        return Kind;
      }

      constexpr bool isTerminator() const noexcept
      {
        return isTerminatorKind(Kind);
      }

      static constexpr bool isTerminatorKind(InstructionKind Kind) noexcept
      {
        return Kind == InstructionKind::Return || Kind == InstructionKind::Branch || Kind == InstructionKind::CondBranch;
      }

      std::string_view getName() const noexcept
      {
        return instructionKindName(Kind);
      }

      // Enumerates explicit data operands only; result types, branch targets, and phi predecessor blocks are excluded.
      std::size_t operandCount() const noexcept;

      // Index must be less than operandCount(); the returned value remains borrowed and read-only.
      const Value &operand(std::size_t Index) const noexcept;

      // Rebinds one existing operand; an invalid index or different type identity returns false without modifying the instruction.
      // Result types and operand counts never change. Callers must preserve borrowed lifetimes, function ownership, SSA dominance, and phi edge consistency.
      [[nodiscard]] bool setOperand(std::size_t Index, ValueRef Replacement) noexcept;

      auto *parent(this auto &&Self) noexcept
      {
        return Self.Parent ? &std::forward_like<decltype(Self) &>(*Self.Parent) : nullptr;
      }

    protected:
      constexpr Instruction(InstructionKind Kind, ValueRef ValueType) noexcept
          : Value(ValueType, ValueKind::Instruction),
            Kind(Kind)
      {
      }

      // Instructions are destroyed through their concrete class, never through Instruction.
      ~Instruction() = default;

    private:
      friend class InstructionBuilder;

      InstructionKind Kind;
      BasicBlock *Parent = nullptr;
  };
} // namespace ink::ir
