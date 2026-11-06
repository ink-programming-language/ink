#include "ink/ir/module/global_variable.h"

namespace ink::ir
{
  GlobalVariable::GlobalVariable(const PointerType &ResultType, core::Name Name, SymbolVisibility Visibility, NativeLinkage Linkage) noexcept
      : Value(ValueRef{ResultType}),
        Name(Name),
        Visibility(Visibility),
        Linkage(Linkage)
  {
  }
} // namespace ink::ir
