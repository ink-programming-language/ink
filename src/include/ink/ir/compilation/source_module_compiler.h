#ifndef INK_IR_SOURCE_MODULE_COMPILER_H
#define INK_IR_SOURCE_MODULE_COMPILER_H

#include "ink/ir/compilation/compilation_session.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace ink::ir
{
  struct SourceModuleCompilerOptions
  {
      std::vector<std::filesystem::path> ModuleSearchPaths;
      std::string ModuleFileExtension = ".ir";
  };

  class SourceModuleCompiler final : public ModuleCompiler
  {
    public:
      explicit SourceModuleCompiler(SourceModuleCompilerOptions Options);

      bool addPrecompiledModule(Name ModuleName, std::shared_ptr<const Module> ModuleValue);
      ModuleCompilationResult compileModule(CompilationSession &Session, const Name &ModuleName) noexcept override;

    private:
      SourceModuleCompilerOptions Options;
      std::unordered_map<Name, std::shared_ptr<const Module>> PrecompiledModules;
  };
} // namespace ink::ir

#endif
