#ifndef INK_BACKEND_LLVM_LLVM_BACKEND_H
#define INK_BACKEND_LLVM_LLVM_BACKEND_H

#include <memory>

namespace llvm
{
  class LLVMContext;
  class Module;
} // namespace llvm

namespace ink::ir
{
  class Module;
} // namespace ink::ir

namespace ink::backend::llvm
{
  // Owns the lowered LLVM module while borrowing the caller-owned LLVMContext used to create it.
  class LoweringResult final
  {
    public:
      ~LoweringResult();
      LoweringResult(const LoweringResult &) = delete;
      LoweringResult &operator=(const LoweringResult &) = delete;
      LoweringResult(LoweringResult &&Other) noexcept;
      LoweringResult &operator=(LoweringResult &&Other) noexcept;

      bool succeeded() const noexcept;
      const ::llvm::Module *module() const noexcept;
      ::llvm::Module *module() noexcept;
      std::unique_ptr<::llvm::Module> takeModule() noexcept;

    private:
      explicit LoweringResult(std::unique_ptr<::llvm::Module> ModuleValue) noexcept;

      std::unique_ptr<::llvm::Module> ModuleValue;

      friend LoweringResult lowerToLLVMIR(::llvm::LLVMContext &Context, const ir::Module &ModuleValue);
  };

  // Lowers verified Closed InkIR. The caller must keep Context alive until the returned module is destroyed.
  LoweringResult lowerToLLVMIR(::llvm::LLVMContext &Context, const ir::Module &ModuleValue);
} // namespace ink::backend::llvm

#endif
