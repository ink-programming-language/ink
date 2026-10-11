#ifndef INK_IR_TYPE_FUNCTION_TYPE_H
#define INK_IR_TYPE_FUNCTION_TYPE_H

#include "ink/target/target_context.h"
#include "ink/ir/type/type.h"

#include <cstddef>
#include <vector>

namespace ink::ir
{
  class TypeContext;

  // A signature whose capture-free callable value occupies one target-sized function address.
  class FunctionType final : public Type
  {
    public:
      const Type *returnType() const noexcept;
      const std::vector<const Type *> &parameters() const noexcept;
      std::size_t parameterCount() const noexcept;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypeContext;

      // Owns the ordered parameter pointer list and borrows all referenced types.
      FunctionType(const target::TargetContext &Target, const Type *ReturnType, std::vector<const Type *> Parameters) noexcept;

      const Type *ReturnType = nullptr;
      std::vector<const Type *> Parameters;
      std::uint64_t PointerByteWidth = 0;
  };
} // namespace ink::ir

#endif
