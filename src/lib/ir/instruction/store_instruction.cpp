#include "ink/ir/instruction/store_instruction.h"

#include "ink/ir/context.h"

namespace ink::ir
{
  StoreInstruction::StoreInstruction(const Value &Address, const Value &StoredValue) noexcept
      : Value(Address.context(), ValueKind::StoreInstruction, Address.context().typePool().getType<TypeKind::Void>()),
        Address(Address),
        StoredValue(StoredValue)
  {
  }
} // namespace ink::ir
