#include "ink/backend/llvm/llvm_backend.h"

#include "lowering_context.h"

#include "ink/ir/analysis/verifier.h"
#include "ink/ir/model/module.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ink::backend::llvm
{
  LoweringResult::LoweringResult(std::unique_ptr<::llvm::Module> ModuleValue) noexcept
      : ModuleValue(std::move(ModuleValue))
  {
  }

  LoweringResult::~LoweringResult() = default;
  LoweringResult::LoweringResult(LoweringResult &&Other) noexcept = default;
  LoweringResult &LoweringResult::operator=(LoweringResult &&Other) noexcept = default;

  bool LoweringResult::succeeded() const noexcept
  {
    return ModuleValue != nullptr;
  }

  const ::llvm::Module *LoweringResult::module() const noexcept
  {
    return ModuleValue.get();
  }

  ::llvm::Module *LoweringResult::module() noexcept
  {
    return ModuleValue.get();
  }

  std::unique_ptr<::llvm::Module> LoweringResult::takeModule() noexcept
  {
    return std::move(ModuleValue);
  }

  LoweringResult lowerToLLVMIR(::llvm::LLVMContext &Context, const ir::Module &ModuleValue)
  {
    ir::VerificationResult Verification = ir::verify(ModuleValue);
    if (!Verification.succeeded())
    {
      return LoweringResult(nullptr);
    }

    LoweringContext Lowering(Context, ModuleValue);
    if (!Lowering.lower())
    {
      return LoweringResult(nullptr);
    }

    std::unique_ptr<::llvm::Module> TargetModule = Lowering.takeModule();
    std::string VerificationMessage;
    ::llvm::raw_string_ostream VerificationStream(VerificationMessage);
    if (::llvm::verifyModule(*TargetModule, &VerificationStream))
    {
      VerificationStream.flush();
      core::Diagnostic Diagnostic = core::makeDiagnostic<core::DiagnosticKind::LLVMBackendVerificationFailed>({}, VerificationMessage);
      ModuleValue.context().diagnosticEngine().report(Diagnostic);
      return LoweringResult(nullptr);
    }
    return LoweringResult(std::move(TargetModule));
  }
} // namespace ink::backend::llvm
