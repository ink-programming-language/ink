#include "execution_frame.h"

namespace ink::execution
{
  ExecutionFrame::ExecutionFrame(const std::vector<RuntimeValueRef> &Arguments) : Values(Arguments)
  {
  }

  RuntimeValueRef ExecutionFrame::find(ir::ValueId Id) const noexcept
  {
    return Id.valid() && Id.value() < Values.size() ? Values[Id.value()] : nullptr;
  }

  bool ExecutionFrame::define(ir::ValueId Id, RuntimeValueRef Value)
  {
    if (!Id.valid() || Id.value() != Values.size() || Value == nullptr)
    {
      return false;
    }
    Values.push_back(Value);
    return true;
  }
} // namespace ink::execution
