#ifndef INK_EXECUTION_BRIDGE_SEMANTIC_VALUE_BRIDGE_H
#define INK_EXECUTION_BRIDGE_SEMANTIC_VALUE_BRIDGE_H

#include "ink/execution/runtime/runtime_value.h"
#include "ink/execution/value/execution_value.h"

#include <deque>
#include <memory>
#include <unordered_map>
#include <vector>

namespace ink::execution
{
  // All references back to semantic types, constants and functions stop here.
  // Runtime tables and values can outlive this adapter and its IR context.
  class SemanticValueBridge final
  {
    public:
      explicit SemanticValueBridge(ir::IRContext &Context, bool ResolveNativeImports = false);
      // Object construction temporarily disables process-local native export resolution.
      bool exchangeNativeImportResolution(bool Resolve) noexcept;
      std::shared_ptr<RuntimeTypeTable> types() const noexcept;
      RuntimeTypeId lowerType(const ir::Type &Type);
      const ir::Type *sourceType(RuntimeTypeId Type) const noexcept;
      FunctionId lowerFunction(const ir::Function &Function);
      const ir::Function *sourceFunction(FunctionId Function) const noexcept;
      const RuntimeFunctionDescriptor *functionDescriptor(FunctionId Function) const noexcept;
      RuntimeValueResult lowerValue(const ExecutionValueRef &Value);
      RuntimeValueResult lowerConstant(const ir::Value &Value);
      ExecutionValueResult raiseValue(ExecutionHeap &Heap, const RuntimeValue &Value, RuntimeTypeId Type);

    private:
      void updateNativeExports();
      ir::IRContext &Context;
      std::shared_ptr<RuntimeTypeTable> Types;
      std::unordered_map<const ir::Type *, RuntimeTypeId> TypeIds;
      std::vector<const ir::Type *> SourceTypes;
      std::unordered_map<const ir::Function *, FunctionId> FunctionIds;
      std::vector<const ir::Function *> SourceFunctions;
      std::deque<RuntimeFunctionDescriptor> Functions;
      bool ResolveNativeImports = false;
      std::uint64_t NativeExportRevision = 0;
      std::unordered_map<std::string, const ir::Function *> NativeExports;
  };
} // namespace ink::execution

#endif
