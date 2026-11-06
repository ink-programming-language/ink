#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/enums.h"
#include "ink/ir/function/basic_block.h"
#include "ink/ir/type/function_type.h"
#include "ink/ir/value/value.h"

#include <cstddef>
#include <list>
#include <ranges>
#include <utility>

namespace ink::ir
{
  class Module;

  // Borrows a FunctionType value and owns its named basic blocks; the signature must outlive this function.
  // Appending a block preserves all existing block addresses and iterators.
  class Function final : public Value
  {
    public:
      explicit Function(ValueRef Signature, core::Name Name = core::NAME_None, SymbolVisibility Visibility = SymbolVisibility::Private, NativeLinkage Linkage = NativeLinkage::None) noexcept
          : Value(Signature),
            Name(Name),
            Visibility(Visibility),
            Linkage(Linkage)
      {
      }

      core::Name name() const noexcept
      {
        return Name;
      }

      auto *parent(this auto &&Self) noexcept
      {
        return Self.Parent ? &std::forward_like<decltype(Self) &>(*Self.Parent) : nullptr;
      }

      SymbolVisibility visibility() const noexcept
      {
        return Visibility;
      }

      void setVisibility(SymbolVisibility Value) noexcept
      {
        Visibility = Value;
      }

      NativeLinkage nativeLinkage() const noexcept
      {
        return Linkage;
      }

      void setNativeLinkage(NativeLinkage Value) noexcept
      {
        Linkage = Value;
      }

      const FunctionType &signature() const noexcept
      {
        return static_cast<const FunctionType &>(getType());
      }

      ABIKind abi() const noexcept
      {
        return signature().abi();
      }

      CallingConvention callingConvention() const noexcept
      {
        return signature().callingConvention();
      }

      BasicBlock &createBasicBlock(core::Name BlockName = core::NAME_None)
      {
        BasicBlock &Block = Blocks.emplace_back(BlockName);
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
      friend class Module;

      core::Name Name = core::NAME_None;
      SymbolVisibility Visibility = SymbolVisibility::Private;
      NativeLinkage Linkage = NativeLinkage::None;
      Module *Parent = nullptr;
      std::list<BasicBlock> Blocks = {};
  };
} // namespace ink::ir
