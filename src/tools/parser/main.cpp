#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/diagnostic/diagnostic_formatter.h"
#include "ink/frontend/context.h"
#include "ink/frontend/parser/parser.h"
#include "ink/frontend/parser/ast_walker.h"

#include <cxxopts.hpp>
#include <spdlog/logger.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/ostream_sink.h>

#include <array>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

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
    const auto OutputSink = std::make_shared<spdlog::sinks::ostream_sink_st>(std::cout, true);
    const auto ErrorSink = std::make_shared<spdlog::sinks::ostream_sink_st>(std::cerr, true);
    spdlog::logger Output("ink-parse-output", OutputSink);
    spdlog::logger ErrorOutput("ink-parse-error", ErrorSink);
    bool OutputFailed = false;
    const auto HandleOutputFailure = [&OutputFailed](const std::string &)
    {
      OutputFailed = true;
    };
    Output.set_error_handler(HandleOutputFailure);
    ErrorOutput.set_error_handler(HandleOutputFailure);
    Output.set_formatter(std::make_unique<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, ""));
    ErrorOutput.set_formatter(std::make_unique<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, ""));
    const auto ArgumentError = [&ErrorOutput](const std::string &Message)
    {
      ErrorOutput.error("ink-parse: error: {}\nTry 'ink-parse --help' for more information.\n", Message);
      return 2;
    };

    int PositionalArgumentStart = ArgumentCount;
    for (int Index = 1; Index < ArgumentCount; ++Index)
    {
      const std::string_view Argument(ArgumentValues[Index]);
      if (Argument == "--")
      {
        PositionalArgumentStart = Index + 1;
        break;
      }
      if (Argument.starts_with("--help=") || Argument.starts_with("--h=") || Argument.starts_with("--version=") || Argument.starts_with("--V="))
      {
        return ArgumentError("--help and --version do not accept values");
      }
    }

    bool Help = false;
    bool Version = false;
    cxxopts::Options Options("ink-parse", "Parse Ink source and print the abstract syntax tree.\nINPUT is a file path, or '-' for standard input (the default).");
    Options.custom_help("[INPUT] [options]");
    Options.allow_unrecognised_options();
    auto AddOption = Options.add_options();
    AddOption("h,help", "Print help", cxxopts::value<bool>(Help));
    AddOption("V,version", "Print version", cxxopts::value<bool>(Version));

    // Reject explicit boolean values before cxxopts so invalid input never enters its fatal error path.
    const cxxopts::ParseResult Parsed = Options.parse(ArgumentCount, ArgumentValues);
    const auto &Unmatched = Parsed.unmatched();
    if (Unmatched.size() > 1)
    {
      return ArgumentError("expected at most one input file");
    }
    if (!Unmatched.empty())
    {
      const std::string &Input = Unmatched.front();
      if (Input.empty())
      {
        return ArgumentError("input file path must not be empty");
      }
      bool AfterTerminator = false;
      for (int Index = PositionalArgumentStart; Index < ArgumentCount; ++Index)
      {
        AfterTerminator = AfterTerminator || Input == ArgumentValues[Index];
      }
      if (Input.front() == '-' && Input != "-" && !AfterTerminator)
      {
        return ArgumentError("unexpected argument '" + Input + "'");
      }
    }
    if (Parsed.count("help") > 1 || Parsed.count("version") > 1)
    {
      return ArgumentError("options must not be repeated");
    }
    if (Help && Version)
    {
      return ArgumentError("option '--help' conflicts with '--version'");
    }
    if (Help)
    {
      Output.info("{}", Options.help());
      return !OutputFailed && std::cout ? 0 : 2;
    }
    if (Version)
    {
      Output.info("ink-parse development\n");
      return !OutputFailed && std::cout ? 0 : 2;
    }

    const std::string SourceFile = Unmatched.empty() ? "-" : Unmatched.front();
    std::string Source;
    if (SourceFile == "-")
    {
#ifdef _WIN32
      if (_setmode(_fileno(stdin), _O_BINARY) == -1)
      {
        ErrorOutput.error("ink-parse: error: cannot read standard input\n");
        return 2;
      }
#endif
      if (!readSource(std::cin, Source))
      {
        ErrorOutput.error("ink-parse: error: cannot read standard input\n");
        return 2;
      }
    }
    else
    {
      std::ifstream Input(SourceFile, std::ios::binary);
      if (!Input)
      {
        ErrorOutput.error("ink-parse: error: cannot open '{}'\n", SourceFile);
        return 2;
      }
      if (!readSource(Input, Source))
      {
        ErrorOutput.error("ink-parse: error: cannot read '{}'\n", SourceFile);
        return 2;
      }
    }

    ink::core::SourceManager Sources;
    ink::core::DiagnosticEngine DiagnosticEngine;
    ink::frontend::FrontendContext Context(Sources, DiagnosticEngine);
    ink::core::CollectingDiagnosticConsumer Diagnostics;
    DiagnosticEngine.addConsumer(Diagnostics);
    const ink::core::SourceId SourceId = Sources.addSource(SourceFile == "-" ? "<stdin>" : SourceFile, std::move(Source));
    const ink::frontend::parser::ParseResult Result = ink::frontend::parser::parse(Context, ink::frontend::tokenizer::tokenizeSource(Context, SourceId));
    std::ostringstream BufferedOutput;
    std::ostringstream BufferedErrorOutput;
    BufferedOutput << ink::frontend::parser::dumpAST(*Result.Unit);
    const ink::core::DiagnosticFormatter Formatter;
    bool HasInternalError = false;
    for (const ink::core::Diagnostic &Diagnostic : Diagnostics.diagnostics())
    {
      HasInternalError = HasInternalError || Diagnostic.classification() == ink::core::DiagnosticClass::InternalCompilerError;
      const ink::core::FormattedDiagnostic Formatted = Formatter.format(Diagnostic);
      const std::shared_ptr<const ink::core::SourceBuffer> DiagnosticSource = Sources.findSource(Diagnostic.Source);
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
    Output.info("{}", BufferedOutput.str());
    ErrorOutput.error("{}", BufferedErrorOutput.str());
    if (OutputFailed || !std::cout || !std::cerr)
    {
      return 2;
    }
    return HasInternalError ? 3 : Result.succeeded() ? 0 : 1;
  }
} // namespace

int main(int ArgumentCount, char **ArgumentValues)
{
  return runParser(ArgumentCount, ArgumentValues);
}
