#ifndef INK_EXECUTION_RUNTIME_VALUE_H
#define INK_EXECUTION_RUNTIME_VALUE_H

#include "ink/ir/ids.h"

#include <cstdint>

namespace ink::execution
{
  class RuntimeValue
  {
  public:
    static RuntimeValue fromBits(ir::IrTypeId Type, std::uint64_t Bits) noexcept;
    static RuntimeValue fromBool(ir::IrTypeId Type, bool Value) noexcept;

    ir::IrTypeId type() const noexcept;
    std::uint64_t bits() const noexcept;

  private:
    RuntimeValue(ir::IrTypeId Type, std::uint64_t Bits) noexcept;

    ir::IrTypeId Type;
    std::uint64_t Bits = 0;
  };

  bool operator==(RuntimeValue Left, RuntimeValue Right) noexcept;
  bool operator!=(RuntimeValue Left, RuntimeValue Right) noexcept;
} // namespace ink::execution

#endif
