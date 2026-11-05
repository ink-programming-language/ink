#include "ink/ir/value/value.h"

#include "ink/ir/type/meta_type.h"

namespace ink::ir
{
  bool Value::isTypeValue() const noexcept
  {
    return &getType() == &MetaType::instance();
  }
} // namespace ink::ir
