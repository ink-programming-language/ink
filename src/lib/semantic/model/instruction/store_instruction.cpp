#include "ink/semantic/model/instruction/store_instruction.h"

#include "ink/semantic/context.h"

namespace ink::semantic
{
  StoreInstruction::StoreInstruction(const Value &Address, const Value &StoredValue) noexcept
      : Value(Address.context(), ValueKind::StoreInstruction, Address.context().getVoidType()),
        Address(Address),
        StoredValue(StoredValue)
  {
  }
} // namespace ink::semantic
