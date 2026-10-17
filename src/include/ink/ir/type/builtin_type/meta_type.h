#ifndef INK_IR_TYPE_BUILTIN_TYPE_META_TYPE_H
#define INK_IR_TYPE_BUILTIN_TYPE_META_TYPE_H

#include "ink/ir/type/type.h"

namespace ink::ir
{
  class TypePool;

  // The compile-time type of type values, including itself; it has no runtime storage layout.
  class MetaType final : public Type
  {
    public:
      bool hasLayout() const noexcept override;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypePool;

      MetaType() noexcept;
  };
} // namespace ink::ir

#endif
