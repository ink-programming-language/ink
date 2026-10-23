#ifndef INK_IR_MODULE_MODULE_ERROR_H
#define INK_IR_MODULE_MODULE_ERROR_H

#include <cstdint>

namespace ink::ir
{
  // Recoverable failures while registering modules or querying their explicit exports.
  enum class ModuleError : std::uint8_t
  {
    InvalidModule,
    InvalidModuleID,
    DuplicateModule,
    ContextMismatch,
    ModuleNotFound,
    InvalidSymbolID,
    DuplicateExport,
    InvalidExport,
    SymbolNotFound,
  };
} // namespace ink::ir

#endif
