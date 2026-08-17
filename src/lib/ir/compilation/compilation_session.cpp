#include "ink/ir/compilation/compilation_session.h"

#include "ink/ir/model/module.h"

#include <mutex>
#include <utility>

namespace ink::ir
{
  ModuleCompilationResult ModuleCompilationResult::found(std::shared_ptr<const Module> ModuleValue) noexcept
  {
    return {ModuleCompilationStatus::Found, std::move(ModuleValue), {}};
  }

  ModuleCompilationResult ModuleCompilationResult::notFound() noexcept
  {
    return {};
  }

  ModuleCompilationResult ModuleCompilationResult::failure(std::vector<core::Diagnostic> Diagnostics) noexcept
  {
    return {ModuleCompilationStatus::Failed, nullptr, std::move(Diagnostics)};
  }

  class CompilationSession::Impl
  {
    public:
      struct ModuleEntry
      {
          bool Compiling = true;
          ModuleCompilationResult Result;
      };

      std::recursive_mutex Mutex;
      std::unordered_map<Name, ModuleEntry> Modules;
  };

  CompilationSession::CompilationSession(core::CompilationContext &Compilation, ModuleCompiler &Compiler)
      : IR(Compilation),
        Compiler(Compiler),
        Implementation(std::make_unique<Impl>())
  {
  }

  CompilationSession::~CompilationSession() = default;

  IRContext &CompilationSession::irContext() noexcept
  {
    return IR;
  }

  const IRContext &CompilationSession::irContext() const noexcept
  {
    return IR;
  }

  ModuleCompilationResult CompilationSession::getOrCompileModule(const Name &ModuleName) noexcept
  {
    const std::lock_guard<std::recursive_mutex> Lock(Implementation->Mutex);
    const auto Existing = Implementation->Modules.find(ModuleName);
    if (Existing != Implementation->Modules.end())
    {
      return Existing->second.Compiling ? ModuleCompilationResult::failure() : Existing->second.Result;
    }

    Implementation->Modules.emplace(ModuleName, Impl::ModuleEntry{});
    ModuleCompilationResult Result = Compiler.compileModule(*this, ModuleName);
    if (Result.Status == ModuleCompilationStatus::Found && (Result.ModuleValue == nullptr || &Result.ModuleValue->context() != &IR))
    {
      Result = ModuleCompilationResult::failure(std::move(Result.Diagnostics));
    }
    Impl::ModuleEntry &Entry = Implementation->Modules.find(ModuleName)->second;
    Entry.Compiling = false;
    Entry.Result = std::move(Result);
    return Entry.Result;
  }
} // namespace ink::ir
