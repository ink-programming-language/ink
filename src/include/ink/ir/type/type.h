#ifndef INK_IR_TYPE_TYPE_H
#define INK_IR_TYPE_TYPE_H

#include "ink/ir/type/value.h"

namespace ink::ir
{
  class MetaType;

  // Common base for IR types; size and alignment remain type-specific.
  class Type : public Value
  {
    public:
      ~Type() noexcept override;

      // Type descriptors are compile-time values whose type is their context's unique metatype.
      const Type *type() const noexcept override;

      // False for type markers or templates awaiting instantiation; size/alignment then carry no runtime layout.
      virtual bool hasLayout() const noexcept;

      // The represented value's storage size and ABI alignment, both in bytes.
      // These describe runtime storage only when hasLayout() is true.
      // Alignment must be a nonzero power of two, including for zero-sized values.
      virtual std::uint64_t size() const noexcept = 0;
      virtual std::uint64_t alignment() const noexcept = 0;
      static bool classof(const Value *Value) noexcept;

    protected:
      // Borrows Meta, which must outlive this descriptor.
      Type(ValueKind Kind, const MetaType &Meta) noexcept;

    private:
      const MetaType &Meta;
  };
} // namespace ink::ir

#endif
