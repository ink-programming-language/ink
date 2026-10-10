#ifndef INK_IR_TYPE_POINTER_TYPE_H
#define INK_IR_TYPE_POINTER_TYPE_H

#include "ink/core/target_context.h"
#include "ink/ir/type/type.h"

namespace ink::ir
{
  class PointerType final : public Type
  {
    public:
      // PointeeType is a non-null borrowed pointer and must outlive the pointer type.
      PointerType(const core::TargetContext &Target, const Type *PointeeType) noexcept;

      const Type *pointeeType() const noexcept;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      static bool classof(const Value *Value) noexcept;

    private:
      const Type *PointeeType = nullptr;
      std::uint64_t PointerByteWidth = 0;
  };
} // namespace ink::ir

#endif
