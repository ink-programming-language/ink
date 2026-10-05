#ifndef INK_EXECUTION_ENGINE_EXECUTION_LINKER_H
#define INK_EXECUTION_ENGINE_EXECUTION_LINKER_H

#include "ink/execution/bytecode/execution_image.h"
#include "ink/execution/engine/native_binding.h"

#include <cstdint>
#include <memory>
#include <unordered_map>

namespace ink::execution
{
  class SemanticValueBridge;

  class ExecutionLinker final
  {
    public:
      explicit ExecutionLinker(SemanticValueBridge &Bridge);
      // A fully linked image can execute after its semantic context is gone.
      explicit ExecutionLinker(ExecutionImage Image);
      bool synchronize(std::uint64_t Revision);
      FunctionId resolveFunction(FunctionId Function);
      const RuntimeFunctionDescriptor *descriptor(FunctionId Function);
      const ExecutableFunction *prepare(FunctionId Function, ExecutionStatus &Status);
      const RuntimeTypeTable *layouts() const noexcept;
      bool bindNative(FunctionId Function, NativeBinding Binding);
      const NativeBinding *nativeBinding(FunctionId Function) const noexcept;

    private:
      ExecutionStatus validate(const ExecutableFunction &Function);
      SemanticValueBridge *Bridge = nullptr;
      ExecutionImage Image;
      std::unordered_map<FunctionId, NativeBinding> NativeBindings;
      std::uint64_t Revision = 0;
  };
} // namespace ink::execution

#endif
