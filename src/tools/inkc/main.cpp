#include "ink/cli/application.h"
#include "ink/cli/io.h"
#include "ink/execution/engine/execution_engine.h"
#include "ink/execution/support/execution_diagnostic.h"
#include "ink/ir/function/function.h"
#include "ink/parser/parser.h"
#include "ink/semantic/analyzer/analyzer.h"
#include "ink/semantic/context.h"
#include "ink/semantic/name_resolve/name_resolver.h"
#include "ink/backend/llvm/llvm_backend.h"
#include <llvm/IR/LLVMContext.h>
#include "bytecode_commands.h"
#include "source_modules.h"

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
  struct NativeOptions
  {
      std::string IR;
      std::string Object;
      ink::backend::llvm::BackendOptions Backend;
  };

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
    if (Function.isNativeImport())
    {
      Error = Entry + " cannot be a native import";
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

  int processSource(const std::string &InputFile, std::string Source, const std::string &EntryName, const std::string &ModuleRoot, const ink::tools::BytecodeOptions *Bytecode, const NativeOptions *Native)
  {
    ink::core::CompilationContext Compilation;
    ink::core::FrontendContext Frontend(Compilation);
    ink::core::CollectingDiagnosticConsumer Diagnostics;
    Compilation.diagnosticEngine().addConsumer(Diagnostics);
    ink::tools::SourceModules Sources(Frontend);
    std::string LoadError;
    if (!Sources.load(InputFile, std::move(Source), ModuleRoot, Bytecode != nullptr, LoadError))
    {
      return reportError(LoadError, ink::cli::ExitCode::SourceError);
    }
    // Every dependency's syntax tree outlives the semantic context borrowing its declarations.
    ink::semantic::SemanticContext Context(Compilation);
    ink::ir::Module *Module = nullptr;
    if (Sources.succeeded())
    {
      const auto Inputs = Sources.inputs();
      Module = ink::semantic::Analyzer{}.analyzeModules(Context, Inputs, Sources.entryName());
    }
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
    if (Bytecode)
    {
      return ink::tools::emitBytecode(Context, *Module, *Bytecode);
    }
    std::string Error;
    const ink::ir::Function *Entry = findEntry(Context, *Module, EntryName, Error);
    if (!Entry)
    {
      return reportError(Error, ink::cli::ExitCode::InvocationError);
    }
    if (Native)
    {
      llvm::LLVMContext LLVMContext;
      auto Lowered = ink::backend::llvm::lowerToLLVMIR(LLVMContext, *Module, Entry, Native->Backend);
      if (!Lowered.succeeded())
      {
        return reportError(Lowered.error(), ink::cli::ExitCode::SourceError);
      }
      const bool Written = Native->IR.empty() ? Lowered.writeObject(Native->Object, Error) : Lowered.writeIR(Native->IR, Error);
      return Written ? 0 : reportError(Error, ink::cli::ExitCode::InvocationError);
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
    std::vector<std::string> LinkInputs;
    std::string LinkOutput;
    std::string ModuleRoot;
    ink::tools::BytecodeOptions Bytecode;
    bool Interpret = false;
    bool RunBytecode = false;
    NativeOptions Native;
    std::string OptimizationLevel;
    Command.addOption("-i,--input", InputFile, "Input source or bytecode file; '-' reads source from standard input").typeName("FILE");
    ink::cli::Option &InterpretOption = Command.addFlag("--interpret", Interpret, "Interpret the source and execute its entry function");
    ink::cli::Option &EmitOption = Command.addOption("--emit-bytecode", Bytecode.Output, "Compile all runtime functions to a bytecode object file").typeName("FILE").excludes(InterpretOption);
    ink::cli::Option &LinkOption = Command.addOption("--link-bytecode", LinkInputs, "Link a bytecode object file; repeat for each input").repeatPolicy(ink::cli::RepeatPolicy::Append).typeName("FILE").excludes(InterpretOption).excludes(EmitOption);
    ink::cli::Option &RunOption = Command.addFlag("--run-bytecode", RunBytecode, "Load a linked bytecode file and execute its saved entry").excludes(InterpretOption).excludes(EmitOption).excludes(LinkOption);
    ink::cli::Option &LLVMOption = Command.addOption("--emit-llvm", Native.IR, "Lower source to LLVM IR with a native entry wrapper").typeName("FILE").excludes(InterpretOption).excludes(EmitOption).excludes(LinkOption).excludes(RunOption);
    ink::cli::Option &ObjectOption = Command.addOption("--emit-object", Native.Object, "Compile a native object; link it with ink_aot_runtime").typeName("FILE").excludes(InterpretOption).excludes(EmitOption).excludes(LinkOption).excludes(RunOption).excludes(LLVMOption);
    Command.addOption("--opt-level", OptimizationLevel, "LLVM optimization level: 0, 1, 2 or 3 (default: 0)").typeName("LEVEL");
    Command.addOption("-o,--output", LinkOutput, "Output linked bytecode executable").typeName("FILE");
    Command.addOption("--module-root", ModuleRoot, "Root directory for source imports and module identities (default: input directory)").typeName("DIRECTORY");
    Command.addOption("--entry", EntryName, "Entry function name (default: main)").typeName("NAME");
    Command.addOption("-oir", IrOutputFile, "Reserved legacy IR output option").typeName("FILE").excludes(InterpretOption).excludes(EmitOption).excludes(LinkOption).excludes(LLVMOption).excludes(ObjectOption);
    const ink::cli::ParseResult ParsedArguments = Command.parse(ArgumentCount, ArgumentValues);
    if (ParsedArguments.ShouldExit)
    {
      return ink::cli::exitStatus(ParsedArguments.Code);
    }
    const bool EmitNative = !Native.IR.empty() || !Native.Object.empty();
    if (!OptimizationLevel.empty())
    {
      if (!EmitNative || OptimizationLevel.size() != 1 || OptimizationLevel[0] < '0' || OptimizationLevel[0] > '3')
      {
        return reportError("--opt-level requires --emit-llvm or --emit-object and a level from 0 to 3", ink::cli::ExitCode::InvocationError);
      }
      Native.Backend.OptimizationLevel = static_cast<unsigned>(OptimizationLevel[0] - '0');
    }
    if (!Interpret && Bytecode.Output.empty() && LinkInputs.empty() && !RunBytecode && !EmitNative)
    {
      return reportError("select --interpret, --emit-bytecode, --link-bytecode, --run-bytecode, --emit-llvm or --emit-object", ink::cli::ExitCode::InvocationError);
    }
    if (!ModuleRoot.empty() && (RunBytecode || !LinkInputs.empty()))
    {
      return reportError("--module-root requires a source compilation or interpretation mode", ink::cli::ExitCode::InvocationError);
    }
    if (!LinkInputs.empty())
    {
      if (LinkOutput.empty() || !InputFile.empty())
      {
        return reportError("--link-bytecode requires --output and uses its own input files", ink::cli::ExitCode::InvocationError);
      }
      return ink::tools::linkBytecode(LinkInputs, LinkOutput, EntryName);
    }
    if (InputFile.empty() || !LinkOutput.empty() || (!IrOutputFile.empty() && RunBytecode))
    {
      return reportError("this mode requires --input and does not accept --output or -oir", ink::cli::ExitCode::InvocationError);
    }
    if (RunBytecode)
    {
      return ink::tools::runBytecode(InputFile);
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
    return processSource(InputFile, std::move(Source), EntryName, ModuleRoot, Bytecode.Output.empty() ? nullptr : &Bytecode, EmitNative ? &Native : nullptr);
  }
} // namespace

int main(int ArgumentCount, char **ArgumentValues)
{
  const auto Run = [ArgumentCount, ArgumentValues]()
  {
    return runCompiler(ArgumentCount, ArgumentValues);
  };
  const int Result = ink::cli::runMain("inkc", Run);
  // Native imports can buffer output without touching the CLI streams during bytecode execution.
  // Flush through the shared output sink while the host runtime is still active.
  const bool OutputWritten = ink::cli::writeOutput(std::cout, "");
  const bool ErrorWritten = ink::cli::writeOutput(std::cerr, "");
  return OutputWritten && ErrorWritten ? Result : ink::cli::exitStatus(ink::cli::ExitCode::InvocationError);
}
