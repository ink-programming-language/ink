#ifndef INK_IR_TYPE_BUILTIN_TYPE_INTEGER_TYPE_H
#define INK_IR_TYPE_BUILTIN_TYPE_INTEGER_TYPE_H

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
      // Bounds describe numeric values; raw bit patterns may also set a signed type's sign bit.
      std::uint64_t maximumValue() const noexcept;
      std::int64_t minimumValue() const noexcept;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      std::string toString() const override;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypePool;

      IntegerType(const MetaType &Meta, const target::TargetContext &Target, std::uint32_t Width, bool Signed) noexcept;

      std::uint32_t Width = 0;
      bool Signed = false;
      std::uint64_t Alignment = 0;
  };
} // namespace ink::ir

#endif
