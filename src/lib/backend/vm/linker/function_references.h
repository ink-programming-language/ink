#pragma once

#include "ink/backend/vm/module/machine_function.h"

#include <expected>
#include <functional>
#include <string>

namespace ink::vm::detail
{
  using SymbolReferenceVisitor = std::function<std::expected<void, std::string>(core::Name)>;

  // Validates function structure and visits symbol references in semantic operand order.
  // A reference failure stops the walk before later operands are validated, preserving the first error.
  std::expected<void, std::string> visitFunctionReferences(const MachineFunction &Function, const SymbolReferenceVisitor &VisitReference);
} // namespace ink::vm::detail
