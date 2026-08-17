#ifndef INK_EXECUTION_NATIVE_ABI_H
#define INK_EXECUTION_NATIVE_ABI_H

#include "engine/function_executor.h"

#include <cstddef>
#include <memory>
#include <vector>

namespace ink::execution
{
  class NativeCallAdapter final : public ExternalFunctionInvoker
  {
    public:
      NativeCallAdapter(ExecutionContext &Context, const ir::Module &ModuleValue);
      ~NativeCallAdapter() override;
      NativeCallAdapter(const NativeCallAdapter &) = delete;
      NativeCallAdapter &operator=(const NativeCallAdapter &) = delete;
      NativeCallAdapter(NativeCallAdapter &&) = delete;
      NativeCallAdapter &operator=(NativeCallAdapter &&) = delete;

      bool initialize(std::vector<core::Diagnostic> &Diagnostics);

    private:
      bool invokeExternal(std::size_t FunctionIndex, const std::vector<RuntimeValueRef> &Arguments, RuntimeValueArena &Values, RuntimeValueRef &Result, std::vector<core::Diagnostic> &Diagnostics) override;

      class Impl;
      std::unique_ptr<Impl> Implementation;
  };
} // namespace ink::execution

#endif
