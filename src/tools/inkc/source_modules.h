#ifndef INKC_SOURCE_MODULES_H
#define INKC_SOURCE_MODULES_H

#include "ink/parser/parser.h"
#include "ink/semantic/analyzer/analyzer.h"

#include <deque>
#include <filesystem>
#include <string>
#include <vector>

namespace ink::tools
{
  // Own parsed dependency sources until after the semantic context borrowing their ASTs is destroyed.
  class SourceModules final
  {
    public:
      explicit SourceModules(core::FrontendContext &Frontend);
      bool load(const std::string &InputFile, std::string Source, const std::string &ModuleRoot, bool RequireCanonicalPath, std::string &Error);
      bool succeeded() const noexcept;
      const std::string &entryName() const noexcept;
      std::vector<semantic::Analyzer::ModuleInput> inputs() const;

    private:
      struct ParsedModule
      {
          std::string Name;
          parser::ParseResult Parsed;
      };

      void addModule(std::string Name, std::string DisplayPath, std::string Source);
      core::FrontendContext &Frontend;
      std::filesystem::path Root;
      std::deque<ParsedModule> Modules;
      bool HasImports = false;
  };
} // namespace ink::tools

#endif
