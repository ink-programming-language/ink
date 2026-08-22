#include "ink/cli/application.h"
#include "ink/cli/io.h"
#include "ink/core/diagnostic.h"
#include "ink/parser/parser.h"
#include "ink/tokenizer/tokenizer.h"

#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <variant>
#include <vector>

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

  void printIndent(std::ostream &Output, std::size_t Depth)
  {
    for (std::size_t Index = 0; Index < Depth; ++Index)
    {
      Output << "  ";
    }
  }

  struct PrintFrame
  {
      ink::parser::CstNodeId Id;
      std::size_t Depth;
      std::size_t NodeStart;
      std::size_t NextChild = 0;
      std::size_t ConsumedTokens = 0;
  };

  bool pushPrintFrame(const ink::parser::CstTree &Tree, ink::parser::CstNodeId Id, std::size_t Depth, std::size_t NodeStart, std::vector<bool> &ActiveNodes, std::vector<PrintFrame> &Frames, std::ostream &Output)
  {
    if (Id >= ActiveNodes.size())
    {
      return false;
    }
    if (ActiveNodes[Id])
    {
      return false;
    }
    ActiveNodes[Id] = true;

    const ink::parser::CstNode &Node = Tree.nodes()[Id];
    printIndent(Output, Depth);
    Output << "Node " << Id << ' ' << ink::parser::cstKindName(Node.Kind) << " tokens=" << Node.TokenCount << " text=" << Node.TextLength << " flags=" << static_cast<unsigned int>(Node.Flags) << '\n';

    const std::vector<ink::parser::CstElement> &Children = Tree.children();
    if (Node.FirstChild > Children.size() || Node.ChildCount > Children.size() - Node.FirstChild)
    {
      ActiveNodes[Id] = false;
      return false;
    }
    Frames.push_back({Id, Depth, NodeStart});
    return true;
  }

  bool printNode(const ink::parser::CstTree &Tree, const ink::tokenizer::TokenizedBuffer &LexedFile, ink::parser::CstNodeId Id, std::size_t NodeStart, std::vector<bool> &ActiveNodes, std::ostream &Output)
  {
    std::vector<PrintFrame> Frames;
    if (!pushPrintFrame(Tree, Id, 0, NodeStart, ActiveNodes, Frames, Output))
    {
      return false;
    }
    while (!Frames.empty())
    {
      PrintFrame &Frame = Frames.back();
      if (Frame.Id >= Tree.nodes().size())
      {
        return false;
      }
      const ink::parser::CstNode &Node = Tree.nodes()[Frame.Id];
      if (Frame.NextChild == Node.ChildCount)
      {
        ActiveNodes[Frame.Id] = false;
        Frames.pop_back();
        continue;
      }

      const ink::parser::CstElement &Element = Tree.children()[Node.FirstChild + Frame.NextChild];
      ++Frame.NextChild;
      if (const ink::parser::CstNodeRef *NodeRef = std::get_if<ink::parser::CstNodeRef>(&Element))
      {
        if (NodeRef->Id >= Tree.nodes().size())
        {
          return false;
        }
        const std::size_t ChildStart = Frame.NodeStart + Frame.ConsumedTokens;
        Frame.ConsumedTokens += Tree.nodes()[NodeRef->Id].TokenCount;
        if (!pushPrintFrame(Tree, NodeRef->Id, Frame.Depth + 1, ChildStart, ActiveNodes, Frames, Output))
        {
          return false;
        }
        continue;
      }
      if (const ink::parser::CstTokenRef *TokenRef = std::get_if<ink::parser::CstTokenRef>(&Element))
      {
        const std::size_t TokenIndex = Frame.NodeStart + TokenRef->TokenOffset;
        if (TokenIndex >= LexedFile.tokens().size())
        {
          return false;
        }
        const ink::tokenizer::Token &Token = LexedFile.tokens()[TokenIndex];
        printIndent(Output, Frame.Depth + 1);
        Output << "Token " << TokenIndex << ' ' << ink::tokenizer::tokenKindName(Token.Kind) << " [" << Token.Span.Start << ", " << Token.Span.End << ")\n";
        ++Frame.ConsumedTokens;
        continue;
      }

      const ink::parser::MissingToken *Missing = std::get_if<ink::parser::MissingToken>(&Element);
      if (Missing == nullptr)
      {
        return false;
      }
      printIndent(Output, Frame.Depth + 1);
      Output << "Missing " << ink::tokenizer::tokenKindName(Missing->ExpectedKind);
      if (!Missing->ExpectedSpelling.empty())
      {
        Output << ' ' << std::quoted(Missing->ExpectedSpelling);
      }
      Output << " [" << Missing->AnchorByteOffset << ", " << Missing->AnchorByteOffset << ")\n";
    }
    return true;
  }

  bool printCst(const ink::parser::ParsedFile &Result, std::ostream &Output)
  {
    const ink::parser::CstTree &Tree = Result.cst();
    if (Tree.root() >= Tree.nodes().size())
    {
      return false;
    }
    std::vector<bool> ActiveNodes(Tree.nodes().size(), false);
    return printNode(Tree, Result.lexedFile(), Tree.root(), 0, ActiveNodes, Output);
  }

  void printDiagnostics(const ink::core::SourceManager &Sources, const std::vector<ink::core::Diagnostic> &Diagnostics, std::ostream &ErrorOutput)
  {
    const ink::core::DiagnosticFormatter Formatter;
    for (const ink::core::Diagnostic &Diagnostic : Diagnostics)
    {
      const ink::core::FormattedDiagnostic Formatted = Formatter.format(Diagnostic);
      const std::shared_ptr<const ink::core::SourceBuffer> DiagnosticSource = Sources.findSource(Diagnostic.Source);
      if (DiagnosticSource != nullptr)
      {
        ErrorOutput << DiagnosticSource->name() << ':' << DiagnosticSource->lineNumber(Diagnostic.Span.Start) << ": ";
      }
      ErrorOutput << (Diagnostic.classification() == ink::core::DiagnosticClass::InternalCompilerError ? ink::core::diagnosticClassName(Diagnostic.classification()) : ink::core::diagnosticSeverityName(Formatted.Severity)) << '[' << Diagnostic.code() << "]: " << Formatted.Message << " [" << Diagnostic.Span.Start << ", " << Diagnostic.Span.End << ")\n";
      for (const ink::core::FormattedDiagnosticNote &Note : Formatted.Notes)
      {
        ErrorOutput << "note: " << Note.Message;
        if (Note.Span)
        {
          ErrorOutput << " [" << Note.Span->Start << ", " << Note.Span->End << ")";
        }
        ErrorOutput << '\n';
      }
    }
  }

  int runParser(int ArgumentCount, char **ArgumentValues)
  {
    ink::cli::Application Command({"ink-parse", "Parse Ink source and print the concrete syntax tree.", "development"});
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
    ink::tokenizer::TokenizedBuffer LexedFile = ink::tokenizer::tokenizeSource(Context, SourceId);
    if (!LexedFile.succeeded())
    {
      std::ostringstream BufferedErrorOutput;
      printDiagnostics(Compilation.sourceManager(), Diagnostics.diagnostics(), BufferedErrorOutput);
      const bool ErrorOutputSucceeded = ink::cli::writeOutput(std::cerr, BufferedErrorOutput.str());
      return ink::cli::exitStatus(ErrorOutputSucceeded ? ink::cli::ExitCode::SourceError : ink::cli::ExitCode::InvocationError);
    }

    const ink::parser::ParsedFile Result = ink::parser::parse(Context, std::move(LexedFile));
    std::ostringstream BufferedOutput;
    std::ostringstream BufferedErrorOutput;
    if (!printCst(Result, BufferedOutput))
    {
      ink::cli::writeOutput(std::cerr, "ink-parse: internal compiler error: concrete syntax tree cannot be traversed\n");
      return ink::cli::exitStatus(ink::cli::ExitCode::InternalError);
    }
    printDiagnostics(Compilation.sourceManager(), Diagnostics.diagnostics(), BufferedErrorOutput);
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
  return ink::cli::runMain("ink-parse", [ArgumentCount, ArgumentValues]()
                           {
                             return runParser(ArgumentCount, ArgumentValues);
                           });
}
