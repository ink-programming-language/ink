#pragma once

#include "ink/ir/instruction/instruction_kind.h"
#include "ink/ir/value/value.h"

#include <utility>

namespace ink::ir
{
  class BasicBlock;
  class InstructionBuilder;

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
