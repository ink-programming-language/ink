#include "module_loader.h"

#include "ink/backend/backend.h"
#include "ink/cli/application.h"
#include "ink/cli/diagnostic.h"
#include "ink/cli/io.h"
#include "ink/execution/interpreter.h"
#include "ink/frontend/compiler.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <cerrno>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif
#endif

#ifndef INK_DEFAULT_STD_PATH
#define INK_DEFAULT_STD_PATH ""
#endif

#ifndef INK_LINKER_PATH
#define INK_LINKER_PATH ""
#endif

#ifndef INK_KERNEL32_LIBRARY_PATH
#define INK_KERNEL32_LIBRARY_PATH "kernel32.lib"
#endif

namespace
{
  using ink::cli::ExitCode;

  struct EntryPointResult
  {
    std::optional<ink::ir::IrFunctionId> Function;
    ink::core::DiagnosticKind FailureKind = ink::core::DiagnosticKind::Unknown;
    std::optional<ink::core::SourceRange> FailureRange;
  };

  struct TemporaryFiles
  {
    std::vector<std::filesystem::path> Paths;

    ~TemporaryFiles()
    {
      for (const std::filesystem::path &Path : Paths)
      {
        std::error_code Error;
        std::filesystem::remove(Path, Error);
      }
    }
  };

  std::string pathText(const std::filesystem::path &Path)
  {
    return Path.generic_u8string();
  }

  std::filesystem::path executablePath(const char *ProgramPath)
  {
#ifdef _WIN32
    std::vector<wchar_t> Buffer(1024);
    while (true)
    {
      const DWORD Length = GetModuleFileNameW(nullptr, Buffer.data(), static_cast<DWORD>(Buffer.size()));
      if (Length == 0)
      {
        break;
      }
      if (Length < Buffer.size() - 1)
      {
        return std::filesystem::path(std::wstring(Buffer.data(), Length));
      }
      Buffer.resize(Buffer.size() * 2);
    }
#elif defined(__linux__)
    std::vector<char> Buffer(1024);
    while (true)
    {
      const ssize_t Length = readlink("/proc/self/exe", Buffer.data(), Buffer.size());
      if (Length < 0)
      {
        break;
      }
      if (static_cast<std::size_t>(Length) < Buffer.size())
      {
        return std::filesystem::path(std::string(Buffer.data(), static_cast<std::size_t>(Length)));
      }
      Buffer.resize(Buffer.size() * 2);
    }
#elif defined(__APPLE__)
    std::uint32_t Size = 0;
    if (_NSGetExecutablePath(nullptr, &Size) == -1)
    {
      std::vector<char> Buffer(Size);
      if (_NSGetExecutablePath(Buffer.data(), &Size) == 0)
      {
        return std::filesystem::path(Buffer.data());
      }
    }
#endif
    std::error_code Error;
    const std::filesystem::path Fallback = std::filesystem::absolute(ink::cli::pathFromUtf8(ProgramPath), Error);
    return Error ? ink::cli::pathFromUtf8(ProgramPath) : Fallback;
  }

  std::filesystem::path standardLibraryPath(const std::string &ExplicitPath, const char *ProgramPath)
  {
    if (!ExplicitPath.empty())
    {
      return ink::cli::pathFromUtf8(ExplicitPath);
    }
    std::error_code Error;
    const std::filesystem::path Executable = executablePath(ProgramPath);
    if (!Executable.empty())
    {
      const std::filesystem::path Deployed = Executable.parent_path() / "std";
      if (std::filesystem::is_directory(Deployed, Error) && !Error)
      {
        return Deployed;
      }
    }
    return ink::cli::pathFromUtf8(INK_DEFAULT_STD_PATH);
  }

  const ink::tools::inkc::SourceSection *sectionForRange(const ink::tools::inkc::LoadedProgram &Program, ink::core::SourceRange Range)
  {
    const ink::tools::inkc::SourceSection *Section = Program.sectionAt(Range.Start);
    if (Section == nullptr && Range.Start > 0)
    {
      Section = Program.sectionAt(Range.Start - 1);
    }
    return Section;
  }

  ink::cli::DiagnosticLocation sourceLocation(const ink::tools::inkc::LoadedProgram &Program, ink::core::SourceRange Range)
  {
    const ink::tools::inkc::SourceSection *Section = sectionForRange(Program, Range);
    if (Section == nullptr)
    {
      return {"<program>", Range};
    }
    return {pathText(Section->Path), ink::core::SourceRange{Range.Start - Section->Start, Range.End - Section->Start}};
  }

