#ifndef INK_LIB_BACKEND_LLVM_LOWERING_CONTEXT_H
#define INK_LIB_BACKEND_LLVM_LOWERING_CONTEXT_H

#include "ink/ir/ir.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace llvm
{
  class Constant;
  class Function;
  class GlobalVariable;
  class StructType;
  class Type;
} // namespace llvm

namespace ink::backend::llvm
{
  class LoweringContext final
  {
    public:
      LoweringContext(::llvm::LLVMContext &Context, const ir::Module &SourceModule);

      bool lower();
      std::unique_ptr<::llvm::Module> takeModule() noexcept;

    private:
      class FunctionLoweringContext;

      struct StructLowering
      {
          ::llvm::StructType *Type = nullptr;
          ::llvm::StructType *PayloadType = nullptr;
          std::vector<std::vector<unsigned>> FieldIndices;
          bool Defining = false;
          bool Defined = false;
      };

      template <core::DiagnosticKind Kind, typename... ArgumentTypes>
      void addFailure(ArgumentTypes &&...Arguments)
      {
        core::Diagnostic Diagnostic = core::makeDiagnostic<Kind>({}, std::forward<ArgumentTypes>(Arguments)...);
        SourceModule.context().diagnosticEngine().report(Diagnostic);
      }

      std::string valueName(ir::ValueId Value) const;
      std::string moduleName() const;
      std::string dataLayoutString() const;

      bool declareStructTypes();
      bool defineStructType(const ir::StructType &TypeValue);
      ::llvm::Type *lowerType(const ir::Type &TypeValue);
      ::llvm::Type *alignmentCarrierType(std::size_t Alignment);
      const std::vector<unsigned> *physicalFieldIndices(const ir::StructType &TypeValue, std::size_t LogicalIndex);

      bool lowerByteConstants();
      bool lowerGlobals();
      ::llvm::Constant *lowerConstant(const ir::Constant &ConstantValue);
      ::llvm::Constant *lowerGlobalAddress(const ir::GlobalAddressOperand &Address);
      ::llvm::Constant *lowerGlobalVariableAddress(const ir::GlobalVariableAddressOperand &Address);

      bool declareFunctions();
      bool lowerFunctions();
      bool lowerLifecycleFunctions();
      bool appendLifecycleFunction(const char *GlobalName, ::llvm::Function &FunctionValue);

      ::llvm::LLVMContext &Context;
      const ir::Module &SourceModule;
      std::unique_ptr<::llvm::Module> TargetModule;
      std::unordered_map<const ir::Type *, ::llvm::Type *> Types;
      std::unordered_map<const ir::StructType *, StructLowering> StructTypes;
      std::vector<::llvm::GlobalVariable *> ByteConstants;
      std::vector<::llvm::GlobalVariable *> Globals;
      std::vector<::llvm::Function *> Functions;
      std::unordered_map<const ir::StringConstant *, ::llvm::Constant *> StringConstants;
  };
} // namespace ink::backend::llvm

#endif
