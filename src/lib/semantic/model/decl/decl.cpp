#include "ink/semantic/model/decl/decl.h"

#include "ink/semantic/context.h"

namespace ink::semantic
{
  Decl::~Decl()
  {
    Owner.context().Scopes->forget(*this);
  }
} // namespace ink::semantic
