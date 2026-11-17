#include "nbody_reference.h"

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

    bool writeTrace(const char* Path, const std::vector<Sample>& Samples, std::uint64_t Steps)
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
            Trace << fmt::format("{}{{\"name\":\"Interpreter::execute\",\"cat\":\"execute\",\"ph\":\"X\",\"pid\":1,\"tid\":1,\"ts\":{:.3f},\"dur\":{:.3f},\"args\":{{\"steps\":{},\"sample\":{}}}}}", Index == 0 ? "" : ",", Timestamp, Duration, Steps, Index);
        }
        Trace << "]}";
        Trace.close();
        return !Trace.fail();
    }
} // namespace

int main(int ArgumentCount, char** ArgumentValues)
{
    spdlog::logger Output("nbody-output", std::make_shared<spdlog::sinks::stdout_sink_st>());
    spdlog::logger Error("nbody-error", std::make_shared<spdlog::sinks::stderr_sink_st>());
    Output.set_pattern("%v");
    Error.set_pattern("%v");
    std::uint64_t Steps = 100000;
    std::uint64_t SampleCount = 11;
    std::uint64_t WarmupCount = 3;
    if (ArgumentCount < 2 || ArgumentCount > 6 || (ArgumentCount > 2 && !parseUnsigned(ArgumentValues[2], Steps)) || (ArgumentCount > 3 && !parseUnsigned(ArgumentValues[3], SampleCount)) || (ArgumentCount > 4 && !parseUnsigned(ArgumentValues[4], WarmupCount)) || Steps > 100000000 || SampleCount == 0 || SampleCount > 1000 || WarmupCount > 1000)
    {
        Error.error("Usage: ink_nbody_benchmark FILE.asm [STEPS=100000 (0..100000000)] [SAMPLES=11 (1..1000)] [WARMUP=3 (0..1000)] [TRACE.json]");
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
    const auto StepsName = Names.intern("nbody_steps");
    const auto InitialName = Names.intern("nbody_initial_energy");
    const auto FinalName = Names.intern("nbody_final_energy");
    const auto BodiesName = Names.intern("nbody_bodies");
    const std::array ExportNames{EntryName, StepsName, InitialName, FinalName, BodiesName};
    ink::vm::ExecutableEmitter Emitter(VM.getDiagnostics());
    auto Executable = Emitter.emit(**Module, EntryName, ExportNames);
    if (reportDiagnostics(Diagnostics, Error))
    {
        return 1;
    }
    const auto Handle = VM.loadLibrary(std::move(Executable));
    const bool LoadDiagnosticsFailed = reportDiagnostics(Diagnostics, Error);
    if (Handle == ink::vm::Interpreter::InvalidLibraryHandle || LoadDiagnosticsFailed)
    {
        return 1;
    }
    const auto* Entry = VM.resolveSymbolAddress(EntryName);
    auto* StepsAddress = VM.resolveSymbolAddress(StepsName);
    const auto* InitialAddress = VM.resolveSymbolAddress(InitialName);
    const auto* FinalAddress = VM.resolveSymbolAddress(FinalName);
    const auto* BodiesAddress = VM.resolveSymbolAddress(BodiesName);
    if (!Entry || !StepsAddress || !InitialAddress || !FinalAddress || !BodiesAddress)
    {
        Error.error("The module must export main, nbody_steps, nbody_initial_energy, nbody_final_energy and nbody_bodies");
        return 1;
    }
    std::memcpy(StepsAddress, &Steps, sizeof(Steps));
    // The reference simulation, parsing, emission, loading and validation are outside the execution interval.
    const auto Expected = ink::benchmark::nbody::reference(Steps);
    const auto ExpectedBodies = ink::benchmark::nbody::bodyWords(Expected.BodyValues);
    std::vector<Sample> Samples;
    Samples.reserve(static_cast<std::size_t>(SampleCount));
    double InitialEnergy = 0;
    double FinalEnergy = 0;
    ink::benchmark::nbody::BodyWords ActualBodies{};
    for (std::uint64_t Index = 0; Index < WarmupCount + SampleCount; ++Index)
    {
        // main resets initial bodies, offsets momentum, computes both energies, and advances the simulation.
        const auto Start = Clock::now();
        const bool Succeeded = VM.execute(Entry);
        const auto End = Clock::now();
        const bool ExecutionDiagnosticsFailed = reportDiagnostics(Diagnostics, Error);
        if (!Succeeded || ExecutionDiagnosticsFailed)
        {
            return 1;
        }
        std::memcpy(&InitialEnergy, InitialAddress, sizeof(InitialEnergy));
        std::memcpy(&FinalEnergy, FinalAddress, sizeof(FinalEnergy));
        std::memcpy(ActualBodies.data(), BodiesAddress, sizeof(ActualBodies));
        if (!ink::benchmark::nbody::nearlyEqual(InitialEnergy, Expected.InitialEnergy) || !ink::benchmark::nbody::nearlyEqual(FinalEnergy, Expected.FinalEnergy))
        {
            Error.error("Energy mismatch: initial {:.17g} / expected {:.17g}, final {:.17g} / expected {:.17g}", InitialEnergy, Expected.InitialEnergy, FinalEnergy, Expected.FinalEnergy);
            return 1;
        }
        for (std::size_t WordIndex = 0; WordIndex < ActualBodies.size(); ++WordIndex)
        {
            if (!ink::benchmark::nbody::nearlyEqual(ActualBodies[WordIndex], ExpectedBodies[WordIndex]))
            {
                Error.error("Body state mismatch at word {}: {:.17g} / expected {:.17g}", WordIndex, ActualBodies[WordIndex], ExpectedBodies[WordIndex]);
                return 1;
            }
        }
        if (Index >= WarmupCount)
        {
            Samples.push_back({Start, End});
        }
    }
    if (ArgumentCount == 6 && !writeTrace(ArgumentValues[5], Samples, Steps))
    {
        Error.error("Cannot write trace {}", ArgumentValues[5]);
        return 2;
    }
    std::string Timings;
    for (const auto& Value : Samples)
    {
        Timings += fmt::format("{}{:.9f}", Timings.empty() ? "" : ",", std::chrono::duration<double, std::milli>(Value.End - Value.Start).count());
    }
    std::string BodyValues;
    for (const auto Value : ActualBodies)
    {
        BodyValues += fmt::format("{}{:.17g}", BodyValues.empty() ? "" : ",", Value);
    }
    Output.info("{{\"runtime\":\"Ink\",\"steps\":{},\"initial_energy\":{:.17g},\"final_energy\":{:.17g},\"bodies\":[{}],\"samples_ms\":[{}],\"warmup\":{},\"build_mode\":\"{}\",\"checks_enabled\":{},\"compiler\":\"{} {}\",\"dispatch\":\"switch\"}}", Steps, InitialEnergy, FinalEnergy, BodyValues, Timings, WarmupCount, INK_BUILD_MODE_NAME, INK_ENABLE_ASSERTIONS ? "true" : "false", INK_COMPILER_ID, INK_COMPILER_VERSION);
    return 0;
}
