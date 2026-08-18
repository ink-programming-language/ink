#ifndef INK_LIB_IR_TEXT_PRINTER_H
#define INK_LIB_IR_TEXT_PRINTER_H

#include <string>

namespace ink::ir
{
  class Module;
}

namespace ink::ir::text
{
  std::string printModule(const Module &ModuleValue);
} // namespace ink::ir::text

#endif
