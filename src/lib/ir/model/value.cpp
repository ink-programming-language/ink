#include "ink/ir/model/value.h"

namespace ink::ir
{
  Value::~Value() = default;

  const char *valueKindName(ValueKind Kind) noexcept
  {
    switch (Kind)
    {
#define INK_IR_VALUE(Name) \
  case ValueKind::Name:    \
    return #Name;
#include "ink/ir/ir.def"
    }
    return "Unknown";
  }
} // namespace ink::ir
