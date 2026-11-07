#pragma once

namespace ink::core
{
  class DiagnosticEngine;
} // namespace ink::core

namespace ink::ir
{
  class Function;
  class Module;

  // Read-only checks of complete, live IR graphs; success returns normally and the first failure reports a fatal ICE in every build mode.
  // Borrowed values must remain alive during verification. Type-valued metadata may retain external generic dependencies.
  void verifyModule(const Module &ModuleValue, const core::DiagnosticEngine &Diagnostics);

  // Also accepts standalone and record-owned functions. Function-local SSA values may only be used by their owning function.
  // The first block is the entry and must have no predecessors. Unreachable blocks still require valid structure, types and local use order.
  void verifyFunction(const Function &FunctionValue, const core::DiagnosticEngine &Diagnostics);
} // namespace ink::ir
