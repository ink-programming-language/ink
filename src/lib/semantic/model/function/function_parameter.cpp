#include "ink/semantic/model/function/function_parameter.h"

#include "ink/semantic/model/function/function.h"

#include <cassert>

namespace ink::semantic
{
  const Function &FunctionParameter::function() const noexcept
  {
    assert(Function::classof(outer()) && "function parameters always belong to a function");
    return *static_cast<const Function *>(outer());
  }
} // namespace ink::semantic
