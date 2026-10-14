#ifndef INK_IR_TYPE_BUILTIN_TYPE_VARIADIC_TYPE_H
#define INK_IR_TYPE_BUILTIN_TYPE_VARIADIC_TYPE_H

#include "ink/ir/type/type.h"

namespace ink::ir
{
  class TypePool;

  // Standalone ellipsis marker without an element type or runtime storage layout.
  class VariadicType final : public Type
  {
    public:
      // Size and alignment are placeholders; consumers must check hasLayout().
      bool hasLayout() const noexcept override;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypePool;

      VariadicType() noexcept;
  };
} // namespace ink::ir

#endif
