#include "ink/ir/function/function_parameter.h"

#include "ink/ir/function/function.h"

#include <cassert>

namespace ink::ir
{
  const Function &FunctionParameter::function() const noexcept
  {
    assert(Function::classof(outer()) && "function parameters always belong to a function");
    return *static_cast<const Function *>(outer());
  }
} // namespace ink::ir
