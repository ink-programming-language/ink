#include "ink/ir/value.h"

#include "ink/ir/context.h"

namespace ink::ir
{
  Value::~Value()
  {
    Context.notifyDestroyed(*this);
  }
} // namespace ink::ir
