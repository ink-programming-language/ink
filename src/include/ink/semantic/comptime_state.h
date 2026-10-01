#ifndef INK_SEMANTIC_COMPTIME_STATE_H
#define INK_SEMANTIC_COMPTIME_STATE_H

#include "ink/core/source_id.h"
#include "ink/core/source_range.h"
#include "ink/execution/engine/execution_engine.h"
#include "ink/ir/instruction/alloca_instruction.h"
#include "ink/ir/lifetime_observer.h"

#include <memory>
#include <unordered_map>
#include <vector>

namespace ink::ir
{
  class Function;
} // namespace ink::ir

namespace ink::semantic
{
  // Semantic identities remain separate from mutable execution storage. Detached
  // addresses are binding descriptors only; they are never emitted as runtime IR.
  class ComptimeState final
  {
    public:
      struct Variable
      {
          bool Comptime = false;
          bool Constant = false;
          bool Initialized = false;
          const ir::Function *Function = nullptr;
      };

      struct FunctionDefinition
      {
          bool Comptime = false;
          core::SourceId Source;
          core::SourceRange NameRange;
      };

      explicit ComptimeState(ir::IRContext &Context)
          : Engine(Context),
            Observer(std::make_unique<BindingObserver>(Context, *this))
      {
      }

      std::vector<std::unique_ptr<ir::AllocaInstruction>> Bindings;
      execution::ExecutionEngine Engine;
      std::unordered_map<const ir::Value *, Variable> Variables;
      std::unordered_map<const ir::Value *, FunctionDefinition> Functions;

    private:
      class BindingObserver final : public ir::LifetimeObserver
      {
        public:
          BindingObserver(ir::IRContext &Context, ComptimeState &State)
              : ir::LifetimeObserver(Context),
                State(State)
          {
          }

        private:
          void valueDestroyed(ir::Value &Target) noexcept override
          {
            State.Variables.erase(&Target);
            State.Functions.erase(&Target);
          }

          void declDestroyed(ir::Decl &) noexcept override
          {
          }

          ComptimeState &State;
      };

      // Unregister before destroying binding descriptors or their indexes.
      std::unique_ptr<BindingObserver> Observer;
  };
} // namespace ink::semantic

#endif
