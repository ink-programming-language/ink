#include "execution_frame.h"

#include <utility>

namespace ink::execution
{
  ExecutionFrame::ExecutionFrame(const std::vector<RuntimeValue> &Arguments)
  {
    for (std::size_t ArgumentIndex = 0; ArgumentIndex < Arguments.size(); ++ArgumentIndex)
    {
      Values.emplace(ArgumentIndex, Arguments[ArgumentIndex]);
    }
  }

  const RuntimeValue *ExecutionFrame::find(ir::ValueId Id) const noexcept
  {
    if (!Id.valid())
    {
      return nullptr;
    }
    const auto Stored = Values.find(Id.value());
    return Stored == Values.end() ? nullptr : &Stored->second;
  }

  bool ExecutionFrame::define(ir::ValueId Id, RuntimeValue Value)
  {
    return Id.valid() && Values.emplace(Id.value(), std::move(Value)).second;
  }
} // namespace ink::execution
