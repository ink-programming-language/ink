#include "ink/cli/application.h"
#include "ink/cli/diagnostic.h"
#include "ink/cli/io.h"
#include "ink/frontend/compilation_session.h"

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
    ink::cli::DiagnosticConsumer Diagnostics("ink-tokenize", std::cerr);

    std::string Source;
    if (SourceFile == "-")
    {
      if (!ink::cli::useBinaryStandardInput() || !readSource(std::cin, Source))
      {
        Diagnostics.reportError("cannot read standard input");
        return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
      }
    }
    else
    {
      std::ifstream Input(ink::cli::pathFromUtf8(SourceFile), std::ios::binary);
      if (!Input)
      {
        Diagnostics.reportError("cannot open '" + SourceFile + "'");
        return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
      }
      if (!readSource(Input, Source))
      {
        Diagnostics.reportError("cannot read '" + SourceFile + "'");
        return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
      }
    }

    ink::frontend::CompilationSession Session;
    const ink::core::SourceFileId File = Session.addSource(SourceFile, std::move(Source));
    const ink::tokenizer::TokenizedBuffer Result = Session.tokenize(File);
    for (const ink::tokenizer::Token &Token : Result.tokens())
    {
      std::cout << ink::tokenizer::tokenKindName(Token.Kind) << " [" << Token.Span.Start << ", " << Token.Span.End << ")\n";
    }
    for (const ink::core::Diagnostic &Diagnostic : Result.diagnostics())
    {
      Diagnostics.report(Diagnostic, Session.sourceManager());
    }
    std::cout.flush();
    Diagnostics.flush();
    if (!std::cout || !Diagnostics.good())
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
