#include "ink/cli/application.h"
#include "ink/cli/io.h"
#include "ink/core/diagnostic.h"
#include "ink/parser/parser.h"
#include "ink/tokenizer/tokenizer.h"

#include <array>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace
{
    bool readSource(std::istream& Input, std::string& Source)
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

    void printIndent(std::ostream& Output, std::size_t Depth)
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

    void pushPrintFrame(const ink::parser::CstTree& Tree, ink::parser::CstNodeId Id, std::size_t Depth, std::size_t NodeStart, std::vector<bool>& ActiveNodes, std::vector<PrintFrame>& Frames, std::ostream& Output)
    {
        if (Id >= ActiveNodes.size())
        {
            throw std::out_of_range("CST node reference is out of range");
        }
        if (ActiveNodes[Id])
        {
            throw std::logic_error("CST contains a node-reference cycle");
        }
        ActiveNodes[Id] = true;

        const ink::parser::CstNode& Node = Tree.node(Id);
        printIndent(Output, Depth);
        Output << "Node " << Id << ' ' << ink::parser::cstKindName(Node.Kind) << " tokens=" << Node.TokenCount << " text=" << Node.TextLength << " flags=" << static_cast<unsigned int>(Node.Flags) << '\n';

        const std::vector<ink::parser::CstElement>& Children = Tree.children();
        if (Node.FirstChild > Children.size() || Node.ChildCount > Children.size() - Node.FirstChild)
        {
            throw std::out_of_range("CST child range is out of bounds");
        }
        Frames.push_back({Id, Depth, NodeStart});
    }

    void printNode(const ink::parser::CstTree& Tree, const ink::tokenizer::TokenizedBuffer& LexedFile, ink::parser::CstNodeId Id, std::size_t NodeStart, std::vector<bool>& ActiveNodes, std::ostream& Output)
    {
        std::vector<PrintFrame> Frames;
        pushPrintFrame(Tree, Id, 0, NodeStart, ActiveNodes, Frames, Output);
        while (!Frames.empty())
        {
            PrintFrame& Frame = Frames.back();
            const ink::parser::CstNode& Node = Tree.node(Frame.Id);
            if (Frame.NextChild == Node.ChildCount)
            {
                ActiveNodes[Frame.Id] = false;
                Frames.pop_back();
                continue;
            }

            const ink::parser::CstElement& Element = Tree.children()[Node.FirstChild + Frame.NextChild];
            ++Frame.NextChild;
            if (const ink::parser::CstNodeRef* NodeRef = std::get_if<ink::parser::CstNodeRef>(&Element))
            {
                if (NodeRef->Id >= Tree.nodes().size())
                {
                    throw std::out_of_range("CST node reference is out of range");
                }
                const std::size_t ChildStart = Frame.NodeStart + Frame.ConsumedTokens;
                Frame.ConsumedTokens += Tree.node(NodeRef->Id).TokenCount;
                pushPrintFrame(Tree, NodeRef->Id, Frame.Depth + 1, ChildStart, ActiveNodes, Frames, Output);
                continue;
            }
            if (const ink::parser::CstTokenRef* TokenRef = std::get_if<ink::parser::CstTokenRef>(&Element))
            {
                const std::size_t TokenIndex = Frame.NodeStart + TokenRef->TokenOffset;
                const ink::tokenizer::Token& Token = LexedFile.tokens().at(TokenIndex);
                printIndent(Output, Frame.Depth + 1);
                Output << "Token " << TokenIndex << ' ' << ink::tokenizer::tokenKindName(Token.Kind) << " [" << Token.Span.Start << ", " << Token.Span.End << ")\n";
                ++Frame.ConsumedTokens;
                continue;
            }

            const ink::parser::MissingToken& Missing = std::get<ink::parser::MissingToken>(Element);
            printIndent(Output, Frame.Depth + 1);
            Output << "Missing " << ink::tokenizer::tokenKindName(Missing.ExpectedKind);
            if (!Missing.ExpectedSpelling.empty())
            {
                Output << ' ' << std::quoted(Missing.ExpectedSpelling);
            }
            Output << " [" << Missing.AnchorByteOffset << ", " << Missing.AnchorByteOffset << ")\n";
        }
    }

    void printCst(const ink::parser::ParsedFile& Result, std::ostream& Output)
    {
        const ink::parser::CstTree& Tree = Result.cst();
        std::vector<bool> ActiveNodes(Tree.nodes().size(), false);
        printNode(Tree, Result.lexedFile(), Tree.root(), 0, ActiveNodes, Output);
    }

    void printDiagnostics(const std::vector<ink::core::Diagnostic>& Diagnostics, std::ostream& ErrorOutput)
    {
        const ink::core::DiagnosticFormatter Formatter;
        for (const ink::core::Diagnostic& Diagnostic : Diagnostics)
        {
            const ink::core::FormattedDiagnostic Formatted = Formatter.format(Diagnostic);
            ErrorOutput << ink::core::diagnosticSeverityName(Formatted.Severity) << '[' << Diagnostic.code() << "]: " << Formatted.Message << " [" << Diagnostic.Span.Start << ", " << Diagnostic.Span.End << ")\n";
            for (const ink::core::FormattedDiagnosticNote& Note : Formatted.Notes)
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

    int runParser(int ArgumentCount, char** ArgumentValues)
    {
        ink::cli::Application Command({"ink-parse", "Parse Ink source and print the concrete syntax tree.", "development"});
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
                ink::cli::writeOutput(std::cerr, "ink-parse: error: cannot read standard input\n");
                return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
            }
        }
        else
        {
            std::ifstream Input(ink::cli::pathFromUtf8(SourceFile), std::ios::binary);
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

        ink::tokenizer::TokenizedBuffer LexedFile = ink::tokenizer::tokenize(std::move(Source));
        if (!LexedFile.succeeded())
        {
            std::ostringstream BufferedErrorOutput;
            printDiagnostics(LexedFile.diagnostics(), BufferedErrorOutput);
            const bool ErrorOutputSucceeded = ink::cli::writeOutput(std::cerr, BufferedErrorOutput.str());
            return ink::cli::exitStatus(ErrorOutputSucceeded ? ink::cli::ExitCode::SourceError : ink::cli::ExitCode::InvocationError);
        }

        const ink::parser::ParsedFile Result = ink::parser::parse(std::move(LexedFile));
        std::ostringstream BufferedOutput;
        std::ostringstream BufferedErrorOutput;
        printCst(Result, BufferedOutput);
        printDiagnostics(Result.diagnostics(), BufferedErrorOutput);
        const bool OutputSucceeded = ink::cli::writeOutput(std::cout, BufferedOutput.str());
        const bool ErrorOutputSucceeded = ink::cli::writeOutput(std::cerr, BufferedErrorOutput.str());
        if (!OutputSucceeded || !ErrorOutputSucceeded)
        {
            return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
        }
        return ink::cli::exitStatus(Result.succeeded() ? ink::cli::ExitCode::Success : ink::cli::ExitCode::SourceError);
    }
} // namespace

int main(int ArgumentCount, char** ArgumentValues)
{
    return ink::cli::runMain("ink-parse", [ArgumentCount, ArgumentValues]()
    {
        return runParser(ArgumentCount, ArgumentValues);
    });
}
