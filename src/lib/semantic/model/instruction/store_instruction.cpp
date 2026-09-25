#include "ink/semantic/model/instruction/store_instruction.h"

#include "ink/semantic/model/context.h"

namespace ink::semantic
{
  const BuiltinType &StoreInstruction::type() const noexcept
  {
    return context().getVoidType();
  }
} // namespace ink::semantic
