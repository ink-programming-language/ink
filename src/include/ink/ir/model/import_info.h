#ifndef INK_IR_IMPORT_INFO_H
#define INK_IR_IMPORT_INFO_H

#include "ink/ir/model/name.h"

namespace ink::ir
{
  struct ImportInfo
  {
      ink::ir::Name Module;
      ink::ir::Name Symbol;
  };
} // namespace ink::ir

#endif
