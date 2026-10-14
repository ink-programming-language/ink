#ifndef INK_IR_TYPE_BUILTIN_TYPE_NULL_POINTER_TYPE_H
#define INK_IR_TYPE_BUILTIN_TYPE_NULL_POINTER_TYPE_H

#include "ink/ir/type/type.h"

namespace ink::ir
{
  class Module;
  class TypePool;

  // The type of the module's unique null pointer literal, independent of any pointee type.
  class NullPointerType final : public Type
  {
    public:
      const Module &module() const noexcept;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypePool;

      explicit NullPointerType(const Module &Owner) noexcept;

      const Module &Owner;
      std::uint64_t PointerByteWidth = 0;
  };
} // namespace ink::ir

#endif
