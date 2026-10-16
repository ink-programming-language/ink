#ifndef INK_IR_TYPE_STRUCTURE_TYPE_FUNCTION_TYPE_H
#define INK_IR_TYPE_STRUCTURE_TYPE_FUNCTION_TYPE_H

#include "ink/ir/type/structure_type/function_type_parameter.h"
#include "ink/target/target_context.h"
#include "ink/ir/type/type.h"

#include <cstddef>
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
      std::span<const FunctionTypeParameter> parameters() const noexcept;
      std::size_t parameterCount() const noexcept;
      std::uint64_t size() const noexcept override;
      std::uint64_t alignment() const noexcept override;
      std::string toString() const override;
      static bool classof(const Value *Value) noexcept;

    private:
      friend class TypePool;

      void appendToString(ValueStringBuilder &Builder) const override;

      // Owns immutable copies of parameter descriptors and borrows all referenced types.
      FunctionType(const target::TargetContext &Target, const Type *ReturnType, std::span<const FunctionTypeParameter> Parameters);

      const Type *ReturnType = nullptr;
      std::vector<FunctionTypeParameter> Parameters;
      std::uint64_t PointerByteWidth = 0;
  };
} // namespace ink::ir

#endif
