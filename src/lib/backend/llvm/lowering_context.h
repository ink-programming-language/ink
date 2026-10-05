#ifndef INK_BACKEND_LLVM_LOWERING_CONTEXT_H
#define INK_BACKEND_LLVM_LOWERING_CONTEXT_H

#include "ink/ir/context.h"
#include "ink/ir/function/function.h"
#include "ink/ir/analysis/type_layout.h"
#include "ink/execution/artifact/bytecode_artifact.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ink::backend::llvm
{
  class LoweringContext final
  {
    public:
      LoweringContext(::llvm::LLVMContext &Context, const ir::Module &Source, ::llvm::Module &Module, std::string &Error, bool HotReload, const std::vector<std::string> &HotModules, std::string &HybridManifest);
      bool lower(const ir::Function *Entry);
      ::llvm::Type *lowerType(const ir::Type &Type);
      ::llvm::FunctionType *signature(const ir::FunctionType &Type, bool Native = false);
      ::llvm::Constant *constant(const ir::Value &Value);
      ::llvm::FunctionCallee helper(::llvm::StringRef Name, ::llvm::Type *Result, ::llvm::ArrayRef<::llvm::Type *> Parameters);
      bool fail(std::string Message);
      std::optional<ir::TypeLayout> layout(const ir::Type &Type);
      bool nativeType(const ir::Type &Type, bool Return = false);
      ::llvm::Function *reflectionThunk(const ir::Function &Function);

      ::llvm::LLVMContext &Context;
      const ir::Module &Source;
      ::llvm::Module &Module;
      std::string &Error;
      ::llvm::IntegerType *SizeType;
      ::llvm::PointerType *PointerType;
      std::unordered_map<const ir::Function *, ::llvm::Function *> Functions;
      std::unordered_map<const ir::Function *, const ir::Function *> NativeDefinitions;
      std::unordered_map<const ir::Function *, execution::FunctionId> HotFunctions;
      ::llvm::GlobalVariable *HybridModule = nullptr;

    private:
      bool declareFunctions();
      bool recordClassABI(const ir::ClassType &Class);
      void emitABIMetadata();
      bool lowerFunction(const ir::Function &Function);
      bool lowerEntry(const ir::Function &Entry);
      bool lowerNativeExports();
      bool lowerReflection();
      bool prepareHybrid(const ir::Function *Entry);
      bool finishHybrid();
      void collect(const ir::BasicBlock &Block);

      std::vector<const ir::Function *> SourceFunctions;
      std::unordered_map<std::string, std::string> ClassDefinitions;
      std::unordered_map<const ir::Type *, ::llvm::Type *> Types;
      std::unordered_set<const ir::Type *> Defining;
      std::unordered_map<const ir::Value *, ::llvm::Constant *> Constants;
      bool HotReload;
      const std::vector<std::string> &HotModules;
      std::string &HybridManifest;
      std::unique_ptr<execution::BytecodeArtifact> HybridMetadata;
      std::unordered_map<execution::FunctionId, const ir::Function *> HybridSources;
  };
} // namespace ink::backend::llvm

#endif
