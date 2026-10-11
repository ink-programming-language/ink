#ifndef INK_IR_TYPE_STRING_TYPE_H
#define INK_IR_TYPE_STRING_TYPE_H

#include "ink/target/target_context.h"
#include "ink/ir/type/type.h"

namespace ink::ir
{
  class TypeContext;

  // A read-only view of valid UTF-8, represented by a data pointer and a target-sized unsigned byte length.
  // The view's storage layout is independent of its text length; it does not own the referenced bytes.
  class StringType final : public Type
  {
    public:
      std::uint64_t maximumByteLength() const noexcept;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypeContext;

      explicit StringType(const target::TargetContext &Target) noexcept;

      std::uint64_t PointerByteWidth = 0;
      std::uint64_t MaximumByteLength = 0;
  };
} // namespace ink::ir

#endif
