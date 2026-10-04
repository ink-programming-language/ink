#ifndef INK_BACKEND_LLVM_LLVM_BACKEND_H
#define INK_BACKEND_LLVM_LLVM_BACKEND_H

#include <memory>
#include <string>
#include <string_view>

namespace llvm
{
  class LLVMContext;
  class Module;
  class TargetMachine;
} // namespace llvm

namespace ink::ir
{
  class Module;
  class Function;
} // namespace ink::ir

namespace ink::backend::llvm
{
  struct BackendOptions
  {
      // Empty selects the host triple. Other targets need a matching runtime.
      std::string TargetTriple;
      unsigned OptimizationLevel = 0;
  };

  class LoweringResult final
  {
    public:
      LoweringResult();
      ~LoweringResult();
      LoweringResult(const LoweringResult &) = delete;
      LoweringResult &operator=(const LoweringResult &) = delete;
      LoweringResult(LoweringResult &&) noexcept;
      LoweringResult &operator=(LoweringResult &&) noexcept;

      bool succeeded() const noexcept;
      const std::string &error() const noexcept;
      const ::llvm::Module *module() const noexcept;
      ::llvm::Module *module() noexcept;
      bool writeIR(std::string_view Path, std::string &Error) const;
      bool writeObject(std::string_view Path, std::string &Error);

    private:
      std::unique_ptr<::llvm::Module> Module;
      std::unique_ptr<::llvm::TargetMachine> Target;
      std::string Error;

      friend LoweringResult lowerToLLVMIR(::llvm::LLVMContext &, const ir::Module &, const ir::Function *, const BackendOptions &);
  };

  // Entry adds a native main wrapper. The object links against ink_aot_runtime.
  // The LLVMContext and source IR must outlive lowering; LLVMContext must outlive the result.
  LoweringResult lowerToLLVMIR(::llvm::LLVMContext &Context, const ir::Module &Module, const ir::Function *Entry = nullptr, const BackendOptions &Options = {});
} // namespace ink::backend::llvm

#endif
