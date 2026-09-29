#include "ink/ir/decl/decl.h"

#include "ink/ir/context.h"

namespace ink::ir
{
  Decl::~Decl()
  {
    Owner.context().notifyDestroyed(*this);
  }
} // namespace ink::ir
