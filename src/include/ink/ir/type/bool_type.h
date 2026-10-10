#ifndef INK_IR_TYPE_BOOL_TYPE_H
#define INK_IR_TYPE_BOOL_TYPE_H

#include "ink/ir/type/type.h"

namespace ink::ir
{
  class BoolType final : public Type
  {
    public:
      BoolType() noexcept;

      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      static bool classof(const Value *Value) noexcept;
  };
} // namespace ink::ir

#endif
