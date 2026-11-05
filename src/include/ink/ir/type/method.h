#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/function/function.h"

namespace ink::ir
{
  // Borrows the function and forwards its name handle; the function must remain alive whenever the method is accessed.
  class Method
  {
    public:
      explicit Method(const Function &FunctionValue) noexcept
          : MethodFunction(&FunctionValue)
      {
      }

      Method(const Function &&) = delete;

      core::Name name() const noexcept
      {
        return MethodFunction->name();
      }

      const Function &function() const noexcept
      {
        return *MethodFunction;
      }

    private:
      const Function *MethodFunction;
  };
} // namespace ink::ir
