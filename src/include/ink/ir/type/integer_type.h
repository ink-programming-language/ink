#ifndef INK_IR_TYPE_INTEGER_TYPE_H
#define INK_IR_TYPE_INTEGER_TYPE_H

#include "ink/core/target_context.h"
#include "ink/ir/type/type.h"

#include <memory>

namespace ink::ir
{
  class IntegerType final : public Type
  {
    public:
      // Width is in bits. Unsupported widths return nullptr; 32 and 64 are supported.
      static std::unique_ptr<IntegerType> create(const core::TargetContext &Target, std::uint32_t Width, bool Signed);

      std::uint32_t width() const noexcept;
      bool isSigned() const noexcept;
      bool isUnsigned() const noexcept;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      static bool classof(const Value *Value) noexcept;

    private:
      IntegerType(const core::TargetContext &Target, std::uint32_t Width, bool Signed) noexcept;

      std::uint32_t Width = 0;
      bool Signed = false;
      std::uint64_t Alignment = 0;
  };
} // namespace ink::ir

#endif
