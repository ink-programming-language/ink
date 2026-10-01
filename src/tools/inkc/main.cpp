#include "ink/cli/application.h"
#include "ink/cli/io.h"
#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/support/execution_diagnostic.h"
#include "ink/ir/function/function.h"
#include "ink/parser/parser.h"
#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/name_resolver.h"

#include <array>
#include <bit>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace
{
  int reportError(std::string_view Message, ink::cli::ExitCode Code)
  {
    const std::string Prefix = Code == ink::cli::ExitCode::InternalError ? "inkc: internal error: " : "inkc: error: ";
    if (!ink::cli::writeOutput(std::cerr, Prefix + std::string(Message) + "\n"))
    {
      return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
    }
    return ink::cli::exitStatus(Code);
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

  bool readInput(const std::string &InputFile, std::string &Source, std::string &Error)
  {
    if (InputFile == "-")
    {
      if (!ink::cli::useBinaryStandardInput() || !readSource(std::cin, Source))
      {
        Error = "cannot read standard input";
        return false;
      }
      return true;
    }
    std::filesystem::path SourcePath;
    if (!ink::cli::pathFromUtf8(InputFile, SourcePath))
    {
      Error = "input path is not valid UTF-8";
      return false;
    }
    std::ifstream Input(SourcePath, std::ios::binary);
    if (!Input)
    {
      Error = "cannot open '" + InputFile + "'";
      return false;
    }
    if (!readSource(Input, Source))
    {
      Error = "cannot read '" + InputFile + "'";
      return false;
    }
    return true;
  }

  bool writeDiagnostics(const ink::core::SourceManager &Sources, const std::vector<ink::core::Diagnostic> &Diagnostics, bool &HasInternalError)
  {
    std::ostringstream Output;
    const ink::core::DiagnosticFormatter Formatter;
    for (const ink::core::Diagnostic &Diagnostic : Diagnostics)
    {
      const bool Internal = Diagnostic.classification() == ink::core::DiagnosticClass::InternalCompilerError;
      HasInternalError = HasInternalError || Internal;
      const ink::core::FormattedDiagnostic Formatted = Formatter.format(Diagnostic);
      const std::shared_ptr<const ink::core::SourceBuffer> Source = Sources.findSource(Diagnostic.Source);
      if (Source && Diagnostic.Span.isValid())
      {
        Output << Source->name() << ':' << Source->lineNumber(Diagnostic.Span.getBegin().getByteOffset()) << ": ";
      }
      Output << (Internal ? ink::core::diagnosticClassName(Diagnostic.classification()) : ink::core::diagnosticSeverityName(Formatted.Severity)) << '[' << Diagnostic.code() << "]: " << Formatted.Message;
      if (Diagnostic.Span.isValid())
      {
        Output << " [" << Diagnostic.Span.getBegin().getByteOffset() << ", " << Diagnostic.Span.getEnd().getByteOffset() << ")";
      }
      Output << '\n';
      for (const ink::core::FormattedDiagnosticNote &Note : Formatted.Notes)
      {
        Output << "note: " << Note.Message;
        if (Note.Span)
        {
          Output << " [" << Note.Span->getBegin().getByteOffset() << ", " << Note.Span->getEnd().getByteOffset() << ")";
        }
        Output << '\n';
      }
    }
    return ink::cli::writeOutput(std::cerr, Output.str());
  }

  const ink::ir::Function *findEntry(ink::semantic::SemanticContext &Context, ink::ir::Module &Module, const std::string &EntryName, std::string &Error)
  {
    const auto *Binding = ink::semantic::NameResolver(Context).lookupMember(Module, Context.namePool().find(EntryName));
    const std::string Entry = "entry '" + EntryName + "'";
    if (!Binding || Binding->targets().empty())
    {
      Error = Entry + " was not found";
      return nullptr;
    }
    if (Binding->targets().size() != 1)
    {
      Error = Entry + " is ambiguous";
      return nullptr;
    }
    const ink::ir::Value *Target = Binding->targets().front();
    if (!ink::ir::Function::classof(Target))
    {
      Error = Entry + " is not a function";
      return nullptr;
    }
    const auto &Function = static_cast<const ink::ir::Function &>(*Target);
    if (Function.languageLinkage() != ink::ir::LanguageLinkage::Ink)
    {
      Error = Entry + " must use Ink language linkage";
      return nullptr;
    }
    if (!Function.parameters().empty())
    {
      Error = Entry + " must take no arguments";
      return nullptr;
    }
    const ink::ir::Type &ReturnType = Function.functionType().returnType();
    const bool ReturnsInt32 = ReturnType.typeKind() == ink::ir::TypeKind::Integer && static_cast<const ink::ir::IntegerType &>(ReturnType).bitWidth() == 32 && static_cast<const ink::ir::IntegerType &>(ReturnType).isSigned();
    if (ReturnType.typeKind() != ink::ir::TypeKind::Void && !ReturnsInt32)
    {
      Error = Entry + " must return void or i32";
      return nullptr;
    }
    const auto Definition = Context.comptimeState().Functions.find(&Function);
    if (Definition != Context.comptimeState().Functions.end() && Definition->second.Comptime)
    {
      Error = Entry + " cannot be a comptime function";
      return nullptr;
    }
    if (!Function.hasBody())
    {
      Error = Entry + " requires a runtime body";
      return nullptr;
    }
    return &Function;
  }

  int executeEntry(ink::semantic::SemanticContext &Context, const ink::ir::Function &Entry, const std::string &EntryName)
  {
    ink::execution::ExecutionEngine Engine(Context.irContext());
    const ink::execution::ExecutionValueResult Result = Engine.execute(Entry);
    if (!Result)
    {
      ink::core::SourceId Source;
      ink::core::SourceRange Span;
      const auto Definition = Context.comptimeState().Functions.find(&Entry);
      if (Definition != Context.comptimeState().Functions.end())
      {
        Source = Definition->second.Source;
        Span = Definition->second.NameRange;
      }
      if (const auto Diagnostic = ink::execution::makeExecutionDiagnostic(Result.Status, Source, Span, "execution of entry '" + EntryName + "' failed"))
      {
        Context.compilationContext().diagnosticEngine().report(*Diagnostic);
      }
      return ink::cli::exitStatus(ink::cli::ExitCode::SourceError);
    }
    if (!Result.Value.valid() || Result.Value.type() != &Entry.functionType().returnType())
    {
      return reportError("interpreter returned an invalid entry result", ink::cli::ExitCode::InternalError);
    }
    if (Result.Value.kind() == ink::execution::ExecutionValueKind::Void)
    {
      return ink::cli::exitStatus(ink::cli::ExitCode::Success);
    }
    const std::uint32_t Bits = static_cast<std::uint32_t>(Result.Value.integer().bits().words().front());
    return static_cast<int>(std::bit_cast<std::int32_t>(Bits));
  }

  int interpretSource(const std::string &InputFile, std::string Source, const std::string &EntryName)
  {
    ink::core::CompilationContext Compilation;
    ink::core::FrontendContext Frontend(Compilation);
    ink::core::CollectingDiagnosticConsumer Diagnostics;
    Compilation.diagnosticEngine().addConsumer(Diagnostics);
    const ink::core::SourceId SourceId = Compilation.sourceManager().addSource(InputFile == "-" ? "<stdin>" : InputFile, std::move(Source));
    const ink::parser::ParseResult Parsed = ink::parser::parse(Frontend, ink::tokenizer::tokenizeSource(Frontend, SourceId));
    // The syntax tree outlives the semantic context that borrows its declarations.
    ink::semantic::SemanticContext Context(Compilation);
    ink::ir::Module *Module = Parsed.succeeded() ? ink::semantic::Analyzer{}.analyze(Context, Parsed) : nullptr;
    bool HasInternalError = false;
    if (!writeDiagnostics(Compilation.sourceManager(), Diagnostics.diagnostics(), HasInternalError))
    {
      return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
    }
    if (HasInternalError)
    {
      return ink::cli::exitStatus(ink::cli::ExitCode::InternalError);
    }
    if (!Module)
    {
      if (Diagnostics.diagnostics().empty())
      {
        return reportError("source analysis failed without a diagnostic", ink::cli::ExitCode::InternalError);
      }
      return ink::cli::exitStatus(ink::cli::ExitCode::SourceError);
    }
    std::string Error;
    const ink::ir::Function *Entry = findEntry(Context, *Module, EntryName, Error);
    if (!Entry)
    {
      return reportError(Error, ink::cli::ExitCode::InvocationError);
    }
    Diagnostics.clear();
    const int ExitStatus = executeEntry(Context, *Entry, EntryName);
    if (!writeDiagnostics(Compilation.sourceManager(), Diagnostics.diagnostics(), HasInternalError))
    {
      return ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
    }
    return ExitStatus;
  }

  int runCompiler(int ArgumentCount, char **ArgumentValues)
  {
    ink::cli::Application Command({"inkc", "Compile or interpret Ink source files.", "development"});
    std::string InputFile;
    std::string IrOutputFile;
    std::string EntryName = "main";
    bool Interpret = false;
    Command.addOption("-i,--input", InputFile, "Input Ink source file, or '-' for standard input").required().typeName("FILE");
    ink::cli::Option &InterpretOption = Command.addFlag("--interpret", Interpret, "Interpret the source and execute its entry function");
    Command.addOption("--entry", EntryName, "Entry function name (default: main)").typeName("NAME");
    Command.addOption("-oir", IrOutputFile, "Output IR file (not supported in interpretation mode)").typeName("FILE").excludes(InterpretOption);
    const ink::cli::ParseResult ParsedArguments = Command.parse(ArgumentCount, ArgumentValues);
    if (ParsedArguments.ShouldExit)
    {
      return ink::cli::exitStatus(ParsedArguments.Code);
    }
    if (!Interpret)
    {
      return reportError("compilation mode is not supported; use --interpret", ink::cli::ExitCode::InvocationError);
    }
    if (EntryName.empty())
    {
      return reportError("entry name must not be empty", ink::cli::ExitCode::InvocationError);
    }
    std::string Source;
    std::string Error;
    if (!readInput(InputFile, Source, Error))
    {
      return reportError(Error, ink::cli::ExitCode::InvocationError);
    }
    return interpretSource(InputFile, std::move(Source), EntryName);
  }
} // namespace

int main(int ArgumentCount, char **ArgumentValues)
{
  const auto Run = [ArgumentCount, ArgumentValues]()
  {
    return runCompiler(ArgumentCount, ArgumentValues);
  };
  return ink::cli::runMain("inkc", Run);
}