  ink::cli::DiagnosticLocation entryLocation(const ink::tools::inkc::LoadedProgram &Program)
  {
    if (Program.Sections.empty())
    {
      return {"<program>", ink::core::SourceRange{}};
    }
    return {pathText(Program.Sections.back().Path), ink::core::SourceRange{}};
  }

  std::optional<ink::core::SourceRange> originRange(const ink::ir::IrModule &Module, ink::ir::IrOriginId Origin)
  {
    if (!Module.contains(Origin))
    {
      return std::nullopt;
    }
    return Module.origin(Origin).Range;
  }

  EntryPointResult findMain(const ink::ir::VerifiedClosedModule &Closed)
  {
    const ink::ir::IrModule &Module = Closed.module();
    for (std::uint32_t Index = 0; Index < Module.functionCount(); ++Index)
    {
      const ink::ir::IrFunctionId Id = ink::ir::IrFunctionId::fromValue(Index);
      const ink::ir::IrFunction &Function = Module.function(Id);
      if (Function.Name != "main")
      {
        continue;
      }
      if (Function.Kind != ink::ir::IrFunctionKind::Definition)
      {
        return {std::nullopt, ink::core::DiagnosticKind::InvalidEntryPoint, originRange(Module, Function.Origin)};
      }
      const ink::ir::IrType &Signature = Module.type(Function.Signature);
      if (Signature.Kind != ink::ir::IrTypeKind::Function || Signature.Parameters.Count != 0 || !Signature.Result)
      {
        return {std::nullopt, ink::core::DiagnosticKind::InvalidEntryPoint, originRange(Module, Function.Origin)};
      }
      const ink::ir::IrType &Result = Module.type(*Signature.Result);
      if (Result.Kind != ink::ir::IrTypeKind::Integer || Result.BitWidth != 32 || Result.Signedness != ink::ir::IrSignedness::Signed)
      {
        return {std::nullopt, ink::core::DiagnosticKind::InvalidEntryPoint, originRange(Module, Function.Origin)};
      }
      return {Id, ink::core::DiagnosticKind::Unknown, std::nullopt};
    }
    return {std::nullopt, ink::core::DiagnosticKind::MissingEntryPoint, std::nullopt};
  }

  void reportCompilationIssues(const ink::frontend::ClosedCompilationResult &Compiled, const ink::tools::inkc::LoadedProgram &Program, ink::cli::DiagnosticConsumer &Diagnostics)
  {
    for (const ink::frontend::CompilationIssue &Issue : Compiled.Issues)
    {
      Diagnostics.report({Issue.Severity, Issue.DiagnosticKind, Issue.Message, sourceLocation(Program, Issue.Range), {}});
    }
  }

  ink::ir::IrTypeId staticOutputResultType(const ink::ir::VerifiedClosedModule &Module, const ink::tools::inkc::StaticOutputCall &Output)
  {
    const ink::ir::IrModule &IrModule = Module.module();
    for (std::uint32_t Index = 0; Index < IrModule.functionCount(); ++Index)
    {
      const ink::ir::IrFunctionId Id = ink::ir::IrFunctionId::fromValue(Index);
      const ink::ir::IrFunction &Function = IrModule.function(Id);
      if (Function.Name != Output.FunctionName)
      {
        continue;
      }
      const ink::ir::IrType &Signature = IrModule.type(Function.Signature);
      if (Function.Kind != ink::ir::IrFunctionKind::External || Signature.Parameters.Count != 0 || !Signature.Result || IrModule.type(*Signature.Result).Kind != ink::ir::IrTypeKind::Integer || IrModule.type(*Signature.Result).BitWidth != 32)
      {
        ink::cli::internalCompilerError("generated std.io.output runtime declaration has an invalid InkIR signature");
      }
      return *Signature.Result;
    }
    ink::cli::internalCompilerError("generated std.io.output runtime declaration is absent from InkIR");
  }

  ink::backend::AotRuntimeSupport makeAotRuntimeSupport(const ink::tools::inkc::LoadedProgram &Program)
  {
    ink::backend::AotRuntimeSupport Support;
    Support.StaticOutputFunctions.reserve(Program.StaticOutputCalls.size());
    for (const ink::tools::inkc::StaticOutputCall &Output : Program.StaticOutputCalls)
    {
      Support.StaticOutputFunctions.push_back({Output.FunctionName, Output.Bytes});
    }
    return Support;
  }

