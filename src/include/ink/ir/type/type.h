#ifndef INK_IR_TYPE_TYPE_H
#define INK_IR_TYPE_TYPE_H

#include "ink/ir/type/value.h"

namespace ink::ir
{
  // Common base for concrete IR types; size and alignment remain type-specific.
  class Type : public Value
  {
    public:
      ~Type() noexcept override;

    protected:
      explicit Type(ValueKind Kind) noexcept;
  };
} // namespace ink::ir

#endif
