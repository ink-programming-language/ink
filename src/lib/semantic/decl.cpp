#include "ink/semantic/decl.h"
#include "ink/semantic/type.h"

namespace ink::semantic
{
  bool VarDecl::setType(const Type &ResolvedType) noexcept
  {
    if (&ResolvedType.context() != &context() || (ValueType && ValueType != &ResolvedType) || (Initializer && &Initializer->type() != &ResolvedType))
    {
      return false;
    }
    ValueType = &ResolvedType;
    return true;
  }

  bool VarDecl::setInitializer(const Value &InitializerValue) noexcept
  {
    if (&InitializerValue.type().context() != &context() || (Initializer && Initializer != &InitializerValue) || (ValueType && ValueType != &InitializerValue.type()))
    {
      return false;
    }
    Initializer = &InitializerValue;
    return true;
  }
} // namespace ink::semantic
