#pragma once

#include "ink/ir/instruction/instruction_kind.h"
#include "ink/ir/value/value.h"

#include <utility>

namespace ink::ir
{
  class BasicBlock;
  class InstructionBuilder;

  // Common base for IR instructions; the borrowed type must outlive the concrete instruction.
  class Instruction : public Value
  {
    public:
      constexpr InstructionKind getKind() const noexcept
      {
        return Kind;
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
      constexpr Instruction(InstructionKind Kind, const Type &ValueType) noexcept
          : Value(ValueType),
            Kind(Kind)
      {
      }

      Instruction(InstructionKind, const Type &&) = delete;

      // Instructions are destroyed through their concrete class, never through Instruction.
      ~Instruction() = default;

    private:
      friend class InstructionBuilder;

      InstructionKind Kind;
      BasicBlock *Parent = nullptr;
  };
} // namespace ink::ir
