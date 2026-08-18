#ifndef INK_LIB_IR_TEXT_RESOLVER_H
#define INK_LIB_IR_TEXT_RESOLVER_H

#include "ink/core/diagnostic.h"

namespace ink::ir::text
{
  class ModuleDraft;

  bool resolveReferences(ModuleDraft &Draft, core::Diagnostic &Error);
} // namespace ink::ir::text

#endif
