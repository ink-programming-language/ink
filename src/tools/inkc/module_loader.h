#ifndef INK_TOOLS_INKC_MODULE_LOADER_H
#define INK_TOOLS_INKC_MODULE_LOADER_H

#include <cstddef>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace ink::tools::inkc
{
  struct SourceSection
  {
    std::filesystem::path Path;
    std::size_t Start = 0;
    std::size_t End = 0;
  };

  struct StaticOutputCall
  {
    std::string FunctionName;
    std::string Bytes;
  };

  struct LoadedProgram
  {
    std::string Source;
    std::vector<std::filesystem::path> Dependencies;
    std::vector<SourceSection> Sections;
    std::vector<StaticOutputCall> StaticOutputCalls;

    const SourceSection *sectionAt(std::size_t Offset) const noexcept;
  };

  struct ModuleLoadError
  {
    std::filesystem::path Path;
    std::string Message;
    bool IsSourceError = false;
  };

  struct ModuleLoadResult
  {
    std::optional<LoadedProgram> Program;
    std::optional<ModuleLoadError> Error;

    bool succeeded() const noexcept;
  };

  class ModuleLoader
  {
  public:
    explicit ModuleLoader(std::filesystem::path StandardLibraryRoot);

    ModuleLoadResult load(const std::filesystem::path &EntryPath);

  private:
    std::filesystem::path StandardLibraryRoot;
  };
} // namespace ink::tools::inkc

#endif
