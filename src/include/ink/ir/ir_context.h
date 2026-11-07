#pragma once

#include "ink/ir/constant/constant_pool.h"
#include "ink/ir/instruction/instruction_builder.h"
#include "ink/ir/type/type_pool.h"

namespace ink::ir
{
  // Owns the pools and default builder for one IR session; modules may share this context.
  // The context must remain alive while graphs borrowing its types or constants are used.
  // Modules and blocks retain instruction ownership; clear or replace the builder's insertion point before using it after its block is destroyed.
  // External type values keep their existing borrowed lifetime requirements. Concurrent operations require external synchronization.
  class IRContext final
  {
    public:
      IRContext();
      IRContext(const IRContext &) = delete;
      IRContext &operator=(const IRContext &) = delete;
      IRContext(IRContext &&) = delete;
      IRContext &operator=(IRContext &&) = delete;

      auto &types(this auto &&Self) noexcept
      {
        return Self.Types;
      }

      auto &constants(this auto &&Self) noexcept
      {
        return Self.Constants;
      }

      auto &builder(this auto &&Self) noexcept
      {
        return Self.Builder;
      }

    private:
      // Destroy the builder before constants, and constants before their canonical types.
      TypePool Types;
      ConstantPool Constants;
      InstructionBuilder Builder;
  };
} // namespace ink::ir
