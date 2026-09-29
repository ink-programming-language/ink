#include "ink/semantic/model/value.h"

#include "ink/semantic/context.h"

namespace ink::semantic
{
  Value::~Value()
  {
    Context.Scopes->forget(*this);
  }
} // namespace ink::semantic
