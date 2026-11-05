#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/instruction/instruction.h"

#include <cstddef>
#include <iterator>
#include <list>
#include <memory>
#include <ranges>
#include <type_traits>
#include <utility>

namespace ink::ir
{
  class Function;
  class InstructionBuilder;

  // A block is detached until a Function creates and owns it; its address is its stable identity.
  // Owns its instructions; insertion preserves all existing instruction addresses and iterators.
  class BasicBlock final
  {
    private:
      using InstructionOwner = std::unique_ptr<Instruction, void (*)(Instruction *) noexcept>;
      using InstructionStorage = std::list<InstructionOwner>;

      template <bool IsConst>
      class InstructionIterator
      {
        public:
          using value_type = Instruction;
          using difference_type = std::ptrdiff_t;
          using pointer = std::conditional_t<IsConst, const Instruction *, Instruction *>;
          using reference = std::conditional_t<IsConst, const Instruction &, Instruction &>;
          using iterator_category = std::bidirectional_iterator_tag;
          using iterator_concept = std::bidirectional_iterator_tag;

          InstructionIterator() noexcept = default;

          template <bool OtherIsConst>
          InstructionIterator(const InstructionIterator<OtherIsConst> &Other) noexcept requires (IsConst && !OtherIsConst)
              : Position(Other.Position)
          {
          }

          reference operator*() const noexcept
          {
            return **Position;
          }

          pointer operator->() const noexcept
          {
            return Position->get();
          }

          InstructionIterator &operator++() noexcept
          {
            ++Position;
            return *this;
          }

          InstructionIterator operator++(int) noexcept
          {
            InstructionIterator Previous = *this;
            ++*this;
            return Previous;
          }

          InstructionIterator &operator--() noexcept
          {
            --Position;
            return *this;
          }

          InstructionIterator operator--(int) noexcept
          {
            InstructionIterator Previous = *this;
            --*this;
            return Previous;
          }

          friend bool operator==(const InstructionIterator &, const InstructionIterator &) noexcept = default;

        private:
          friend class BasicBlock;
          template <bool>
          friend class InstructionIterator;

          using StorageIterator = std::conditional_t<IsConst, InstructionStorage::const_iterator, InstructionStorage::iterator>;

          explicit InstructionIterator(StorageIterator Position) noexcept
              : Position(Position)
          {
          }

          StorageIterator Position;
      };

    public:
      using Iterator = InstructionIterator<false>;
      using ConstIterator = InstructionIterator<true>;

      explicit BasicBlock(core::Name Name = core::NAME_None) noexcept
          : Name(Name)
      {
      }

      BasicBlock(const BasicBlock &) = delete;
      BasicBlock &operator=(const BasicBlock &) = delete;
      BasicBlock(BasicBlock &&) = delete;
      BasicBlock &operator=(BasicBlock &&) = delete;

      core::Name name() const noexcept
      {
        return Name;
      }

      auto *parent(this auto &&Self) noexcept
      {
        return Self.Parent ? &std::forward_like<decltype(Self) &>(*Self.Parent) : nullptr;
      }

      std::size_t instructionCount() const noexcept
      {
        return Instructions.size();
      }

      auto instructions(this auto &&Self) noexcept
      {
        if constexpr (std::is_const_v<std::remove_reference_t<decltype(Self)>>)
        {
          return std::ranges::subrange(ConstIterator(Self.Instructions.begin()), ConstIterator(Self.Instructions.end()));
        }
        else
        {
          return std::ranges::subrange(Iterator(Self.Instructions.begin()), Iterator(Self.Instructions.end()));
        }
      }

    private:
      friend class Function;
      friend class InstructionBuilder;

      core::Name Name;
      Function *Parent = nullptr;
      InstructionStorage Instructions;
  };
} // namespace ink::ir
