#include "ink/cli/application.h"
#include "ink/cli/io.h"
#include "ink/tokenizer/tokenizer.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>

namespace
{
  bool readSource(std::istream &Input, std::string &Source)
  {
    std::array<char, 64 * 1024> Buffer;
    while (Input)
    {
      Input.read(Buffer.data(), static_cast<std::streamsize>(Buffer.size()));
      const std::streamsize Count = Input.gcount();
      if (Count > 0)
      {
        Source.append(Buffer.data(), static_cast<std::size_t>(Count));
      }
    }
    return Input.eof() && !Input.bad();
  }

  int runTokenizer(int ArgumentCount, char **ArgumentValues)
  {
    ink::cli::Application Command({"ink-tokenize", "Tokenize Ink source and print the token stream.", "development"});
    std::string SourceFile = "-";
    Command.addOption("INPUT", SourceFile, "Input file, or '-' for standard input").typeName("FILE");
    const ink::cli::ParseResult ParsedArguments = Command.parse(ArgumentCount, ArgumentValues);
    if (ParsedArguments.ShouldExit)
    {
      return ink::cli::exitStatus(ParsedArguments.Code);
    }

    std::string Source;
    if (SourceFile == "-")
    {
      if (!ink::cli::useBinaryStandardInput() || !readSource(std::cin, Source))
      {
        ink::cli::writeOutput(std::cerr, "ink-tokenize: error: cannot read standard input\n");
        return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
      }
    }
    else
    {
      std::filesystem::path SourcePath;
      if (!ink::cli::pathFromUtf8(SourceFile, SourcePath))
      {
        ink::cli::writeOutput(std::cerr, "ink-tokenize: error: input path is not valid UTF-8\n");
        return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
      }
      std::ifstream Input(SourcePath, std::ios::binary);
      if (!Input)
      {
        ink::cli::writeOutput(std::cerr, "ink-tokenize: error: cannot open '" + SourceFile + "'\n");
        return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
      }
      if (!readSource(Input, Source))
      {
        ink::cli::writeOutput(std::cerr, "ink-tokenize: error: cannot read '" + SourceFile + "'\n");
        return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
      }
    }

    ink::core::CompilationContext Compilation;
    ink::core::FrontendContext Context(Compilation);
    ink::core::CollectingDiagnosticConsumer Diagnostics;
    Compilation.diagnosticEngine().addConsumer(Diagnostics);
    const ink::tokenizer::TokenizedBuffer Result = ink::tokenizer::tokenize(Context, std::move(Source));
    std::ostringstream BufferedOutput;
    std::ostringstream BufferedErrorOutput;
    for (const ink::tokenizer::Token &Token : Result.tokens())
    {
      BufferedOutput << ink::tokenizer::tokenKindName(Token.Kind) << " [" << Token.Span.Start << ", " << Token.Span.End << ")\n";
    }
    const ink::core::DiagnosticFormatter Formatter;
    for (const ink::core::Diagnostic &Diagnostic : Diagnostics.diagnostics())
    {
      const ink::core::FormattedDiagnostic Formatted = Formatter.format(Diagnostic);
      BufferedErrorOutput << (Diagnostic.classification() == ink::core::DiagnosticClass::InternalCompilerError ? ink::core::diagnosticClassName(Diagnostic.classification()) : ink::core::diagnosticSeverityName(Formatted.Severity)) << "[" << Diagnostic.code() << "]: " << Formatted.Message << " [" << Diagnostic.Span.Start << ", " << Diagnostic.Span.End << ")\n";
      for (const ink::core::FormattedDiagnosticNote &Note : Formatted.Notes)
      {
        BufferedErrorOutput << "note: " << Note.Message;
        if (Note.Span)
        {
          BufferedErrorOutput << " [" << Note.Span->Start << ", " << Note.Span->End << ")";
        }
        BufferedErrorOutput << '\n';
      }
    }
    const bool OutputSucceeded = ink::cli::writeOutput(std::cout, BufferedOutput.str());
    const bool ErrorOutputSucceeded = ink::cli::writeOutput(std::cerr, BufferedErrorOutput.str());
    if (!OutputSucceeded || !ErrorOutputSucceeded)
    {
      return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
    }
    return ink::cli::exitStatus(Result.succeeded() ? ink::cli::ExitCode::Success : ink::cli::ExitCode::SourceError);
  }
} // namespace

int main(int ArgumentCount, char **ArgumentValues)
{
  return ink::cli::runMain("ink-tokenize", [ArgumentCount, ArgumentValues]()
  {
    return runTokenizer(ArgumentCount, ArgumentValues);
  });
}
