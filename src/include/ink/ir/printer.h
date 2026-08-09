#ifndef INK_IR_PRINTER_H
#define INK_IR_PRINTER_H

#include "ink/ir/module.h"

#include <string>

namespace ink::ir
{
  std::string printIr(const IrModule &Module);
  std::string printIr(const UnverifiedStagedModule &Module);
  std::string printIr(const VerifiedStagedModule &Module);
  std::string printIr(const VerifiedClosedModule &Module);
} // namespace ink::ir

#endif
