#ifndef INK_IR_TYPE_STRUCTURE_TYPE_POINTER_TYPE_H
#define INK_IR_TYPE_STRUCTURE_TYPE_POINTER_TYPE_H

#include "ink/target/target_context.h"
#include "ink/ir/type/type.h"

namespace ink::ir
{
  class TypePool;

  class PointerType final : public Type
  {
    public:
      // For *void this returns the void marker, not the unknown object's storage layout.
      const Type *pointeeType() const noexcept;
      // *void carries an unknown object address; it cannot be dereferenced or indexed.
      // Other pointers, including **void, retain a known pointee type.
      bool isOpaque() const noexcept;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      std::string toString() const override;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypePool;

      void appendToString(ValueStringBuilder &Builder) const override;

      // PointeeType is a non-null borrowed pointer and must outlive the pointer type.
      PointerType(const MetaType &Meta, const target::TargetContext &Target, const Type *PointeeType, bool Mutable = true) noexcept;

      const Type *PointeeType = nullptr;
      std::uint64_t PointerByteWidth = 0;
  };
} // namespace ink::ir

#endif
