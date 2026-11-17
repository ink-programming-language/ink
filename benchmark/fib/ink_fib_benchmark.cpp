#include "ink/backend/vm/executable_emitter/executable_emitter.h"
#include "ink/backend/vm/interpreter/interpreter.h"
#include "ink/backend/vm/serialization/machine_module_text.h"
#include "ink/core/diagnostic/diagnostic_consumer.h"
#include "ink/core/diagnostic/diagnostic_formatter.h"
#include "ink/core/name/name_pool.h"
#include "ink/core/support/core_define.h"

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_sinks.h>

#include <array>
#include <charconv>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace
{
    using Clock = std::chrono::steady_clock;

    struct Sample
    {
            Clock::time_point Start;
            Clock::time_point End;
    };

    bool parseUnsigned(std::string_view Text, std::uint64_t& Value)
    {
        const auto Parsed = std::from_chars(Text.data(), Text.data() + Text.size(), Value);
        return Parsed.ec == std::errc{} && Parsed.ptr == Text.data() + Text.size();
    }

    std::uint64_t referenceFib(std::uint64_t N)
    {
        std::uint64_t Previous = 0;
        std::uint64_t Current = 1;
        for (std::uint64_t Index = 0; Index < N; ++Index)
        {
            const auto Next = Previous + Current;
            Previous = Current;
            Current = Next;
        }
        return Previous;
    }

    bool reportDiagnostics(ink::core::CollectingDiagnosticConsumer& Diagnostics, spdlog::logger& Error)
    {
        bool Failed = false;
        for (const auto& Diagnostic : Diagnostics.takeDiagnostics())
        {
            const auto Formatted = ink::core::DiagnosticFormatter{}.format(Diagnostic);
            Error.error("{}[{}]: {}", ink::core::diagnosticSeverityName(Formatted.Severity), Diagnostic.code(), Formatted.Message);
            Failed = Failed || Formatted.Severity == ink::core::DiagnosticSeverity::Error;
        }
        return Failed;
    }

    bool writeTrace(const char* Path, const std::vector<Sample>& Samples, std::uint64_t N)
    {
        std::ofstream Trace(Path, std::ios::binary);
        if (!Trace)
        {
            return false;
        }
        Trace << "{\"displayTimeUnit\":\"ms\",\"traceEvents\":[";
        for (std::size_t Index = 0; Index < Samples.size(); ++Index)
        {
            const auto& Value = Samples[Index];
            const auto Timestamp = std::chrono::duration<double, std::micro>(Value.Start - Samples.front().Start).count();
            const auto Duration = std::chrono::duration<double, std::micro>(Value.End - Value.Start).count();
            Trace << fmt::format("{}{{\"name\":\"Interpreter::execute\",\"cat\":\"execute\",\"ph\":\"X\",\"pid\":1,\"tid\":1,\"ts\":{:.3f},\"dur\":{:.3f},\"args\":{{\"n\":{},\"sample\":{}}}}}", Index == 0 ? "" : ",", Timestamp, Duration, N, Index);
        }
        Trace << "]}";
        Trace.close();
        return !Trace.fail();
    }
} // namespace

