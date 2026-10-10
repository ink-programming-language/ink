#ifndef INK_IR_TYPE_REFERENCE_TYPE_H
#define INK_IR_TYPE_REFERENCE_TYPE_H

#include "ink/core/target_context.h"
#include "ink/ir/type/type.h"

namespace ink::ir
{
  class ReferenceType final : public Type
  {
    public:
      // ReferencedType is a non-null borrowed pointer and must outlive the reference type.
      ReferenceType(const core::TargetContext &Target, const Type *ReferencedType) noexcept;

      const Type *referencedType() const noexcept;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      static bool classof(const Value *Value) noexcept;

    private:
      const Type *ReferencedType = nullptr;
      std::uint64_t PointerByteWidth = 0;
  };
} // namespace ink::ir

#endif
