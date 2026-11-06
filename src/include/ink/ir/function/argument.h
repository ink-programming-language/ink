#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/function/parameter.h"
#include "ink/ir/value/value.h"

#include <cstddef>
#include <utility>

namespace ink::ir
{
  class Function;

  // A function owns each argument at a stable address; its borrowed type value must remain alive during use.
  // Arguments are distinct values even when their functions share a signature or their names match.
  class Argument final : public Value
  {
    public:
      core::Name name() const noexcept
      {
        return Name;
      }

      void setName(core::Name Value) noexcept
      {
        Name = Value;
      }

      std::size_t index() const noexcept
      {
        return Index;
      }

      Parameter::Kind kind() const noexcept;

      auto *parent(this auto &&Self) noexcept
      {
        return &std::forward_like<decltype(Self) &>(*Self.Parent);
      }

    private:
      friend class Function;

      Argument(ValueRef ArgumentType, Function &Parent, std::size_t Index) noexcept
          : Value(ArgumentType),
            Parent(&Parent),
            Index(Index)
      {
      }

      Function *Parent;
      std::size_t Index;
      core::Name Name = core::NAME_None;
  };
} // namespace ink::ir