int main(int ArgumentCount, char** ArgumentValues)
{
    spdlog::logger Output("fib-output", std::make_shared<spdlog::sinks::stdout_sink_st>());
    spdlog::logger Error("fib-error", std::make_shared<spdlog::sinks::stderr_sink_st>());
    Output.set_pattern("%v");
    Error.set_pattern("%v");
    std::uint64_t N = 30;
    std::uint64_t SampleCount = 11;
    std::uint64_t WarmupCount = 3;
    if (ArgumentCount < 2 || ArgumentCount > 6 || (ArgumentCount > 2 && !parseUnsigned(ArgumentValues[2], N)) || (ArgumentCount > 3 && !parseUnsigned(ArgumentValues[3], SampleCount)) || (ArgumentCount > 4 && !parseUnsigned(ArgumentValues[4], WarmupCount)) || N > 40 || SampleCount == 0 || SampleCount > 1000 || WarmupCount > 1000)
    {
        Error.error("Usage: ink_fib_benchmark FILE.asm [N=30 (0..40)] [SAMPLES=11 (1..1000)] [WARMUP=3 (0..1000)] [TRACE.json]");
        return 2;
    }

    std::ifstream Input(ArgumentValues[1], std::ios::binary);
    if (!Input)
    {
        Error.error("Cannot read {}", ArgumentValues[1]);
        return 2;
    }
    const std::string Text{std::istreambuf_iterator<char>(Input), std::istreambuf_iterator<char>()};
    if (Input.bad())
    {
        Error.error("Cannot finish reading {}", ArgumentValues[1]);
        return 2;
    }
    ink::core::CollectingDiagnosticConsumer Diagnostics;
    ink::vm::Interpreter VM;
    VM.getDiagnostics().addConsumer(Diagnostics);
    auto Module = ink::vm::readMachineModuleText(Text);
    if (!Module)
    {
        VM.getDiagnostics().report(Module.error());
        reportDiagnostics(Diagnostics, Error);
        return 1;
    }
    auto& Names = ink::core::NamePool::instance();
    const auto EntryName = Names.intern("main");
    const auto InputName = Names.intern("fib_input");
    const auto ResultName = Names.intern("fib_result");
    const std::array ExportNames{EntryName, InputName, ResultName};
    ink::vm::ExecutableEmitter Emitter(VM.getDiagnostics());
    auto Executable = Emitter.emit(**Module, EntryName, ExportNames);
    if (reportDiagnostics(Diagnostics, Error))
    {
        return 1;
    }
    if (VM.loadLibrary(std::move(Executable)) == ink::vm::Interpreter::InvalidLibraryHandle || reportDiagnostics(Diagnostics, Error))
    {
        return 1;
    }
    const auto* Entry = VM.resolveSymbolAddress(EntryName);
    auto* InputAddress = VM.resolveSymbolAddress(InputName);
    auto* ResultAddress = VM.resolveSymbolAddress(ResultName);
    if (!Entry || !InputAddress || !ResultAddress)
    {
        Error.error("The module must export main, fib_input and fib_result");
        return 1;
    }
    const auto Expected = referenceFib(N);
    std::memcpy(InputAddress, &N, sizeof(N));
    std::vector<Sample> Samples;
    Samples.reserve(static_cast<std::size_t>(SampleCount));
    std::uint64_t Result = 0;
    for (std::uint64_t Index = 0; Index < WarmupCount + SampleCount; ++Index)
    {
        const auto Sentinel = std::numeric_limits<std::uint64_t>::max();
        std::memcpy(ResultAddress, &Sentinel, sizeof(Sentinel));
        // Input parsing, emission, loading, symbol lookup and result validation are outside this interval.
        const auto Start = Clock::now();
        const bool Succeeded = VM.execute(Entry);
        const auto End = Clock::now();
        std::memcpy(&Result, ResultAddress, sizeof(Result));
        const bool FailedDiagnostics = reportDiagnostics(Diagnostics, Error);
        if (!Succeeded || FailedDiagnostics || Result != Expected)
        {
            Error.error("fib({}) failed: expected {}, got {}", N, Expected, Result);
            return 1;
        }
        if (Index >= WarmupCount)
        {
            Samples.push_back({Start, End});
        }
    }
    if (ArgumentCount == 6 && !writeTrace(ArgumentValues[5], Samples, N))
    {
        Error.error("Cannot write trace {}", ArgumentValues[5]);
        return 2;
    }
    std::string Timings;
    for (const auto& Value : Samples)
    {
        Timings += fmt::format("{}{:.9f}", Timings.empty() ? "" : ",", std::chrono::duration<double, std::milli>(Value.End - Value.Start).count());
    }
#if defined(INK_VM_USE_COMPUTED_GOTO) && INK_VM_USE_COMPUTED_GOTO
    constexpr std::string_view DispatchName = "computed-goto";
#else
    constexpr std::string_view DispatchName = "switch";
#endif
    Output.info("{{\"runtime\":\"Ink\",\"n\":{},\"result\":{},\"samples_ms\":[{}],\"warmup\":{},\"build_mode\":\"{}\",\"checks_enabled\":{},\"compiler\":\"{} {}\",\"dispatch\":\"{}\"}}", N, Result, Timings, WarmupCount, INK_BUILD_MODE_NAME, INK_ENABLE_ASSERTIONS ? "true" : "false", INK_COMPILER_ID, INK_COMPILER_VERSION, DispatchName);
    return 0;
}
