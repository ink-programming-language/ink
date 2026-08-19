#ifndef INK_IR_COMPILATION_SESSION_H
#define INK_IR_COMPILATION_SESSION_H

#include "ink/ir/model/context.h"
#include "ink/ir/model/name.h"

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
namespace ink::ir
{
  class Module;

  enum class ModuleCompilationStatus : std::uint8_t
  {
    Found,
    NotFound,
    Failed,
  };

  struct ModuleCompilationResult
  {
      ModuleCompilationStatus Status = ModuleCompilationStatus::NotFound;
      std::shared_ptr<const Module> ModuleValue;

      static ModuleCompilationResult found(std::shared_ptr<const Module> ModuleValue) noexcept;
      static ModuleCompilationResult notFound() noexcept;
      static ModuleCompilationResult failure() noexcept;
  };

  class CompilationSession;

  class ModuleCompiler
  {
    public:
      virtual ~ModuleCompiler() = default;

      // Compilation is serialized by CompilationSession. The returned module must use Session.irContext() and remain immutable.
      // A compiler may request dependency modules from Session; a recursive request for an active module fails as a compile cycle.
      virtual ModuleCompilationResult compileModule(CompilationSession &Session, const Name &ModuleName) noexcept = 0;
  };

  class CompilationSession
  {
    public:
      CompilationSession(core::CompilationContext &Compilation, ModuleCompiler &Compiler);
      ~CompilationSession();
      CompilationSession(const CompilationSession &) = delete;
      CompilationSession &operator=(const CompilationSession &) = delete;
      CompilationSession(CompilationSession &&) = delete;
      CompilationSession &operator=(CompilationSession &&) = delete;

      IRContext &irContext() noexcept;
      const IRContext &irContext() const noexcept;
      ModuleCompilationResult getOrCompileModule(const Name &ModuleName) noexcept;

    private:
      class Impl;

      IRContext IR;
      ModuleCompiler &Compiler;
      std::unique_ptr<Impl> Implementation;
  };
} // namespace ink::ir

#endif
