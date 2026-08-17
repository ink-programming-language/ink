#include "ink/ir/compilation/source_module_compiler.h"

#include "ink/ir/compilation/module_name.h"
#include "ink/ir/serialization.h"

#include <array>
#include <cstddef>
#include <fstream>
#include <system_error>
#include <utility>

namespace ink::ir
{
  namespace
  {
    enum class SourceReadStatus
    {
      Succeeded,
      OpenFailed,
      ReadFailed,
    };

    std::filesystem::path moduleRelativePath(const Name &ModuleName, std::string_view Extension)
    {
      const std::string_view Text = ModuleName.text();
      std::filesystem::path Result;
      std::size_t SegmentStart = 0;
      while (SegmentStart < Text.size())
      {
        const std::size_t Separator = Text.find('.', SegmentStart);
        const std::size_t SegmentEnd = Separator == std::string_view::npos ? Text.size() : Separator;
        Result /= std::string(Text.substr(SegmentStart, SegmentEnd - SegmentStart));
        SegmentStart = SegmentEnd + 1;
      }
      Result += Extension;
      return Result;
    }

    SourceReadStatus readSourceFile(const std::filesystem::path &Path, std::string &Text)
    {
      std::ifstream Input(Path, std::ios::binary);
      if (!Input)
      {
        return SourceReadStatus::OpenFailed;
      }
      std::array<char, 64 * 1024> Buffer;
      while (Input)
      {
        Input.read(Buffer.data(), static_cast<std::streamsize>(Buffer.size()));
        const std::streamsize Count = Input.gcount();
        if (Count > 0)
        {
          Text.append(Buffer.data(), static_cast<std::size_t>(Count));
        }
      }
      return Input.eof() && !Input.bad() ? SourceReadStatus::Succeeded : SourceReadStatus::ReadFailed;
    }

    template <core::DiagnosticKind Kind>
    ModuleCompilationResult sourceFailure(CompilationSession &Session, const Name &ModuleName)
    {
      core::Diagnostic Error = core::makeDiagnostic<Kind>({}, ModuleName);
      Session.irContext().diagnosticEngine().report(Error);
      std::vector<core::Diagnostic> Diagnostics;
      Diagnostics.push_back(std::move(Error));
      return ModuleCompilationResult::failure(std::move(Diagnostics));
    }
  } // namespace

  SourceModuleCompiler::SourceModuleCompiler(SourceModuleCompilerOptions Options)
      : Options(std::move(Options))
  {
  }

  bool SourceModuleCompiler::addPrecompiledModule(Name ModuleName, std::shared_ptr<const Module> ModuleValue)
  {
    if (!isValidModuleName(ModuleName) || ModuleValue == nullptr)
    {
      return false;
    }
    return PrecompiledModules.emplace(std::move(ModuleName), std::move(ModuleValue)).second;
  }

  ModuleCompilationResult SourceModuleCompiler::compileModule(CompilationSession &Session, const Name &ModuleName) noexcept
  {
    const auto Precompiled = PrecompiledModules.find(ModuleName);
    if (Precompiled != PrecompiledModules.end())
    {
      return ModuleCompilationResult::found(Precompiled->second);
    }
    if (!isValidModuleName(ModuleName))
    {
      return ModuleCompilationResult::notFound();
    }

    const std::filesystem::path RelativePath = moduleRelativePath(ModuleName, Options.ModuleFileExtension);
    for (const std::filesystem::path &SearchPath : Options.ModuleSearchPaths)
    {
      const std::filesystem::path Candidate = SearchPath / RelativePath;
      std::error_code FileStatusError;
      const std::filesystem::file_status FileStatus = std::filesystem::status(Candidate, FileStatusError);
      if (FileStatusError)
      {
        if (FileStatusError == std::errc::no_such_file_or_directory || FileStatusError == std::errc::not_a_directory)
        {
          continue;
        }
        return sourceFailure<core::DiagnosticKind::IrModuleSourceOpenFailed>(Session, ModuleName);
      }
      if (!std::filesystem::exists(FileStatus))
      {
        continue;
      }
      if (!std::filesystem::is_regular_file(FileStatus))
      {
        return sourceFailure<core::DiagnosticKind::IrModuleSourceOpenFailed>(Session, ModuleName);
      }

      std::string Text;
      const SourceReadStatus ReadStatus = readSourceFile(Candidate, Text);
      if (ReadStatus == SourceReadStatus::OpenFailed)
      {
        return sourceFailure<core::DiagnosticKind::IrModuleSourceOpenFailed>(Session, ModuleName);
      }
      if (ReadStatus == SourceReadStatus::ReadFailed)
      {
        return sourceFailure<core::DiagnosticKind::IrModuleSourceReadFailed>(Session, ModuleName);
      }

      DeserializeResult Parsed = deserialize(Session.irContext(), Text);
      if (!Parsed.succeeded())
      {
        return ModuleCompilationResult::failure(Parsed.diagnostics());
      }
      return ModuleCompilationResult::found(std::make_shared<Module>(std::move(*Parsed.module())));
    }
    return ModuleCompilationResult::notFound();
  }
} // namespace ink::ir
