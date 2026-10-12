#ifndef INK_IR_TYPE_NULL_POINTER_TYPE_H
#define INK_IR_TYPE_NULL_POINTER_TYPE_H

#include "ink/ir/type/type.h"

namespace ink::ir
{
  class TypeContext;

  // The type of the context's unique null pointer literal, independent of any pointee type.
  class NullPointerType final : public Type
  {
    public:
      const TypeContext &typeContext() const noexcept;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypeContext;

      explicit NullPointerType(const TypeContext &Context) noexcept;

      const TypeContext &Context;
      std::uint64_t PointerByteWidth = 0;
  };
} // namespace ink::ir

#endif
