#ifndef INK_IR_VERIFIER_H
#define INK_IR_VERIFIER_H

#include "ink/core/diagnostic.h"
#include "ink/ir/model/context.h"
#include "ink/ir/model/module.h"

namespace ink::ir
{
  class VerificationResult
  {
    public:
      bool succeeded() const noexcept
      {
        return Succeeded;
      }

    private:
      explicit VerificationResult(bool Succeeded)
          : Succeeded(Succeeded)
      {
      }

      bool Succeeded;

      friend VerificationResult verify(IRContext &Context, const Module &ModuleValue, core::DiagnosticClass Class);
      friend VerificationResult verify(IRContext &Context, const Module &ModuleValue, core::DiagnosticClass Class, core::SourceId Source);
  };

  VerificationResult verify(IRContext &Context, const Module &ModuleValue);
  VerificationResult verify(IRContext &Context, const Module &ModuleValue, core::DiagnosticClass Class);
  VerificationResult verify(IRContext &Context, const Module &ModuleValue, core::DiagnosticClass Class, core::SourceId Source);
  VerificationResult verify(const Module &ModuleValue);
} // namespace ink::ir

#endif
