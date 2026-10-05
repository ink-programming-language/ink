#include "ink/backend/llvm/llvm_backend.h"

#include "lowering_context.h"

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>

#include <mutex>
#include <optional>
#include <utility>

namespace ink::backend::llvm
{
  LoweringResult::LoweringResult() = default;
  LoweringResult::~LoweringResult() = default;
  LoweringResult::LoweringResult(LoweringResult &&) noexcept = default;
  LoweringResult &LoweringResult::operator=(LoweringResult &&) noexcept = default;

  bool LoweringResult::succeeded() const noexcept
  {
    return Module != nullptr && Error.empty();
  }

  const std::string &LoweringResult::error() const noexcept
  {
    return Error;
  }

  const std::string &LoweringResult::hybridManifest() const noexcept
  {
    return HybridManifest;
  }

  const ::llvm::Module *LoweringResult::module() const noexcept
  {
    return Module.get();
  }

  ::llvm::Module *LoweringResult::module() noexcept
  {
    return Module.get();
  }

  bool LoweringResult::writeIR(std::string_view Path, std::string &Message) const
  {
    Message.clear();
    if (!succeeded())
    {
      Message = Error;
      return false;
    }
    std::error_code Code;
    ::llvm::raw_fd_ostream Output(::llvm::StringRef(Path.data(), Path.size()), Code, ::llvm::sys::fs::OF_None);
    if (Code)
    {
      Message = Code.message();
      return false;
    }
    Module->print(Output, nullptr);
    Output.flush();
    if (Output.has_error())
    {
      Message = Output.error().message();
      Output.clear_error();
      return false;
    }
    return true;
  }

  bool LoweringResult::writeObject(std::string_view Path, std::string &Message)
  {
    Message.clear();
    if (!succeeded())
    {
      Message = Error;
      return false;
    }
    std::error_code Code;
    ::llvm::raw_fd_ostream Output(::llvm::StringRef(Path.data(), Path.size()), Code, ::llvm::sys::fs::OF_None);
    if (Code)
    {
      Message = Code.message();
      return false;
    }
    ::llvm::legacy::PassManager Passes;
    if (Target->addPassesToEmitFile(Passes, Output, nullptr, ::llvm::CodeGenFileType::ObjectFile, false))
    {
      Message = "LLVM target cannot emit an object file";
      return false;
    }
    Passes.run(*Module);
    Output.flush();
    if (Output.has_error())
    {
      Message = Output.error().message();
      Output.clear_error();
      return false;
    }
    return true;
  }

  LoweringResult lowerToLLVMIR(::llvm::LLVMContext &Context, const ir::Module &Source, const ir::Function *Entry, const BackendOptions &Options)
  {
    LoweringResult Result;
    if (!Source.context().compilationContext().targetContext().isNativeAbiCompatible())
    {
      Result.Error = "AOT currently requires the native target ABI";
      return Result;
    }
    const std::string NativeTriple = ::llvm::Triple::normalize(::llvm::sys::getDefaultTargetTriple());
    const std::string TripleText = Options.TargetTriple.empty() ? NativeTriple : ::llvm::Triple::normalize(Options.TargetTriple);
    if (TripleText != NativeTriple)
    {
      Result.Error = "AOT cross compilation requires a matching target runtime and is not supported yet";
      return Result;
    }
    if (Options.OptimizationLevel > 3)
    {
      Result.Error = "AOT optimization level must be between 0 and 3";
      return Result;
    }
    static std::once_flag Initialization;
    static bool InitializationFailed = false;
    std::call_once(Initialization, []()
    {
      InitializationFailed = ::llvm::InitializeNativeTarget() || ::llvm::InitializeNativeTargetAsmPrinter();
    });
    if (InitializationFailed)
    {
      Result.Error = "LLVM native target initialization failed";
      return Result;
    }
    const ::llvm::Target *Target = ::llvm::TargetRegistry::lookupTarget(TripleText, Result.Error);
    if (!Target)
    {
      return Result;
    }
    const ::llvm::CodeGenOptLevel CodegenLevel = Options.OptimizationLevel == 0 ? ::llvm::CodeGenOptLevel::None : Options.OptimizationLevel == 1 ? ::llvm::CodeGenOptLevel::Less : Options.OptimizationLevel == 2 ? ::llvm::CodeGenOptLevel::Default : ::llvm::CodeGenOptLevel::Aggressive;
    Result.Target.reset(Target->createTargetMachine(::llvm::Triple(TripleText), "generic", "", ::llvm::TargetOptions{}, ::llvm::Reloc::PIC_, std::nullopt, CodegenLevel));
    if (!Result.Target)
    {
      Result.Error = "LLVM could not create the native target machine";
      return Result;
    }
    Result.Module = std::make_unique<::llvm::Module>(std::string(Source.context().namePool().text(Source.name())), Context);
    Result.Module->setTargetTriple(::llvm::Triple(TripleText));
    Result.Module->setDataLayout(Result.Target->createDataLayout());
    const auto &Layout = Result.Module->getDataLayout();
    const auto &InkTarget = Source.context().compilationContext().targetContext();
    if (Layout.getPointerSize() != InkTarget.pointerByteWidth() || Layout.isLittleEndian() != (InkTarget.byteOrder() == core::ByteOrder::LittleEndian))
    {
      Result.Error = "LLVM native target layout differs from the Ink target";
      Result.Module.reset();
      return Result;
    }
    LoweringContext Lowering(Context, Source, *Result.Module, Result.Error, Options.HotReload, Options.HotModules, Result.HybridManifest);
    if (!Lowering.lower(Entry))
    {
      Result.Module.reset();
      return Result;
    }
    std::string Verification;
    ::llvm::raw_string_ostream Diagnostics(Verification);
    if (::llvm::verifyModule(*Result.Module, &Diagnostics))
    {
      Result.Error = "LLVM verification failed: " + Verification;
      Result.Module.reset();
      return Result;
    }
    if (Options.OptimizationLevel != 0)
    {
      ::llvm::LoopAnalysisManager Loops;
      ::llvm::FunctionAnalysisManager Functions;
      ::llvm::CGSCCAnalysisManager Calls;
      ::llvm::ModuleAnalysisManager Modules;
      ::llvm::PassBuilder Builder(Result.Target.get());
      Builder.registerModuleAnalyses(Modules);
      Builder.registerCGSCCAnalyses(Calls);
      Builder.registerFunctionAnalyses(Functions);
      Builder.registerLoopAnalyses(Loops);
      Builder.crossRegisterProxies(Loops, Functions, Calls, Modules);
      const auto Level = Options.OptimizationLevel == 1 ? ::llvm::OptimizationLevel::O1 : Options.OptimizationLevel == 2 ? ::llvm::OptimizationLevel::O2 : ::llvm::OptimizationLevel::O3;
      ::llvm::ModulePassManager Passes = Builder.buildPerModuleDefaultPipeline(Level);
      Passes.run(*Result.Module, Modules);
      if (::llvm::verifyModule(*Result.Module, &Diagnostics))
      {
        Result.Error = "Optimized LLVM verification failed: " + Verification;
        Result.Module.reset();
      }
    }
    return Result;
  }
} // namespace ink::backend::llvm