  int interpretProgram(const ink::ir::VerifiedClosedModule &Module, ink::ir::IrFunctionId Main, const ink::tools::inkc::LoadedProgram &Program, ink::cli::DiagnosticConsumer &Diagnostics)
  {
    ink::execution::RuntimeWorld World(Module.targetKey());
    for (const ink::tools::inkc::StaticOutputCall &Output : Program.StaticOutputCalls)
    {
      const ink::ir::IrTypeId ResultType = staticOutputResultType(Module, Output);
      const bool Bound = World.bindExternalFunction(Output.FunctionName, [ResultType, Bytes = Output.Bytes](const std::vector<ink::execution::RuntimeValue> &Arguments) -> std::optional<ink::execution::RuntimeValue>
      {
        if (!Arguments.empty())
        {
          ink::cli::internalCompilerError("generated std.io.output runtime call received arguments");
        }
        std::cout.write(Bytes.data(), static_cast<std::streamsize>(Bytes.size()));
        std::cout.flush();
        const std::uint64_t Result = std::cout.good() && Bytes.size() <= static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()) ? static_cast<std::uint64_t>(Bytes.size()) : std::numeric_limits<std::uint32_t>::max();
        return ink::execution::RuntimeValue::fromBits(ResultType, Result);
      });
      if (!Bound)
      {
        ink::cli::internalCompilerError("duplicate generated std.io.output runtime function");
      }
    }
    const ink::execution::ExecutionResult Result = ink::execution::interpret(Module, Main, World, {});
    if (!Result.returned())
    {
      const std::string Message = Result.Message.empty() ? ink::core::diagnosticDefaultMessage(ink::core::DiagnosticKind::RuntimeExecutionFailed) : Result.Message;
      if (Result.Status == ink::execution::ExecutionStatus::InternalInvariantFailure)
      {
        ink::cli::internalCompilerError("interpreter invariant failure: " + Message);
      }
      std::optional<ink::cli::DiagnosticLocation> Location;
      if (const std::optional<ink::core::SourceRange> Range = originRange(Module.module(), Result.Origin))
      {
        Location = sourceLocation(Program, *Range);
      }
      Diagnostics.report({ink::core::DiagnosticSeverity::Error, ink::core::DiagnosticKind::RuntimeExecutionFailed, Message, std::move(Location), {}});
      return ink::cli::exitStatus(Diagnostics.good() ? ExitCode::SourceError : ExitCode::InvocationError);
    }
    if (!Result.Value)
    {
      ink::cli::internalCompilerError("i32 main returned without a value");
    }
    return static_cast<std::int32_t>(Result.Value->bits());
  }

  std::filesystem::path makeTemporaryPath(const std::filesystem::path &Output, std::string_view Suffix)
  {
    const std::filesystem::path Parent = Output.has_parent_path() ? Output.parent_path() : std::filesystem::current_path();
    const std::string Base = Output.filename().generic_u8string();
    const std::uint64_t Seed = static_cast<std::uint64_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    for (std::uint64_t Attempt = 0; Attempt < 100; ++Attempt)
    {
      const std::filesystem::path Candidate = Parent / std::filesystem::u8path("." + Base + ".inkc-" + std::to_string(Seed + Attempt) + std::string(Suffix));
      std::error_code Error;
      if (!std::filesystem::exists(Candidate, Error) && !Error)
      {
        return Candidate;
      }
    }
    return {};
  }

