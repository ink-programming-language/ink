#include "ink/cli/application.h"
#include "ink/cli/io.h"
#include "ink/parser/parser.h"
#include "ink/parser/ast_walker.h"

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
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

  int runParser(int ArgumentCount, char **ArgumentValues)
  {
    ink::cli::Application Command({"ink-parse", "Parse Ink source and print the abstract syntax tree.", "development"});
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
        ink::cli::writeOutput(std::cerr, "ink-parse: error: cannot read standard input\n");
        return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
      }
    }
    else
    {
      std::filesystem::path SourcePath;
      if (!ink::cli::pathFromUtf8(SourceFile, SourcePath))
      {
        ink::cli::writeOutput(std::cerr, "ink-parse: error: input path is not valid UTF-8\n");
        return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
      }
      std::ifstream Input(SourcePath, std::ios::binary);
      if (!Input)
      {
        ink::cli::writeOutput(std::cerr, "ink-parse: error: cannot open '" + SourceFile + "'\n");
        return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
      }
      if (!readSource(Input, Source))
      {
        ink::cli::writeOutput(std::cerr, "ink-parse: error: cannot read '" + SourceFile + "'\n");
        return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
      }
    }

    ink::core::CompilationContext Compilation;
    ink::core::FrontendContext Context(Compilation);
    ink::core::CollectingDiagnosticConsumer Diagnostics;
    Compilation.diagnosticEngine().addConsumer(Diagnostics);
    const ink::core::SourceId SourceId = Compilation.sourceManager().addSource(SourceFile == "-" ? "<stdin>" : SourceFile, std::move(Source));
    const ink::parser::ParseResult Result = ink::parser::parse(Context, ink::tokenizer::tokenizeSource(Context, SourceId));
    std::ostringstream BufferedOutput;
    std::ostringstream BufferedErrorOutput;
    BufferedOutput << ink::parser::dumpAST(*Result.Unit);
    const ink::core::DiagnosticFormatter Formatter;
    bool HasInternalError = false;
    for (const ink::core::Diagnostic &Diagnostic : Diagnostics.diagnostics())
    {
      HasInternalError = HasInternalError || Diagnostic.classification() == ink::core::DiagnosticClass::InternalCompilerError;
      const ink::core::FormattedDiagnostic Formatted = Formatter.format(Diagnostic);
      const std::shared_ptr<const ink::core::SourceBuffer> DiagnosticSource = Compilation.sourceManager().findSource(Diagnostic.Source);
      if (DiagnosticSource != nullptr && Diagnostic.Span.isValid())
      {
        BufferedErrorOutput << DiagnosticSource->name() << ':' << DiagnosticSource->lineNumber(Diagnostic.Span.getBegin().getByteOffset()) << ": ";
      }
      BufferedErrorOutput << (Diagnostic.classification() == ink::core::DiagnosticClass::InternalCompilerError ? ink::core::diagnosticClassName(Diagnostic.classification()) : ink::core::diagnosticSeverityName(Formatted.Severity)) << "[" << Diagnostic.code() << "]: " << Formatted.Message;
      if (Diagnostic.Span.isValid())
      {
        BufferedErrorOutput << " [" << Diagnostic.Span.getBegin().getByteOffset() << ", " << Diagnostic.Span.getEnd().getByteOffset() << ")";
      }
      BufferedErrorOutput << '\n';
      for (const ink::core::FormattedDiagnosticNote &Note : Formatted.Notes)
      {
        BufferedErrorOutput << "note: " << Note.Message;
        if (Note.Span)
        {
          BufferedErrorOutput << " [" << Note.Span->getBegin().getByteOffset() << ", " << Note.Span->getEnd().getByteOffset() << ")";
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
    return ink::cli::exitStatus(HasInternalError ? ink::cli::ExitCode::InternalError : Result.succeeded() ? ink::cli::ExitCode::Success
                                                                                                          : ink::cli::ExitCode::SourceError);
  }
} // namespace

int main(int ArgumentCount, char **ArgumentValues)
{
  const auto Run = [ArgumentCount, ArgumentValues]()
  {
    return runParser(ArgumentCount, ArgumentValues);
  };
  return ink::cli::runMain("ink-parse", Run);
}
