#include "ink/cli/application.h"
#include "ink/cli/io.h"
#include "ink/core/diagnostic.h"
#include "ink/parser/parser.h"
#include "ink/tokenizer/tokenizer.h"

#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
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

  void printRange(std::ostream &Output, ink::core::SourceRange Span)
  {
    Output << '[' << Span.Start << ", " << Span.End << ')';
  }

  bool printTokenField(const ink::tokenizer::TokenizedBuffer &LexedFile, const char *Name, ink::parser::AstTokenId Id, std::ostream &Output)
  {
    if (Id == ink::parser::InvalidAstTokenId)
    {
      return true;
    }
    if (Id >= LexedFile.tokens().size())
    {
      return false;
    }
    Output << ' ' << Name << '=';
    printSpelling(Output, LexedFile.raw(LexedFile.tokens()[Id]));
    return true;
  }

  const char *accessName(ink::parser::AccessKind Access)
  {
    switch (Access)
    {
    case ink::parser::AccessKind::Unspecified:
      return "Unspecified";
    case ink::parser::AccessKind::Public:
      return "Public";
    case ink::parser::AccessKind::Private:
      return "Private";
    }
    return "Unknown";
  }

  const char *bindingName(ink::parser::BindingKind Binding)
  {
    switch (Binding)
    {
    case ink::parser::BindingKind::Let:
      return "Let";
    case ink::parser::BindingKind::Var:
      return "Var";
    case ink::parser::BindingKind::Const:
      return "Const";
    }
    return "Unknown";
  }

  void printArguments(const std::vector<ink::parser::AstArgument> &Arguments, std::ostream &Output)
  {
    for (std::size_t Index = 0; Index < Arguments.size(); ++Index)
    {
      const ink::parser::AstArgument &Argument = Arguments[Index];
      Output << " Argument[" << Index << "]={Pack=" << (Argument.IsPackExpansion ? "true" : "false") << " Span=";
      printRange(Output, Argument.Span);
      Output << '}';
    }
  }

  bool printPayload(const ink::parser::AstNode &Node, const ink::tokenizer::TokenizedBuffer &LexedFile, std::ostream &Output)
  {
    const auto Print = [&LexedFile, &Output](const auto &Data) -> bool
    {
      using NodeType = std::decay_t<decltype(Data)>;
      if constexpr (std::is_same_v<NodeType, ink::parser::Error>)
      {
        Output << " Expected=";
        printSpelling(Output, Data.Expected);
      }
      else if constexpr (std::is_same_v<NodeType, ink::parser::ImportDeclaration>)
      {
        Output << " MemberImport=" << (Data.IsMemberImport ? "true" : "false") << " Package=[";
        for (std::size_t Index = 0; Index < Data.Package.size(); ++Index)
        {
          if (Index != 0)
          {
            Output << ", ";
          }
          if (Data.Package[Index] == ink::parser::InvalidAstTokenId)
          {
            Output << "<missing>";
            continue;
          }
          if (Data.Package[Index] >= LexedFile.tokens().size())
          {
            return false;
          }
          printSpelling(Output, LexedFile.raw(LexedFile.tokens()[Data.Package[Index]]));
        }
        Output << ']';
        return printTokenField(LexedFile, "Member", Data.Member, Output) && printTokenField(LexedFile, "Alias", Data.Alias, Output);
      }
      else if constexpr (std::is_same_v<NodeType, ink::parser::FunctionDeclaration>)
      {
        Output << " Access=" << accessName(Data.Access) << " Const=" << (Data.IsConst ? "true" : "false") << " ClassMethod=" << (Data.IsClassMethod ? "true" : "false");
        return printTokenField(LexedFile, "Name", Data.Name, Output) && printTokenField(LexedFile, "Linkage", Data.Linkage, Output);
      }
      else if constexpr (std::is_same_v<NodeType, ink::parser::ClassDeclaration> || std::is_same_v<NodeType, ink::parser::InterfaceDeclaration> || std::is_same_v<NodeType, ink::parser::EnumDeclaration>)
      {
        Output << " Access=" << accessName(Data.Access);
        return printTokenField(LexedFile, "Name", Data.Name, Output);
      }
      else if constexpr (std::is_same_v<NodeType, ink::parser::ClassFieldDeclaration>)
      {
        Output << " Access=" << accessName(Data.Access) << " Const=" << (Data.IsConst ? "true" : "false");
        return printTokenField(LexedFile, "Name", Data.Name, Output);
      }
      else if constexpr (std::is_same_v<NodeType, ink::parser::EnumFieldDeclaration> || std::is_same_v<NodeType, ink::parser::NamePattern> || std::is_same_v<NodeType, ink::parser::NameExpression>)
      {
        return printTokenField(LexedFile, "Name", Data.Name, Output);
      }
      else if constexpr (std::is_same_v<NodeType, ink::parser::BindingDeclaration>)
      {
        Output << " Access=" << accessName(Data.Access) << " Binding=" << bindingName(Data.Binding);
      }
      else if constexpr (std::is_same_v<NodeType, ink::parser::GenericParameter>)
      {
        Output << " Variadic=" << (Data.IsVariadic ? "true" : "false");
        return printTokenField(LexedFile, "Name", Data.Name, Output);
      }
      else if constexpr (std::is_same_v<NodeType, ink::parser::FunctionParameter>)
      {
        Output << " Variadic=" << (Data.IsVariadic ? "true" : "false");
      }
      else if constexpr (std::is_same_v<NodeType, ink::parser::ForInStatement>)
      {
        Output << " Binding=" << bindingName(Data.Binding);
      }
      else if constexpr (std::is_same_v<NodeType, ink::parser::AssignmentStatement> || std::is_same_v<NodeType, ink::parser::BinaryExpression> || std::is_same_v<NodeType, ink::parser::UnaryExpression>)
      {
        Output << " Operator=";
        printSpelling(Output, ink::tokenizer::symbolSpelling(Data.Operator));
      }
      else if constexpr (std::is_same_v<NodeType, ink::parser::ReferenceTypeExpression> || std::is_same_v<NodeType, ink::parser::PointerTypeExpression>)
      {
        Output << " Const=" << (Data.IsConst ? "true" : "false");
      }
      else if constexpr (std::is_same_v<NodeType, ink::parser::FunctionTypeExpression>)
      {
        if (!printTokenField(LexedFile, "Linkage", Data.Linkage, Output))
        {
          return false;
        }
        for (std::size_t Index = 0; Index < Data.Parameters.size(); ++Index)
        {
          const ink::parser::FunctionTypeParameter &Parameter = Data.Parameters[Index];
          Output << " Parameter[" << Index << "]={Variadic=" << (Parameter.IsVariadic ? "true" : "false") << " Bare=" << (Parameter.TypeExpression == ink::parser::InvalidAstNodeId ? "true" : "false") << " Span=";
          printRange(Output, Parameter.Span);
          Output << '}';
        }
      }
      else if constexpr (std::is_same_v<NodeType, ink::parser::CallExpression> || std::is_same_v<NodeType, ink::parser::GenericInstantiationExpression>)
      {
        printArguments(Data.Arguments, Output);
      }
      else if constexpr (std::is_same_v<NodeType, ink::parser::MemberExpression>)
      {
        Output << " Pointer=" << (Data.IsPointer ? "true" : "false");
        return printTokenField(LexedFile, "Member", Data.Member, Output);
      }
      else if constexpr (std::is_same_v<NodeType, ink::parser::LiteralExpression>)
      {
        if (Data.Token >= LexedFile.tokens().size() || !printTokenField(LexedFile, "Literal", Data.Token, Output))
        {
          return false;
        }
        const ink::tokenizer::Token &Token = LexedFile.tokens()[Data.Token];
        Output << " Kind=" << ink::tokenizer::tokenKindName(Token.Kind);
        if (const ink::tokenizer::NumericInfo *Numeric = std::get_if<ink::tokenizer::NumericInfo>(&Token.Payload))
        {
          Output << " Base=" << Numeric->Base;
        }
        if (const ink::tokenizer::StringInfo *String = std::get_if<ink::tokenizer::StringInfo>(&Token.Payload))
        {
          Output << " StringMode=" << static_cast<unsigned int>(String->Mode) << " Decoded=";
          printSpelling(Output, String->Decoded);
        }
      }
      else if constexpr (std::is_same_v<NodeType, ink::parser::BuiltinTypeExpression>)
      {
        Output << " Type=";
        printSpelling(Output, ink::tokenizer::keywordSpelling(Data.Type));
      }
      else if constexpr (std::is_same_v<NodeType, ink::parser::ReceiverExpression>)
      {
        Output << " Receiver=";
        printSpelling(Output, ink::tokenizer::keywordSpelling(Data.Receiver));
      }
      return true;
    };
    return ink::parser::visitAstNode(Node, Print);
  }

  struct PrintFrame
  {
      ink::parser::AstNodeId Id;
      std::size_t Depth;
      std::vector<ink::parser::AstChildEdge> Children;
      std::size_t NextChild = 0;
  };

  bool pushPrintFrame(const ink::parser::AstTree &Tree, const ink::tokenizer::TokenizedBuffer &LexedFile, ink::parser::AstChildEdge Edge, std::size_t Depth, std::vector<bool> &ActiveNodes, std::vector<PrintFrame> &Frames, std::ostream &Output)
  {
    if (Edge.Id >= ActiveNodes.size() || ActiveNodes[Edge.Id])
    {
      return false;
    }
    const ink::parser::AstNode &Node = Tree.node(Edge.Id);
    if (Node.Span.Start > Node.Span.End || Node.Span.End > LexedFile.source().size())
    {
      return false;
    }
    printIndent(Output, Depth);
    Output << Edge.Role;
    if (Edge.Index != ink::parser::InvalidAstNodeId)
    {
      Output << '[' << Edge.Index << ']';
    }
    Output << ": Node " << Edge.Id << ' ' << ink::parser::astKindName(Node.kind()) << ' ';
    printRange(Output, Node.Span);
    Output << " error=" << (ink::parser::hasFlag(Node.Flags, ink::parser::AstNodeFlags::HasError) ? "true" : "false") << " missing=" << (ink::parser::hasFlag(Node.Flags, ink::parser::AstNodeFlags::HasMissing) ? "true" : "false");
    if (!printPayload(Node, LexedFile, Output))
    {
      return false;
    }
    Output << '\n';
    ActiveNodes[Edge.Id] = true;
    Frames.push_back({Edge.Id, Depth, Tree.children(Edge.Id)});
    return true;
  }

  bool printAst(const ink::parser::ParsedFile &Result, std::ostream &Output)
  {
    const ink::parser::AstTree &Tree = Result.ast();
    std::vector<bool> ActiveNodes(Tree.size(), false);
    std::vector<PrintFrame> Frames;
    if (!pushPrintFrame(Tree, Result.lexedFile(), {"Root", Tree.root()}, 0, ActiveNodes, Frames, Output))
    {
      return false;
    }
    while (!Frames.empty())
    {
      PrintFrame &Frame = Frames.back();
      if (Frame.NextChild == Frame.Children.size())
      {
        ActiveNodes[Frame.Id] = false;
        Frames.pop_back();
        continue;
      }
      const ink::parser::AstChildEdge Edge = Frame.Children[Frame.NextChild++];
      if (!pushPrintFrame(Tree, Result.lexedFile(), Edge, Frame.Depth + 1, ActiveNodes, Frames, Output))
      {
        return false;
      }
    }
    return true;
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
    ink::tokenizer::TokenizedBuffer LexedFile = ink::tokenizer::tokenizeSource(Context, SourceId);
    if (!LexedFile.succeeded())
    {
      std::ostringstream BufferedErrorOutput;
      printDiagnostics(Compilation.sourceManager(), Diagnostics.diagnostics(), BufferedErrorOutput);
      const bool ErrorOutputSucceeded = ink::cli::writeOutput(std::cerr, BufferedErrorOutput.str());
      bool InternalError = false;
      for (const ink::core::Diagnostic &Diagnostic : Diagnostics.diagnostics())
      {
        InternalError = InternalError || Diagnostic.classification() == ink::core::DiagnosticClass::InternalCompilerError;
      }
      return ink::cli::exitStatus(ErrorOutputSucceeded ? InternalError ? ink::cli::ExitCode::InternalError : ink::cli::ExitCode::SourceError : ink::cli::ExitCode::InvocationError);
    }

    const ink::parser::ParsedFile Result = ink::parser::parse(Context, std::move(LexedFile));
    std::ostringstream BufferedOutput;
    std::ostringstream BufferedErrorOutput;
    if (!printAst(Result, BufferedOutput))
    {
      ink::cli::writeOutput(std::cerr, "ink-parse: internal compiler error: abstract syntax tree cannot be traversed\n");
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
  const auto Run = [ArgumentCount, ArgumentValues]()
  {
    return runParser(ArgumentCount, ArgumentValues);
  };
  return ink::cli::runMain("ink-parse", Run);
}
