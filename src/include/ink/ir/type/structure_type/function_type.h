#ifndef INK_IR_TYPE_STRUCTURE_TYPE_FUNCTION_TYPE_H
#define INK_IR_TYPE_STRUCTURE_TYPE_FUNCTION_TYPE_H

#include "ink/ir/param/parameter.h"
#include "ink/target/target_context.h"
#include "ink/ir/type/type.h"

#include <cstddef>
#include <memory>
#include <span>
#include <vector>

namespace ink::ir
{
  class TypePool;

  // A signature whose capture-free callable value occupies one target-sized function address.
  class FunctionType final : public Type
  {
    public:
      const Type *returnType() const noexcept;
      const std::vector<std::unique_ptr<const Parameter>> &parameters() const noexcept;
      std::size_t parameterCount() const noexcept;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypePool;

      // Owns immutable copies of parameter descriptors and borrows all referenced types.
      FunctionType(const target::TargetContext &Target, const Type *ReturnType, std::span<const Parameter *const> Parameters);

      const Type *ReturnType = nullptr;
      std::vector<std::unique_ptr<const Parameter>> Parameters;
      std::uint64_t PointerByteWidth = 0;
  };
} // namespace ink::ir

#endif
