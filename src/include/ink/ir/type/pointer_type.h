#ifndef INK_IR_TYPE_POINTER_TYPE_H
#define INK_IR_TYPE_POINTER_TYPE_H

#include "ink/target/target_context.h"
#include "ink/ir/type/type.h"

namespace ink::ir
{
  class TypeContext;

  class PointerType final : public Type
  {
    public:
      const Type *pointeeType() const noexcept;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypeContext;

      // PointeeType is a non-null borrowed pointer and must outlive the pointer type.
      PointerType(const target::TargetContext &Target, const Type *PointeeType) noexcept;

      const Type *PointeeType = nullptr;
      std::uint64_t PointerByteWidth = 0;
  };
} // namespace ink::ir

#endif
