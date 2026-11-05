#pragma once

#include "ink/core/name/name.h"

#include <utility>

namespace ink::ir
{
  class Function;

  // A block is detached until a Function creates and owns it; its address is its stable identity.
  class BasicBlock final
  {
    public:
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

    private:
      friend class Function;

      core::Name Name;
      Function *Parent = nullptr;
  };
} // namespace ink::ir
