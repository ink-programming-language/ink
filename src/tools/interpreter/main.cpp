#include "ink/backend/vm/executable/executable_file.h"
#include "ink/backend/vm/executable_emitter/executable_emitter.h"
#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/backend/vm/serialization/machine_module_text.h"
#include "ink/core/config/config_manager.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_engine.h"
#include "ink/core/diagnostic/diagnostic_formatter.h"
#include "ink/core/name/name_pool.h"

#include <llvm/Support/CommandLine.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/MemoryBuffer.h>
#include <llvm/Support/raw_ostream.h>
#include <spdlog/logger.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/ostream_sink.h>

#include <array>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace cl = llvm::cl;

namespace
{
    static cl::opt<std::string> InputFile(cl::desc("<input module>"), cl::Positional, cl::init("-"));
    // String flags keep validation errors out of LLVM's direct stderr output paths.
    static cl::opt<std::string> Help("h", cl::desc("Print help"), cl::ValueOptional);
    static cl::opt<std::string> Version("v", cl::desc("Print version"), cl::ValueOptional);
    // Defer aliases until LLVM's built-in options have been removed.
    static cl::alias HelpAlias("help", cl::aliasopt(Help), cl::DefaultOption);
    static cl::alias VersionAlias("version", cl::aliasopt(Version), cl::DefaultOption);

    static spdlog::logger Output("ink-interpreter-output", std::make_shared<spdlog::sinks::ostream_sink_st>(std::cout, true));
    static spdlog::logger ErrorOutput("ink-interpreter-error", std::make_shared<spdlog::sinks::ostream_sink_st>(std::cerr, true));

    // An exit code handles help, version, or errors; no value continues to input loading.
    std::optional<int> parseArgument(int ArgumentCount, char** ArgumentValues)
    {
        Output.set_formatter(std::make_unique<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, ""));
        ErrorOutput.set_formatter(std::make_unique<spdlog::pattern_formatter>("%v", spdlog::pattern_time_type::local, ""));

        // Replace LLVM's common options with the file-scope interpreter options.
        cl::getRegisteredOptions();
        cl::ResetCommandLineParser();
        InputFile.addArgument();
        Help.addArgument();
        Version.addArgument();
        HelpAlias.addArgument();
        VersionAlias.addArgument();

        std::string ParseErrors;
        llvm::raw_string_ostream Errors(ParseErrors);
        if (!cl::ParseCommandLineOptions(ArgumentCount, ArgumentValues, "Ink VM interpreter.\n", &Errors))
        {
            ErrorOutput.error("{}", ParseErrors);
            return 2;
        }
        const int HelpCount = Help.getNumOccurrences() + HelpAlias.getNumOccurrences();
        const int VersionCount = Version.getNumOccurrences() + VersionAlias.getNumOccurrences();

        if (HelpCount != 0)
        {
            Output.info("Ink VM interpreter.\nUsage:\n  ink-interpreter [options] [<input module>]\n\n  <input module>  MachineModule assembly text (default: -, standard input)\n  -h, --help      Print help\n  -v, --version   Print version\n");
            return std::cout ? 0 : 2;
        }
        if (VersionCount != 0)
        {
            Output.info("ink-interpreter {}\n", ink::core::ConfigManager::getVersion<ink::core::ConfigKind::InkVersion>().toString());
            return std::cout ? 0 : 2;
        }

        return std::nullopt;
    }

    std::optional<ink::vm::ExecutableFile> prepareInputBitCode(const ink::core::DiagnosticEngine& Diagnostics)
    {
        const auto Input = llvm::MemoryBuffer::getFileOrSTDIN(InputFile.getValue(), false, false);
        if (!Input)
        {
            ErrorOutput.error("ink-interpreter: error: cannot read '{}': {}\n", InputFile.getValue(), Input.getError().message());
            return std::nullopt;
        }
        const auto Text = (*Input)->getBuffer();
        auto Module = ink::vm::readMachineModuleText(std::string_view(Text.data(), Text.size()));
        if (!Module)
        {
            Diagnostics.report(Module.error());
            return std::nullopt;
        }
        const auto EntryName = ink::core::NamePool::instance().intern("main");
        const std::array ExportNames{EntryName};
        ink::vm::ExecutableEmitter Emitter(Diagnostics);
        return Emitter.emit(**Module, EntryName, ExportNames);
    }
} // namespace

int main(int ArgumentCount, char** ArgumentValues)
{
    llvm::InitLLVM LLVM(ArgumentCount, ArgumentValues);
    if (const auto ExitCode = parseArgument(ArgumentCount, ArgumentValues))
    {
        return *ExitCode;
    }

    ink::core::CollectingDiagnosticConsumer Diagnostics;
    ink::vm::Interpreter Interpreter;
    Interpreter.getDiagnostics().addConsumer(Diagnostics);
    const auto ReportDiagnostics = [&Diagnostics]()
    {
        bool HasErrors = false;
        for (const ink::core::Diagnostic& Diagnostic : Diagnostics.takeDiagnostics())
        {
            const auto Formatted = ink::core::DiagnosticFormatter{}.format(Diagnostic);
            HasErrors = HasErrors || Formatted.Severity == ink::core::DiagnosticSeverity::Error;
            ErrorOutput.error("ink-interpreter: {}[{}]: {}\n", ink::core::diagnosticSeverityName(Formatted.Severity), Diagnostic.code(), Formatted.Message);
            for (const auto& Note : Formatted.Notes)
            {
                ErrorOutput.error("ink-interpreter: note: {}\n", Note.Message);
            }
        }
        return HasErrors;
    };

    auto Executable = prepareInputBitCode(Interpreter.getDiagnostics());
    const bool PreparationErrors = ReportDiagnostics();
    if (!Executable || PreparationErrors)
    {
        return 2;
    }

    const auto Handle = Interpreter.loadLibrary(std::move(*Executable));
    const bool LoadErrors = ReportDiagnostics();
    if (Handle == ink::vm::Interpreter::InvalidLibraryHandle || LoadErrors)
    {
        if (!LoadErrors)
        {
            ErrorOutput.error("ink-interpreter: error: cannot load '{}'\n", InputFile.getValue());
        }
        return 2;
    }
    const auto& Entry = Interpreter.resolveSymbolAddress(ink::core::NamePool::instance().intern("main"));
    const bool Succeeded = Interpreter.execute(Entry);
    const bool ExecutionErrors = ReportDiagnostics();
    return Succeeded && !ExecutionErrors ? 0 : 1;
}
