#ifndef INK_IR_TYPE_STRUCTURE_TYPE_REFERENCE_TYPE_H
#define INK_IR_TYPE_STRUCTURE_TYPE_REFERENCE_TYPE_H

#include "ink/target/target_context.h"
#include "ink/ir/type/type.h"

namespace ink::ir
{
  class TypePool;

  class ReferenceType final : public Type
  {
    public:
      const Type *referencedType() const noexcept;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      std::string toString() const override;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypePool;

      void appendToString(ValueStringBuilder &Builder) const override;

      // ReferencedType is a non-null borrowed pointer and must outlive the reference type.
      ReferenceType(const MetaType &Meta, const target::TargetContext &Target, const Type *ReferencedType, bool Mutable = true) noexcept;

      const Type *ReferencedType = nullptr;
      std::uint64_t PointerByteWidth = 0;
  };
} // namespace ink::ir

#endif
