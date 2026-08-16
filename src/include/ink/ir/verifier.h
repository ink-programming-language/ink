#ifndef INK_IR_VERIFIER_H
#define INK_IR_VERIFIER_H

#include "ink/core/diagnostic.h"
#include "ink/ir/context.h"
#include "ink/ir/module.h"

#include <utility>
#include <vector>

namespace ink::ir
{
  class VerificationResult
  {
  public:
    bool succeeded() const noexcept
    {
      return Diagnostics.empty();
    }

    const std::vector<core::Diagnostic> &diagnostics() const noexcept
    {
      return Diagnostics;
    }

  private:
    explicit VerificationResult(std::vector<core::Diagnostic> Diagnostics) : Diagnostics(std::move(Diagnostics))
    {
    }

    std::vector<core::Diagnostic> Diagnostics;

    friend VerificationResult verify(IRContext &Context, const Module &ModuleValue, core::DiagnosticClass Class);
  };

  VerificationResult verify(IRContext &Context, const Module &ModuleValue);
  VerificationResult verify(IRContext &Context, const Module &ModuleValue, core::DiagnosticClass Class);
  VerificationResult verify(const Module &ModuleValue);
} // namespace ink::ir

#endif
