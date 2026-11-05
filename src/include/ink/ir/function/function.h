#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/function/basic_block.h"
#include "ink/ir/type/function_type.h"
#include "ink/ir/value/value.h"

#include <cstddef>
#include <list>
#include <ranges>

namespace ink::ir
{
  // Stores an interned name and owns its basic blocks; the signature is borrowed and must outlive this function.
  // Appending a block preserves all existing block addresses and iterators.
  class Function final : public Value
  {
    public:
      explicit Function(const FunctionType &Signature, core::Name Name = core::NAME_None) noexcept
          : Value(Signature),
            Name(Name)
      {
      }

      Function(const FunctionType &&, core::Name = core::NAME_None) = delete;

      core::Name name() const noexcept
      {
        return Name;
      }

      const FunctionType &signature() const noexcept
      {
        return static_cast<const FunctionType &>(getType());
      }

      BasicBlock &createBasicBlock(core::Name Name = core::NAME_None)
      {
        BasicBlock &Block = Blocks.emplace_back(Name);
        Block.Parent = this;
        return Block;
      }

      auto basicBlocks(this auto &&Self) noexcept
      {
        return std::ranges::subrange(Self.Blocks.begin(), Self.Blocks.end());
      }

      std::size_t basicBlockCount() const noexcept
      {
        return Blocks.size();
      }

    private:
      core::Name Name;
      std::list<BasicBlock> Blocks;
  };
} // namespace ink::ir
