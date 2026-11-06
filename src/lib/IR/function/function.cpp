#include "ink/ir/function/function.h"

#include "ink/core/support/core_define.h"

namespace ink::ir
{
  void Function::initializeArguments() noexcept
  {
    checkMsgf(FunctionType::classof(&getType()), "Function requires a FunctionType signature");
    const auto Parameters = signature().parameters();
    Arguments.reserve(Parameters.size());
    for (std::size_t Index = 0; Index < Parameters.size(); ++Index)
    {
      Arguments.push_back(std::unique_ptr<Argument>(new Argument(Parameters[Index].type(), *this, Index)));
    }
  }

  Argument &Function::argumentAt(std::size_t Index) const noexcept
  {
    checkMsgf(Index < Arguments.size(), "Function argument index is out of range");
    return *Arguments[Index];
  }
} // namespace ink::ir
