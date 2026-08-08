#include "ink/cli/application.h"
#include "ink/cli/io.h"
#include "ink/tokenizer/tokenizer.h"

#include <array>
#include <fstream>
#include <iostream>
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
    Command.app().add_option("INPUT", SourceFile, "Input file, or '-' for standard input")->type_name("FILE");
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
        std::cerr << "ink-tokenize: error: cannot read standard input\n";
        return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
      }
    }
    else
    {
      std::ifstream Input(ink::cli::pathFromUtf8(SourceFile), std::ios::binary);
      if (!Input)
      {
        std::cerr << "ink-tokenize: error: cannot open '" << SourceFile << "'\n";
        return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
      }
      if (!readSource(Input, Source))
      {
        std::cerr << "ink-tokenize: error: cannot read '" << SourceFile << "'\n";
        return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
      }
    }

    const ink::tokenizer::TokenizedBuffer Result = ink::tokenizer::tokenize(std::move(Source));
    for (const ink::tokenizer::Token &Token : Result.tokens())
    {
      std::cout << ink::tokenizer::tokenKindName(Token.Kind) << " [" << Token.Span.Start << ", " << Token.Span.End << ")\n";
    }
    const ink::core::DiagnosticFormatter Formatter;
    for (const ink::core::Diagnostic &Diagnostic : Result.diagnostics())
    {
      const ink::core::FormattedDiagnostic Formatted = Formatter.format(Diagnostic);
      std::cerr << ink::core::diagnosticSeverityName(Formatted.Severity) << "[" << Diagnostic.code() << "]: " << Formatted.Message << " [" << Diagnostic.Span.Start << ", " << Diagnostic.Span.End << ")\n";
      for (const ink::core::FormattedDiagnosticNote &Note : Formatted.Notes)
      {
        std::cerr << "note: " << Note.Message;
        if (Note.Span)
        {
          std::cerr << " [" << Note.Span->Start << ", " << Note.Span->End << ")";
        }
        std::cerr << '\n';
      }
    }
    std::cout.flush();
    std::cerr.flush();
    if (!std::cout || !std::cerr)
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
