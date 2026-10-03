#include "source_modules.h"

#include "ink/cli/io.h"
#include "ink/semantic/module_import.h"
#include "ink/tokenizer/tokenizer.h"

#include <algorithm>
#include <array>
#include <fstream>
#include <system_error>
#include <unordered_set>
#include <utility>

namespace ink::tools
{
  namespace
  {
    std::string pathText(const std::filesystem::path &Path)
    {
      const auto Text = Path.generic_u8string();
      return std::string(reinterpret_cast<const char *>(Text.data()), Text.size());
    }

    bool readModule(const std::filesystem::path &Path, std::string &Source, std::size_t &Remaining)
    {
      std::ifstream Input(Path, std::ios::binary);
      if (!Input)
      {
        return false;
      }
      std::array<char, 64 * 1024> Buffer;
      while (Input)
      {
        Input.read(Buffer.data(), static_cast<std::streamsize>(Buffer.size()));
        const std::size_t Count = static_cast<std::size_t>(Input.gcount());
        if (Count > Remaining)
        {
          return false;
        }
        Source.append(Buffer.data(), Count);
        Remaining -= Count;
      }
      return Input.eof() && !Input.bad();
    }
  } // namespace

  SourceModules::SourceModules(core::FrontendContext &Frontend)
      : Frontend(Frontend)
  {
  }

  void SourceModules::addModule(std::string Name, std::string DisplayPath, std::string Source)
  {
    const core::SourceId Id = Frontend.sourceManager().addSource(DisplayPath, std::move(Source));
    Modules.push_back({std::move(Name), parser::parse(Frontend, tokenizer::tokenizeSource(Frontend, Id))});
  }

  bool SourceModules::load(const std::string &InputFile, std::string Source, const std::string &ModuleRoot, bool RequireCanonicalPath, std::string &Error)
  {
    std::error_code Code;
    std::filesystem::path InputPath;
    if (InputFile != "-" && !cli::pathFromUtf8(InputFile, InputPath))
    {
      Error = "input path is not valid UTF-8";
      return false;
    }
    if (InputFile != "-")
    {
      InputPath = std::filesystem::absolute(InputPath, Code).lexically_normal();
      if (Code)
      {
        Error = "cannot resolve input path";
        return false;
      }
    }
    if (!ModuleRoot.empty())
    {
      if (!cli::pathFromUtf8(ModuleRoot, Root))
      {
        Error = "module root is not valid UTF-8";
        return false;
      }
      Root = std::filesystem::absolute(Root, Code).lexically_normal();
    }
    else
    {
      Root = InputFile == "-" ? std::filesystem::current_path(Code) : InputPath.parent_path();
    }
    if (Code)
    {
      Error = "cannot resolve module root";
      return false;
    }
    bool CanonicalEntryPath = true;
    std::string Entry = "main";
    if (InputFile != "-")
    {
      std::filesystem::path Relative = InputPath.lexically_relative(Root);
      if (Relative.empty() || Relative.is_absolute() || *Relative.begin() == "..")
      {
        Error = "input source must be inside --module-root";
        return false;
      }
      CanonicalEntryPath = Relative.extension() == ".ink";
      Relative.replace_extension();
      for (const auto &Component : Relative)
      {
        // A literal dot in a path component would alias a package directory separator.
        CanonicalEntryPath = CanonicalEntryPath && pathText(Component).find('.') == std::string::npos;
      }
      Entry = pathText(Relative);
      std::replace(Entry.begin(), Entry.end(), '/', '.');
    }
    if (RequireCanonicalPath && InputFile != "-" && !CanonicalEntryPath)
    {
      Error = "module source path must use an unambiguous .ink module name";
      return false;
    }
    std::size_t Remaining = 64 * 1024 * 1024;
    if (Source.size() > Remaining)
    {
      Error = "source module graph exceeds its 64 MiB source limit";
      return false;
    }
    Remaining -= Source.size();
    addModule(Entry, InputFile == "-" ? "<stdin>" : pathText(InputPath), std::move(Source));
    std::unordered_set<std::string> Visited{Entry};
    for (std::size_t Index = 0; Index < Modules.size(); ++Index)
    {
      const ParsedModule &Current = Modules[Index];
      if (!Current.Parsed.succeeded())
      {
        continue;
      }
      std::vector<std::string> Dependencies;
      for (const parser::Stmt *Statement : Current.Parsed.Unit->root()->statements())
      {
        if (parser::DirectImportStmt::classof(Statement))
        {
          HasImports = true;
          for (const parser::ImportEntry &Import : static_cast<const parser::DirectImportStmt &>(*Statement).imports())
          {
            Dependencies.push_back(semantic::resolveImportModuleName(Current.Name, Import.path()));
          }
        }
        else if (parser::FromImportStmt::classof(Statement))
        {
          HasImports = true;
          const auto &Import = static_cast<const parser::FromImportStmt &>(*Statement);
          Dependencies.push_back(semantic::resolveImportModuleName(Current.Name, Import.path(), Import.relativeLevel()));
        }
      }
      if (HasImports && !CanonicalEntryPath)
      {
        Error = "module source path must use an unambiguous .ink module name";
        return false;
      }
      for (const std::string &Dependency : Dependencies)
      {
        if (Dependency.empty() || !Visited.insert(Dependency).second)
        {
          continue;
        }
        if (Visited.size() > 1024)
        {
          Error = "source module graph exceeds its 1024 module limit";
          return false;
        }
        std::string Relative = Dependency;
        std::replace(Relative.begin(), Relative.end(), '.', '/');
        Relative += ".ink";
        std::filesystem::path RelativePath;
        if (!cli::pathFromUtf8(Relative, RelativePath))
        {
          Error = "imported module path is not valid UTF-8";
          return false;
        }
        const std::filesystem::path Path = Root / RelativePath;
        if (!std::filesystem::exists(Path, Code))
        {
          if (Code)
          {
            Error = "cannot inspect imported module '" + Dependency + "': " + Code.message();
            return false;
          }
          // The semantic import checker diagnoses a missing module at the import's source range.
          continue;
        }
        std::string Text;
        if (!readModule(Path, Text, Remaining))
        {
          Error = "cannot read imported module '" + Dependency + "' within the source byte limit";
          return false;
        }
        addModule(Dependency, pathText(Path), std::move(Text));
      }
    }
    if (!RequireCanonicalPath && !HasImports)
    {
      // Standalone interpretation has no exported module identity and accepts arbitrary source filenames.
      Modules.front().Name = "main";
    }
    return true;
  }

  bool SourceModules::succeeded() const noexcept
  {
    return !Modules.empty() && std::all_of(Modules.begin(), Modules.end(), [](const ParsedModule &Module)
    {
      return Module.Parsed.succeeded();
    });
  }

  const std::string &SourceModules::entryName() const noexcept
  {
    return Modules.front().Name;
  }

  std::vector<semantic::Analyzer::ModuleInput> SourceModules::inputs() const
  {
    std::vector<semantic::Analyzer::ModuleInput> Result;
    Result.reserve(Modules.size());
    for (const ParsedModule &Module : Modules)
    {
      Result.push_back({Module.Name, &Module.Parsed});
    }
    return Result;
  }
} // namespace ink::tools
