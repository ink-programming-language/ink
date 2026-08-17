#include "ink/cli/application.h"
#include "ink/cli/io.h"
#include "ink/execution/execution_engine.h"
#include "ink/execution/runtime_symbols.h"
#include "ink/ir/serialization.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
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

  bool writeDiagnostics(const std::vector<ink::core::Diagnostic> &Diagnostics)
  {
    std::ostringstream BufferedErrorOutput;
    const ink::core::DiagnosticFormatter Formatter;
    for (const ink::core::Diagnostic &Diagnostic : Diagnostics)
    {
      const ink::core::FormattedDiagnostic Formatted = Formatter.format(Diagnostic);
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
    Command.addOption("-i", InputFile, "InkIR input file").required().typeName("FILE");
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
    ink::ir::IRContext IRContext(Compilation);
    ink::ir::DeserializeResult Deserialized = ink::ir::deserialize(IRContext, Text);
    if (!Deserialized.succeeded())
    {
      const bool OutputSucceeded = writeDiagnostics(Diagnostics.diagnostics());
      return ink::cli::exitStatus(OutputSucceeded ? diagnosticExitCode(Diagnostics.diagnostics()) : ink::cli::ExitCode::InvocationError);
    }

    ink::ir::Module ModuleValue = std::move(*Deserialized.module());
    ink::execution::ExecutionContext ExecutionContext(Compilation);
    if (!ink::execution::registerRuntimeSymbols(ExecutionContext.nativeSymbols()))
    {
      Compilation.diagnosticEngine().report<ink::core::DiagnosticKind::RuntimeSymbolRegistrationFailed>({});
      const bool OutputSucceeded = writeDiagnostics(Diagnostics.diagnostics());
      return ink::cli::exitStatus(OutputSucceeded ? ink::cli::ExitCode::InternalError : ink::cli::ExitCode::InvocationError);
    }
    ink::execution::ExecutionEngine Engine(ExecutionContext, ModuleValue);
    const ink::execution::ExecutionResult Executed = Engine.execute("main");
    if (!Executed.succeeded())
    {
      const bool OutputSucceeded = writeDiagnostics(Diagnostics.diagnostics());
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
