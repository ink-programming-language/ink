#ifndef INK_IR_TYPE_BUILTIN_TYPE_INTEGER_TYPE_H
#define INK_IR_TYPE_BUILTIN_TYPE_INTEGER_TYPE_H

#include "ink/core/numeric/integer_bits.h"
#include "ink/target/target_context.h"
#include "ink/ir/type/type.h"

namespace ink::ir
{
  class TypePool;

  class IntegerType final : public Type
  {
    public:
      std::uint32_t width() const noexcept;
      bool isSigned() const noexcept;
      bool isUnsigned() const noexcept;
      // Bounds use this type's width and signedness, encoded as normalized two's-complement bits.
      core::IntegerBits maximumValue() const;
      core::IntegerBits minimumValue() const;
      // Numeric input checks do not allocate or narrow the supplied host value.
      bool canRepresentSignedValue(std::int64_t Value) const noexcept;
      bool canRepresentUnsignedValue(std::uint64_t Value) const noexcept;
      // Storage bytes round up to whole bytes; aggregate strides additionally apply alignment.
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      std::string toString() const override;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypePool;

      IntegerType(const MetaType &Meta, const target::TargetContext &Target, std::uint32_t Width, bool Signed, bool Mutable = true) noexcept;

      std::uint32_t Width = 0;
      bool Signed = false;
      std::uint64_t Alignment = 0;
  };
} // namespace ink::ir

#endif
