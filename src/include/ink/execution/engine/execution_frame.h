#ifndef INK_EXECUTION_ENGINE_EXECUTION_FRAME_H
#define INK_EXECUTION_ENGINE_EXECUTION_FRAME_H

#include "ink/execution/value/execution_value.h"

#include <unordered_map>
#include <vector>

namespace ink::execution
{
  class ExecutionEngine;

  enum class ExecutionFrameKind
  {
    Module,
    Analysis,
    Call,
    Block,
  };

  // Parent is the definition environment, not the dynamic caller's environment.
  // Frame identities remain stable until their engine is destroyed.
  class ExecutionFrame final
  {
    public:
      ~ExecutionFrame();
      ExecutionFrame(const ExecutionFrame &) = delete;
      ExecutionFrame &operator=(const ExecutionFrame &) = delete;
      ExecutionFrame(ExecutionFrame &&) = delete;
      ExecutionFrame &operator=(ExecutionFrame &&) = delete;

      ExecutionFrameKind kind() const noexcept
      {
        return Kind;
      }

      ExecutionFrame *parent() const noexcept
      {
        return Parent;
      }

      bool active() const noexcept
      {
        return Active;
      }

    private:
      struct Event
      {
          bool Running = true;
          ExecutionResult Result;
      };

      ExecutionFrame(ExecutionEngine &Owner, ExecutionFrameKind Kind, ExecutionFrame *Parent);

      ExecutionEngine *Owner;
      ExecutionFrameKind Kind;
      ExecutionFrame *Parent;
      bool Active = true;
      std::unordered_map<const void *, ExecutionPlace> Bindings;
      std::unordered_map<const void *, Event> Events;
      // SSA values are snapshots belonging to this activation, never cached on IR nodes.
      std::unordered_map<const ir::Value *, ExecutionValueRef> Values;
      std::vector<ExecutionFrame *> Children;
      std::vector<ExecutionStorageRef> Storage;

      friend class ExecutionEngine;
  };
} // namespace ink::execution

#endif
