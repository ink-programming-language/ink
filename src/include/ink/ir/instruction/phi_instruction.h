#pragma once

#include "ink/ir/instruction/instruction.h"

#include <cstddef>
#include <span>
#include <vector>

namespace ink::ir
{
  class BasicBlock;

  // Borrows its result type and incoming value/block pairs; all borrowed objects must outlive this instruction.
  // Incoming pairs may be staged, including loop backedges; the caller is responsible for their final validity.
  class PhiInstruction final : public Instruction
  {
    public:
      struct Incoming
      {
        public:
          const Value *IncomingValue;
          BasicBlock *Predecessor;

          const Value &getValue() const noexcept
          {
            return *IncomingValue;
          }

          auto &getBlock(this auto &&Self) noexcept
          {
            return std::forward_like<decltype(Self) &>(*Self.Predecessor);
          }
      };

      static constexpr InstructionKind Kind = InstructionKind::Phi;

      constexpr explicit PhiInstruction(ValueRef ResultType) noexcept
          : Instruction(Kind, ResultType)
      {
      }

      static constexpr bool classof(const Instruction *Value) noexcept
      {
        return Value && Value->getKind() == Kind;
      }

      void addIncoming(ValueRef IncomingValue, BasicBlock &Predecessor);

      std::size_t incomingCount() const noexcept
      {
        return IncomingValues.size();
      }

      const Value &incomingValue(std::size_t Index) const noexcept;

      auto &incomingBlock(this auto &&Self, std::size_t Index) noexcept
      {
        return std::forward_like<decltype(Self) &>(Self.incomingBlockAt(Index));
      }

      // Appending an incoming pair may invalidate the returned view, but never the borrowed values or blocks.
      std::span<const Incoming> incoming() const noexcept
      {
        return IncomingValues;
      }

    private:
      friend struct detail::InstructionOperands<PhiInstruction>;

      BasicBlock &incomingBlockAt(std::size_t Index) const noexcept;

      std::vector<Incoming> IncomingValues = {};
  };
} // namespace ink::ir