#ifdef _WIN32
  std::wstring quoteWindowsArgument(const std::wstring &Argument)
  {
    std::wstring Result = L"\"";
    std::size_t Backslashes = 0;
    for (const wchar_t Character : Argument)
    {
      if (Character == L'\\')
      {
        ++Backslashes;
        continue;
      }
      if (Character == L'\"')
      {
        Result.append(Backslashes * 2 + 1, L'\\');
        Result.push_back(Character);
        Backslashes = 0;
        continue;
      }
      Result.append(Backslashes, L'\\');
      Backslashes = 0;
      Result.push_back(Character);
    }
    Result.append(Backslashes * 2, L'\\');
    Result.push_back(L'\"');
    return Result;
  }

  int executeProcess(const std::vector<std::filesystem::path> &Arguments, std::string &Error)
  {
    std::wstring CommandLine;
    for (const std::filesystem::path &Argument : Arguments)
    {
      if (!CommandLine.empty())
      {
        CommandLine.push_back(L' ');
      }
      CommandLine += quoteWindowsArgument(Argument.wstring());
    }
    STARTUPINFOW Startup{};
    Startup.cb = sizeof(Startup);
    PROCESS_INFORMATION Process{};
    if (!CreateProcessW(nullptr, CommandLine.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr, &Startup, &Process))
    {
      Error = "cannot start linker (Windows error " + std::to_string(GetLastError()) + ')';
      return -1;
    }
    const DWORD WaitResult = WaitForSingleObject(Process.hProcess, INFINITE);
    DWORD ExitStatus = 0;
    const bool GotStatus = GetExitCodeProcess(Process.hProcess, &ExitStatus) != FALSE;
    CloseHandle(Process.hThread);
    CloseHandle(Process.hProcess);
    if (WaitResult != WAIT_OBJECT_0 || !GotStatus)
    {
      Error = "cannot wait for linker process";
      return -1;
    }
    return static_cast<int>(ExitStatus);
  }
#else
  int executeProcess(const std::vector<std::filesystem::path> &Arguments, std::string &Error)
  {
    std::vector<std::string> Storage;
    std::vector<char *> Values;
    Storage.reserve(Arguments.size());
    Values.reserve(Arguments.size() + 1);
    for (const std::filesystem::path &Argument : Arguments)
    {
      Storage.push_back(Argument.string());
    }
    for (std::string &Argument : Storage)
    {
      Values.push_back(Argument.data());
    }
    Values.push_back(nullptr);
    const pid_t Child = fork();
    if (Child == -1)
    {
      Error = "cannot start linker: errno " + std::to_string(errno);
      return -1;
    }
    if (Child == 0)
    {
      execvp(Values.front(), Values.data());
      _exit(127);
    }
    int Status = 0;
    if (waitpid(Child, &Status, 0) == -1)
    {
      Error = "cannot wait for linker: errno " + std::to_string(errno);
      return -1;
    }
    if (!WIFEXITED(Status))
    {
      Error = "linker was terminated abnormally";
      return -1;
    }
    return WEXITSTATUS(Status);
  }
