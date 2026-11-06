#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/enums.h"
#include "ink/ir/function/function.h"

namespace ink::ir
{
  // Borrows the function and forwards its name handle; the function must remain alive whenever the method is accessed.
  class Method
  {
    public:
      explicit Method(const Function &FunctionValue, MemberKind Kind = MemberKind::Instance) noexcept
          : MethodFunction(&FunctionValue),
            Kind(Kind)
      {
      }

      Method(const Function &&, MemberKind = MemberKind::Instance) = delete;

      core::Name name() const noexcept
      {
        return MethodFunction->name();
      }

      const Function &function() const noexcept
      {
        return *MethodFunction;
      }

      MemberKind kind() const noexcept
      {
        return Kind;
      }

      bool isStatic() const noexcept
      {
        return Kind == MemberKind::Static;
      }

    private:
      const Function *MethodFunction = nullptr;
      MemberKind Kind = MemberKind::Instance;
  };
} // namespace ink::ir
