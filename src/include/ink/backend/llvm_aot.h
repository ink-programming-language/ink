#ifndef INK_BACKEND_LLVM_AOT_H
#define INK_BACKEND_LLVM_AOT_H

#include "ink/backend/result.h"
#include "ink/ir/module.h"

#include <filesystem>
#include <string>
#include <vector>

namespace ink::backend
{
  struct StaticOutputFunction
  {
    std::string FunctionName;
    std::string Bytes;
  };

  struct AotRuntimeSupport
  {
    std::vector<StaticOutputFunction> StaticOutputFunctions;
  };

  BackendResult<std::string> emitLlvmText(const ir::VerifiedClosedModule &Module, const AotRuntimeSupport &RuntimeSupport = {});
  BackendResult<void> emitObject(const ir::VerifiedClosedModule &Module, const std::filesystem::path &OutputPath, const AotRuntimeSupport &RuntimeSupport = {});
} // namespace ink::backend

#endif
