#pragma once

#include "ink/core/name/name.h"
#include "ink/ir/enums.h"
#include "ink/ir/function/function.h"

#include <memory>

namespace ink::ir
{
  // Owns its function; the borrowed signature must remain alive whenever the function is used.
  // Moving a method preserves function and body addresses; moved-from methods may only be destroyed or assigned.
  class Method
  {
    public:
      explicit Method(const FunctionType &Signature, core::Name Name = core::NAME_None, MemberKind Kind = MemberKind::Instance, SymbolVisibility Visibility = SymbolVisibility::Private, NativeLinkage Linkage = NativeLinkage::None)
          : MethodFunction(std::make_unique<Function>(Signature, Name, Visibility, Linkage)),
            Kind(Kind)
      {
      }

      Method(const FunctionType &&, core::Name = core::NAME_None, MemberKind = MemberKind::Instance, SymbolVisibility = SymbolVisibility::Private, NativeLinkage = NativeLinkage::None) = delete;

      Method(const Method &) = delete;
      Method &operator=(const Method &) = delete;
      Method(Method &&) noexcept = default;
      Method &operator=(Method &&) noexcept = default;

      core::Name name() const noexcept
      {
        return MethodFunction->name();
      }

      Function &function() noexcept
      {
        return *MethodFunction;
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
      std::unique_ptr<Function> MethodFunction = nullptr;
      MemberKind Kind = MemberKind::Instance;
  };
} // namespace ink::ir
