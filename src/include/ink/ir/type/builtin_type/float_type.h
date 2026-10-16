#ifndef INK_IR_TYPE_BUILTIN_TYPE_FLOAT_TYPE_H
#define INK_IR_TYPE_BUILTIN_TYPE_FLOAT_TYPE_H

#include "ink/target/target_context.h"
#include "ink/ir/type/type.h"

namespace ink::ir
{
  class TypePool;

  class FloatType final : public Type
  {
    public:
      std::uint32_t width() const noexcept;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      std::string toString() const override;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypePool;

      FloatType(const target::TargetContext &Target, std::uint32_t Width) noexcept;

      std::uint32_t Width = 0;
      std::uint64_t Alignment = 0;
  };
} // namespace ink::ir

#endif
