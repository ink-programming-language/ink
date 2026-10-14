#ifndef INK_IR_TYPE_BUILTIN_TYPE_VOID_TYPE_H
#define INK_IR_TYPE_BUILTIN_TYPE_VOID_TYPE_H

#include "ink/ir/type/type.h"

namespace ink::ir
{
  class TypePool;

  // Unit type with one value and a valid zero-byte runtime layout.
  class VoidType final : public Type
  {
    public:
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypePool;

      VoidType() noexcept;
  };
} // namespace ink::ir

#endif
