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
    if (!Id.valid() || Value == nullptr)
    {
      return false;
    }
    if (Id.value() >= Values.size())
    {
      Values.resize(Id.value() + 1, nullptr);
    }
    Values[Id.value()] = Value;
    return true;
  }
} // namespace ink::execution
