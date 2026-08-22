#ifndef INK_TESTCASE_IR_IR_TEST_SUPPORT_H
#define INK_TESTCASE_IR_IR_TEST_SUPPORT_H

#include "ink/core/target_context.h"
#include "ink/ir/model/context.h"

#include "../diagnostic_test_support.h"

namespace ink::ir::test
{
  struct IRTestContext
  {
      IRTestContext()
          : Compilation(),
            IR(Compilation),
            Diagnostics(Compilation)
      {
      }

      explicit IRTestContext(core::TargetContext Target)
          : Compilation(Target),
            IR(Compilation),
            Diagnostics(Compilation)
      {
      }

      core::CompilationContext Compilation;
      IRContext IR;
      ink::test::DiagnosticCapture Diagnostics;
  };
} // namespace ink::ir::test

#endif
