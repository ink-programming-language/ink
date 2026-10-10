#ifndef INK_IR_TYPE_FLOAT_TYPE_H
#define INK_IR_TYPE_FLOAT_TYPE_H

#include "ink/core/target_context.h"
#include "ink/ir/type/type.h"

#include <memory>

namespace ink::ir
{
  class FloatType final : public Type
  {
    public:
      // Width is in bits. Unsupported widths return nullptr; 32 and 64 are supported.
      static std::unique_ptr<FloatType> create(const core::TargetContext &Target, std::uint32_t Width);

      std::uint32_t width() const noexcept;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      static bool classof(const Value *Value) noexcept;

    private:
      FloatType(const core::TargetContext &Target, std::uint32_t Width) noexcept;

      std::uint32_t Width = 0;
      std::uint64_t Alignment = 0;
  };
} // namespace ink::ir

#endif
