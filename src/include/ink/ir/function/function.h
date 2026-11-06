#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/enums.h"
#include "ink/ir/function/argument.h"
#include "ink/ir/function/basic_block.h"
#include "ink/ir/type/function_type.h"
#include "ink/ir/value/value.h"

#include <cstddef>
#include <list>
#include <memory>
#include <ranges>
#include <utility>
#include <vector>

namespace ink::ir
{
  class Module;

  // Borrows a FunctionType value and owns distinct arguments and named basic blocks; the signature must outlive this function.
  // Argument types borrow the exact signature type values, without rebinding generic dependencies.
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
        initializeArguments();
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

      auto &argument(this auto &&Self, std::size_t Index) noexcept
      {
        return std::forward_like<decltype(Self) &>(Self.argumentAt(Index));
      }

      auto arguments(this auto &&Self) noexcept
      {
        return Self.Arguments | std::views::transform([](const std::unique_ptr<Argument> &Owned) -> decltype(auto)
        {
          return std::forward_like<decltype(Self) &>(*Owned);
        });
      }

      std::size_t argumentCount() const noexcept
      {
        return Arguments.size();
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

      void initializeArguments() noexcept;
      Argument &argumentAt(std::size_t Index) const noexcept;

      core::Name Name = core::NAME_None;
      SymbolVisibility Visibility = SymbolVisibility::Private;
      NativeLinkage Linkage = NativeLinkage::None;
      Module *Parent = nullptr;
      std::vector<std::unique_ptr<Argument>> Arguments = {};
      std::list<BasicBlock> Blocks = {};
  };
} // namespace ink::ir
