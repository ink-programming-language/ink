#pragma once

#include "ink/core/name/name.h"

namespace ink::vm
{
  // Names the host function independently of its VM link alias.
  struct NativeFunctionImport
  {
      core::Name Name = core::NAME_None;
  };
} // namespace ink::vm
