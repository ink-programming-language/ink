#include "ink/cli/application.h"
#include "ink/cli/io.h"
#include "ink/execution/execution_engine.h"
#include "ink/execution/module/compiling_module_provider.h"
#include "ink/execution/runtime/runtime_symbols.h"
#include "ink/ir/compilation/compilation_session.h"
#include "ink/ir/compilation/source_module_compiler.h"
#include "ink/ir/serialization.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
  bool readFile(std::istream &Input, std::string &Text)
  {
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
    return Input.eof() && !Input.bad();
  }

  bool writeDiagnostics(const ink::core::SourceManager &Sources, const std::vector<ink::core::Diagnostic> &Diagnostics)
  {
    std::ostringstream BufferedErrorOutput;
    const ink::core::DiagnosticFormatter Formatter;
    for (const ink::core::Diagnostic &Diagnostic : Diagnostics)
    {
      const ink::core::FormattedDiagnostic Formatted = Formatter.format(Diagnostic);
      const std::shared_ptr<const ink::core::SourceBuffer> DiagnosticSource = Sources.findSource(Diagnostic.Source);
      if (DiagnosticSource != nullptr)
      {
        BufferedErrorOutput << DiagnosticSource->name() << ':' << DiagnosticSource->lineNumber(Diagnostic.Span.Start) << ": ";
      }
      BufferedErrorOutput << (Diagnostic.classification() == ink::core::DiagnosticClass::InternalCompilerError ? ink::core::diagnosticClassName(Diagnostic.classification()) : ink::core::diagnosticSeverityName(Formatted.Severity)) << '[' << Diagnostic.code() << "]: " << Formatted.Message << " [" << Diagnostic.Span.Start << ", " << Diagnostic.Span.End << ")\n";
      for (const ink::core::FormattedDiagnosticNote &Note : Formatted.Notes)
      {
        BufferedErrorOutput << "note: " << Note.Message;
        if (Note.Span)
        {
          BufferedErrorOutput << " [" << Note.Span->Start << ", " << Note.Span->End << ')';
        }
        BufferedErrorOutput << '\n';
      }
    }
    return ink::cli::writeOutput(std::cerr, BufferedErrorOutput.str());
  }

  ink::cli::ExitCode diagnosticExitCode(const std::vector<ink::core::Diagnostic> &Diagnostics)
  {
    const bool HasInternalCompilerError = std::any_of(Diagnostics.begin(), Diagnostics.end(), [](const ink::core::Diagnostic &Diagnostic)
                                                      {
                                                        return Diagnostic.classification() == ink::core::DiagnosticClass::InternalCompilerError;
                                                      });
    return HasInternalCompilerError ? ink::cli::ExitCode::InternalError : ink::cli::ExitCode::SourceError;
  }

  bool appendModuleSearchPath(std::string_view PathText, std::vector<std::filesystem::path> &SearchPaths)
  {
    std::filesystem::path Path;
    if (PathText.empty() || !ink::cli::pathFromUtf8(PathText, Path))
    {
      return false;
    }
    SearchPaths.push_back(std::move(Path));
    return true;
  }

  bool appendEnvironmentModuleSearchPaths(std::string_view Value, std::vector<std::filesystem::path> &SearchPaths)
  {
#ifdef _WIN32
    constexpr char PathSeparator = ';';
#else
    constexpr char PathSeparator = ':';
#endif
    std::size_t PathStart = 0;
    while (PathStart <= Value.size())
    {
      const std::size_t Separator = Value.find(PathSeparator, PathStart);
      const std::size_t PathEnd = Separator == std::string_view::npos ? Value.size() : Separator;
      if (PathEnd != PathStart && !appendModuleSearchPath(Value.substr(PathStart, PathEnd - PathStart), SearchPaths))
      {
        return false;
      }
      if (Separator == std::string_view::npos)
      {
        break;
      }
      PathStart = Separator + 1;
    }
    return true;
  }

  int successfulExitStatus(const ink::execution::RuntimeValue &ReturnValue)
  {
    if (ReturnValue.type().kind() == ink::ir::TypeKind::I32)
    {
      return static_cast<int>(static_cast<std::int32_t>(*ReturnValue.integer()));
    }
    return ink::cli::exitStatus(ink::cli::ExitCode::Success);
  }

  int runInterpreter(int ArgumentCount, char **ArgumentValues)
  {
    ink::cli::Application Command({"ink_interpreter", "Interpret and execute an InkIR module.", "development"});
    std::string InputFile;
    std::vector<std::string> ModulePathArguments;
    Command.addOption("-i", InputFile, "InkIR input file").required().typeName("FILE");
    Command.addOption("-I,--module-path", ModulePathArguments, "Add an InkIR module search path").repeatPolicy(ink::cli::RepeatPolicy::Append).typeName("DIRECTORY");
    const ink::cli::ParseResult ParsedArguments = Command.parse(ArgumentCount, ArgumentValues);
    if (ParsedArguments.ShouldExit)
    {
      return ink::cli::exitStatus(ParsedArguments.Code);
    }

    std::filesystem::path InputPath;
    if (!ink::cli::pathFromUtf8(InputFile, InputPath))
    {
      ink::cli::writeOutput(std::cerr, "ink_interpreter: error: input path is not valid UTF-8\n");
      return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
    }
    ink::ir::SourceModuleCompilerOptions CompilerOptions;
    for (const std::string &ModulePath : ModulePathArguments)
    {
      if (!appendModuleSearchPath(ModulePath, CompilerOptions.ModuleSearchPaths))
      {
        ink::cli::writeOutput(std::cerr, "ink_interpreter: error: module search path is not valid UTF-8\n");
        return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
      }
    }
    std::string EnvironmentModulePaths;
    const ink::cli::EnvironmentVariableStatus EnvironmentStatus = ink::cli::readEnvironmentVariable("INK_MODULE_PATH", EnvironmentModulePaths);
    if (EnvironmentStatus == ink::cli::EnvironmentVariableStatus::Invalid || (EnvironmentStatus == ink::cli::EnvironmentVariableStatus::Found && !appendEnvironmentModuleSearchPaths(EnvironmentModulePaths, CompilerOptions.ModuleSearchPaths)))
    {
      ink::cli::writeOutput(std::cerr, "ink_interpreter: error: INK_MODULE_PATH is not valid UTF-8\n");
      return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
    }
    std::ifstream Input(InputPath, std::ios::binary);
    if (!Input)
    {
      ink::cli::writeOutput(std::cerr, "ink_interpreter: error: cannot open '" + InputFile + "'\n");
      return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
    }
    std::string Text;
    if (!readFile(Input, Text))
    {
      ink::cli::writeOutput(std::cerr, "ink_interpreter: error: cannot read '" + InputFile + "'\n");
      return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
    }

    ink::core::CompilationContext Compilation;
    ink::core::CollectingDiagnosticConsumer Diagnostics;
    Compilation.diagnosticEngine().addConsumer(Diagnostics);
    ink::ir::SourceModuleCompiler Compiler(std::move(CompilerOptions));
    ink::ir::CompilationSession Session(Compilation, Compiler);
    const ink::core::SourceId Source = Compilation.sourceManager().addSource(InputFile, std::move(Text));
    ink::ir::DeserializeResult Deserialized = ink::ir::deserializeSource(Session.irContext(), Source);
    if (!Deserialized.succeeded())
    {
      const bool OutputSucceeded = writeDiagnostics(Compilation.sourceManager(), Diagnostics.diagnostics());
      return ink::cli::exitStatus(OutputSucceeded ? diagnosticExitCode(Diagnostics.diagnostics()) : ink::cli::ExitCode::InvocationError);
    }

    std::shared_ptr<const ink::ir::Module> EntryModule = std::make_shared<ink::ir::Module>(std::move(*Deserialized.module()));
    const ink::ir::Name EntryModuleName = EntryModule->Name.value_or(ink::ir::Name("application.entry"));
    if (!Compiler.addPrecompiledModule(EntryModuleName, EntryModule))
    {
      ink::cli::writeOutput(std::cerr, "ink_interpreter: internal error: cannot register the entry module\n");
      return ink::cli::exitStatus(ink::cli::ExitCode::InternalError);
    }
    ink::execution::ExecutionContext ExecutionContext(Compilation);
    if (!ink::execution::registerRuntimeSymbols(ExecutionContext.nativeSymbols()))
    {
      Compilation.diagnosticEngine().report<ink::core::DiagnosticKind::RuntimeSymbolRegistrationFailed>({});
      const bool OutputSucceeded = writeDiagnostics(Compilation.sourceManager(), Diagnostics.diagnostics());
      return ink::cli::exitStatus(OutputSucceeded ? ink::cli::ExitCode::InternalError : ink::cli::ExitCode::InvocationError);
    }
    ink::execution::CompilingModuleProvider Provider(Session);
    ink::execution::ExecutionEngine Engine(ExecutionContext, Provider, EntryModuleName);
    const ink::execution::ExecutionResult Executed = Engine.execute("main");
    if (!Executed.succeeded())
    {
      const bool OutputSucceeded = writeDiagnostics(Compilation.sourceManager(), Diagnostics.diagnostics());
      return ink::cli::exitStatus(OutputSucceeded ? diagnosticExitCode(Diagnostics.diagnostics()) : ink::cli::ExitCode::InvocationError);
    }
    return successfulExitStatus(*Executed.returnValue());
  }
} // namespace

int main(int ArgumentCount, char **ArgumentValues)
{
  return ink::cli::runMain("ink_interpreter", [ArgumentCount, ArgumentValues]()
                           {
                             return runInterpreter(ArgumentCount, ArgumentValues);
                           });
}