#endif

  int linkExecutable(const std::filesystem::path &Object, const std::filesystem::path &Executable, bool UsesStaticOutput, std::string &Error)
  {
    std::vector<std::filesystem::path> Arguments;
    Arguments.emplace_back(INK_LINKER_PATH);
#ifdef INK_USE_MSVC_LINKER
    Arguments.emplace_back("/NOLOGO");
    Arguments.emplace_back("/SUBSYSTEM:CONSOLE");
#if defined(_M_IX86)
    Arguments.emplace_back("/ENTRY:_main");
#else
    Arguments.emplace_back("/ENTRY:main");
#endif
    Arguments.emplace_back("/NODEFAULTLIB");
    Arguments.emplace_back(std::string("/OUT:") + Executable.generic_u8string());
    Arguments.push_back(Object);
    if (UsesStaticOutput)
    {
      Arguments.emplace_back(INK_KERNEL32_LIBRARY_PATH);
    }
#else
    Arguments.push_back(Object);
    Arguments.emplace_back("-o");
    Arguments.push_back(Executable);
#endif
    return executeProcess(Arguments, Error);
  }

  std::optional<ink::cli::DiagnosticLocation> backendErrorLocation(const ink::ir::VerifiedClosedModule &Module, const ink::backend::BackendError &Error, const ink::tools::inkc::LoadedProgram &Program)
  {
    const ink::ir::IrModule &IrModule = Module.module();
    ink::ir::IrOriginId Origin;
    if (IrModule.contains(Error.Operation))
    {
      Origin = IrModule.operation(Error.Operation).Origin;
    }
    else if (IrModule.contains(Error.Block))
    {
      Origin = IrModule.block(Error.Block).Origin;
    }
    else if (IrModule.contains(Error.Function))
    {
      Origin = IrModule.function(Error.Function).Origin;
    }
    const std::optional<ink::core::SourceRange> Range = originRange(IrModule, Origin);
    return Range ? std::optional<ink::cli::DiagnosticLocation>(sourceLocation(Program, *Range)) : std::nullopt;
  }

  int emitExecutable(const ink::ir::VerifiedClosedModule &Module, const std::filesystem::path &Output, const ink::tools::inkc::LoadedProgram &Program, ink::cli::DiagnosticConsumer &Diagnostics)
  {
    std::error_code FileSystemError;
    if (std::filesystem::exists(Output, FileSystemError))
    {
      Diagnostics.reportError("output already exists: '" + pathText(Output) + "'");
      return ink::cli::exitStatus(ExitCode::InvocationError);
    }
    if (FileSystemError)
    {
      Diagnostics.reportError("cannot inspect output path '" + pathText(Output) + "': " + FileSystemError.message());
      return ink::cli::exitStatus(ExitCode::InvocationError);
    }
    const std::filesystem::path Object = makeTemporaryPath(Output,
#ifdef INK_USE_MSVC_LINKER
      ".obj"
#else
      ".o"
#endif
    );
    const std::filesystem::path TemporaryExecutable = makeTemporaryPath(Output,
#ifdef _WIN32
      ".exe"
#else
      ".out"
#endif
    );
    if (Object.empty() || TemporaryExecutable.empty())
    {
      Diagnostics.reportError("cannot reserve temporary output paths");
      return ink::cli::exitStatus(ExitCode::InvocationError);
    }
    TemporaryFiles Cleanup{{Object, TemporaryExecutable}};
    const ink::backend::AotRuntimeSupport RuntimeSupport = makeAotRuntimeSupport(Program);
    const ink::backend::BackendResult<void> Emitted = ink::backend::emitObject(Module, Object, RuntimeSupport);
    if (!Emitted)
    {
      const ink::backend::BackendError &Error = Emitted.error();
      const std::string Message = std::string(ink::backend::backendErrorCodeName(Error.Code)) + ": " + Error.Message;
      if (Error.Code == ink::backend::BackendErrorCode::InvalidIr || Error.Code == ink::backend::BackendErrorCode::LlvmVerificationFailed)
      {
        ink::cli::internalCompilerError("backend invariant failure: " + Message);
      }
      if (Error.Code == ink::backend::BackendErrorCode::IoError || Error.Code == ink::backend::BackendErrorCode::OutputAlreadyExists)
      {
        Diagnostics.reportError(Message);
        return ink::cli::exitStatus(ExitCode::InvocationError);
      }
      Diagnostics.report({ink::core::DiagnosticSeverity::Error, ink::core::DiagnosticKind::BackendEmissionFailed, Message, backendErrorLocation(Module, Error, Program), {}});
      return ink::cli::exitStatus(Diagnostics.good() ? ExitCode::SourceError : ExitCode::InvocationError);
    }
    std::string LinkError;
    const int LinkStatus = linkExecutable(Object, TemporaryExecutable, !Program.StaticOutputCalls.empty(), LinkError);
    if (LinkStatus < 0)
    {
      Diagnostics.reportError(LinkError);
      return ink::cli::exitStatus(ExitCode::InvocationError);
    }
    if (LinkStatus != 0)
    {
      Diagnostics.report({ink::core::DiagnosticSeverity::Error, ink::core::DiagnosticKind::BackendEmissionFailed, "linker exited with status " + std::to_string(LinkStatus), std::nullopt, {}});
      return ink::cli::exitStatus(Diagnostics.good() ? ExitCode::SourceError : ExitCode::InvocationError);
    }
    std::filesystem::rename(TemporaryExecutable, Output, FileSystemError);
    if (FileSystemError)
    {
      Diagnostics.reportError("cannot publish output '" + pathText(Output) + "': " + FileSystemError.message());
      return ink::cli::exitStatus(ExitCode::InvocationError);
    }
    return ink::cli::exitStatus(ExitCode::Success);
  }

  std::filesystem::path defaultOutputPath(const std::filesystem::path &Input)
  {
    std::filesystem::path Output = Input.filename();
    Output.replace_extension();
#ifdef _WIN32
    Output += ".exe";
#endif
    return Output;
  }

  int runCompiler(int ArgumentCount, char **ArgumentValues)
  {
    ink::cli::Application Command({"inkc", "Compile and run Ink programs, or emit a native AOT executable.", "development"});
    bool Interpret = false;
    bool Aot = false;
    std::string InputText;
    std::string OutputText;
    std::string StandardLibraryText;
    CLI::Option *InterpretOption = Command.app().add_flag("--interpret", Interpret, "Interpret the program (the default mode).");
    CLI::Option *AotOption = Command.app().add_flag("--aot", Aot, "Emit a native executable instead of running the program.");
    InterpretOption->excludes(AotOption);
    AotOption->excludes(InterpretOption);
    Command.app().add_option("-o,--output", OutputText, "AOT executable output path.")->type_name("FILE");
    Command.app().add_option("--std-path", StandardLibraryText, "Override the Ink standard-library directory.")->type_name("DIR");
    Command.app().add_option("INPUT", InputText, "Entry Ink source file.")->type_name("FILE")->required();
    const ink::cli::ParseResult Parsed = Command.parse(ArgumentCount, ArgumentValues);
    if (Parsed.ShouldExit)
    {
      return ink::cli::exitStatus(Parsed.Code);
    }
    ink::cli::DiagnosticConsumer Diagnostics("inkc", std::cerr);
    if (!Aot && !OutputText.empty())
    {
      Diagnostics.reportError("'-o' requires '--aot'");
      return ink::cli::exitStatus(ExitCode::InvocationError);
    }
    static_cast<void>(Interpret);

    const std::filesystem::path Input = ink::cli::pathFromUtf8(InputText);
    const std::filesystem::path StandardLibrary = standardLibraryPath(StandardLibraryText, ArgumentValues[0]);
    if (!StandardLibraryText.empty())
    {
      std::error_code Error;
      if (!std::filesystem::is_directory(StandardLibrary, Error) || Error)
      {
        Diagnostics.reportError("standard-library directory does not exist: '" + pathText(StandardLibrary) + "'");
        return ink::cli::exitStatus(ExitCode::InvocationError);
      }
    }

    ink::tools::inkc::ModuleLoader Loader(StandardLibrary);
    ink::tools::inkc::ModuleLoadResult Loaded = Loader.load(Input);
    if (!Loaded.succeeded())
    {
      const ink::tools::inkc::ModuleLoadError &Error = *Loaded.Error;
      if (Error.IsSourceError)
      {
        Diagnostics.report({ink::core::DiagnosticSeverity::Error, ink::core::DiagnosticKind::InvalidModuleImport, Error.Message, ink::cli::DiagnosticLocation{pathText(Error.Path), std::nullopt}, {}});
        return ink::cli::exitStatus(Diagnostics.good() ? ExitCode::SourceError : ExitCode::InvocationError);
      }
      Diagnostics.reportError(pathText(Error.Path) + ": " + Error.Message);
      return ink::cli::exitStatus(ExitCode::InvocationError);
    }

    ink::frontend::CompilationSession Session;
    const ink::core::SourceFileId SourceFile = Session.addSource(pathText(Input), Loaded.Program->Source);
    ink::frontend::Compiler Compiler(Session);
    ink::frontend::ClosedCompilationResult Compiled = Compiler.compile(SourceFile);
    if (!Compiled.succeeded() || !Compiled.Module)
    {
      if (Compiled.Issues.empty())
      {
        ink::cli::internalCompilerError("frontend compilation failed without a diagnostic");
      }
      reportCompilationIssues(Compiled, *Loaded.Program, Diagnostics);
      return ink::cli::exitStatus(Diagnostics.good() ? ExitCode::SourceError : ExitCode::InvocationError);
    }
    const EntryPointResult EntryPoint = findMain(*Compiled.Module);
    if (!EntryPoint.Function)
    {
      const ink::cli::DiagnosticLocation Location = EntryPoint.FailureRange ? sourceLocation(*Loaded.Program, *EntryPoint.FailureRange) : entryLocation(*Loaded.Program);
      Diagnostics.report({ink::core::diagnosticDefaultSeverity(EntryPoint.FailureKind), EntryPoint.FailureKind, ink::core::diagnosticDefaultMessage(EntryPoint.FailureKind), Location, {}});
      return ink::cli::exitStatus(Diagnostics.good() ? ExitCode::SourceError : ExitCode::InvocationError);
    }
    if (!Aot)
    {
      return interpretProgram(*Compiled.Module, *EntryPoint.Function, *Loaded.Program, Diagnostics);
    }
    const std::filesystem::path Output = OutputText.empty() ? defaultOutputPath(Input) : ink::cli::pathFromUtf8(OutputText);
    return emitExecutable(*Compiled.Module, Output, *Loaded.Program, Diagnostics);
  }
} // namespace

int main(int ArgumentCount, char **ArgumentValues)
{
  return ink::cli::runMain("inkc", [ArgumentCount, ArgumentValues]()
  {
    return runCompiler(ArgumentCount, ArgumentValues);
  });
}
