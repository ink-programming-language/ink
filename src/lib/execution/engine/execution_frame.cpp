#include "ink/execution/engine/execution_frame.h"

namespace ink::execution
{
  ExecutionFrame::ExecutionFrame(ExecutionEngine &Owner, ExecutionFrameKind Kind, ExecutionFrame *Parent)
      : Owner(&Owner),
        Kind(Kind),
        Parent(Parent)
  {
  }

  ExecutionFrame::~ExecutionFrame() = default;
} // namespace ink::execution
