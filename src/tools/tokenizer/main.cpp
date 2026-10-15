#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/diagnostic/diagnostic_formatter.h"
#include "ink/frontend/context.h"
#include "ink/frontend/tokenizer/tokenizer.h"

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
  void printSpelling(std::ostream &Output, std::string_view Spelling)
  {
    constexpr char HexDigits[] = "0123456789ABCDEF";
    Output << '\"';
    for (const unsigned char Byte : Spelling)
    {
      if (Byte == '\"' || Byte == '\\')
      {
        Output << '\\' << static_cast<char>(Byte);
      }
      else if (Byte >= 0x20 && Byte < 0x7F)
      {
        Output << static_cast<char>(Byte);
      }
      else
      {
        Output << "\\x" << HexDigits[Byte >> 4] << HexDigits[Byte & 0x0F];
      }
    }
    Output << '\"';
  }

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
    const auto OutputSink = std::make_shared<spdlog::sinks::ostream_sink_st>(std::cout, true);
    const auto ErrorSink = std::make_shared<spdlog::sinks::ostream_sink_st>(std::cerr, true);
    spdlog::logger OutputLogger("ink-tokenize-output", OutputSink);
    spdlog::logger ErrorLogger("ink-tokenize-error", ErrorSink);
    bool OutputFailed = false;
    bool ErrorFailed = false;
    const auto OutputErrorHandler = [&OutputFailed](const std::string &)
    {
      OutputFailed = true;
    };
    const auto ErrorErrorHandler = [&ErrorFailed](const std::string &)
    {
      ErrorFailed = true;
    };
    OutputLogger.set_error_handler(OutputErrorHandler);
    ErrorLogger.set_error_handler(ErrorErrorHandler);
    OutputLogger.set_formatter(std::make_unique<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, ""));
    ErrorLogger.set_formatter(std::make_unique<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, ""));
    const auto Error = [&ErrorLogger](const std::string &Message)
    {
      ErrorLogger.error("ink-tokenize: error: {}\nTry 'ink-tokenize --help' for more information.\n", Message);
      return 2;
    };
    int OptionTerminator = ArgumentCount;
    for (int Index = 1; Index < ArgumentCount; ++Index)
    {
      const std::string_view Argument(ArgumentValues[Index]);
      if (Argument == "--")
      {
        OptionTerminator = Index;
        break;
      }
      if (Argument.starts_with("--help=") || Argument.starts_with("--h=") || Argument.starts_with("--version=") || Argument.starts_with("--V="))
      {
        return Error("--help and --version do not accept values");
      }
    }

    bool Help = false;
    bool Version = false;
    cxxopts::Options Options("ink-tokenize", "Tokenize Ink source and print the token stream.\nINPUT is a file path, or '-' for standard input (the default).");
    Options.custom_help("[INPUT] [options]");
    Options.allow_unrecognised_options();
    auto AddOption = Options.add_options();
    AddOption("h,help", "Print help", cxxopts::value<bool>(Help));
    AddOption("V,version", "Print version", cxxopts::value<bool>(Version));
    // Only cxxopts' built-in boolean values reach conversion in its no-exception mode.
    const cxxopts::ParseResult Parsed = Options.parse(ArgumentCount, ArgumentValues);
    const auto &Unmatched = Parsed.unmatched();
    if (Unmatched.size() > 1)
    {
      return Error("expected at most one input file");
    }
    if (!Unmatched.empty())
    {
      const std::string &Input = Unmatched.front();
      if (Input.empty())
      {
        return Error("input file path must not be empty");
      }
      bool AfterTerminator = false;
      for (int Index = OptionTerminator; Index < ArgumentCount; ++Index)
      {
        AfterTerminator = AfterTerminator || (Index > OptionTerminator && Input == ArgumentValues[Index]);
      }
      if (Input.front() == '-' && Input != "-" && !AfterTerminator)
      {
        return Error("unexpected argument '" + Input + "'");
      }
    }
    if (Parsed.count("help") > 1 || Parsed.count("version") > 1)
    {
      return Error("options must not be repeated");
    }
    if (Help && Version)
    {
      return Error("option '--help' conflicts with '--version'");
    }
    if (Help)
    {
      OutputLogger.info("{}", Options.help());
      return !OutputFailed && std::cout ? 0 : 2;
    }
    if (Version)
    {
      OutputLogger.info("ink-tokenize development\n");
      return !OutputFailed && std::cout ? 0 : 2;
    }

    const std::string SourceFile = Unmatched.empty() ? "-" : Unmatched.front();
    std::string Source;
    if (SourceFile == "-")
    {
#ifdef _WIN32
      if (_setmode(_fileno(stdin), _O_BINARY) == -1)
      {
        ErrorLogger.error("ink-tokenize: error: cannot read standard input\n");
        return 2;
      }
#endif
      if (!readSource(std::cin, Source))
      {
        ErrorLogger.error("ink-tokenize: error: cannot read standard input\n");
        return 2;
      }
    }
    else
    {
      std::ifstream Input(SourceFile, std::ios::binary);
      if (!Input)
      {
        ErrorLogger.error("ink-tokenize: error: cannot open '{}'\n", SourceFile);
        return 2;
      }
      if (!readSource(Input, Source))
      {
        ErrorLogger.error("ink-tokenize: error: cannot read '{}'\n", SourceFile);
        return 2;
      }
    }

    ink::core::SourceManager Sources;
    ink::core::DiagnosticEngine DiagnosticEngine;
    ink::frontend::FrontendContext Context(Sources, DiagnosticEngine);
    ink::core::CollectingDiagnosticConsumer Diagnostics;
    DiagnosticEngine.addConsumer(Diagnostics);
    const ink::core::SourceId SourceId = Sources.addSource(SourceFile == "-" ? "<stdin>" : SourceFile, std::move(Source));
    const ink::frontend::tokenizer::TokenizedBuffer Result = ink::frontend::tokenizer::tokenizeSource(Context, SourceId);
    std::ostringstream BufferedOutput;
    std::ostringstream BufferedErrorOutput;
    for (const ink::frontend::tokenizer::Token &Token : Result.tokens())
    {
      BufferedOutput << ink::frontend::tokenizer::tokenKindName(Token.Kind) << " [" << Token.Span.getBegin().getByteOffset() << ", " << Token.Span.getEnd().getByteOffset() << ") ";
      printSpelling(BufferedOutput, Result.raw(Token));
      BufferedOutput << '\n';
    }
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
    OutputLogger.info("{}", BufferedOutput.str());
    ErrorLogger.error("{}", BufferedErrorOutput.str());
    if (OutputFailed || ErrorFailed || !std::cout || !std::cerr)
    {
      return 2;
    }
    return HasInternalError ? 3 : Result.succeeded() ? 0 : 1;
  }
} // namespace

int main(int ArgumentCount, char **ArgumentValues)
{
  return runTokenizer(ArgumentCount, ArgumentValues);
}
