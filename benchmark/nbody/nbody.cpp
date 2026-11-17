#include "nbody_reference.h"

#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_sinks.h>

#include <charconv>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

namespace
{
    bool parseUnsigned(std::string_view Text, std::uint64_t& Value)
    {
        const auto Parsed = std::from_chars(Text.data(), Text.data() + Text.size(), Value);
        return Parsed.ec == std::errc{} && Parsed.ptr == Text.data() + Text.size();
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
    if (ArgumentCount > 4 || (ArgumentCount > 1 && !parseUnsigned(ArgumentValues[1], Steps)) || (ArgumentCount > 2 && !parseUnsigned(ArgumentValues[2], SampleCount)) || (ArgumentCount > 3 && !parseUnsigned(ArgumentValues[3], WarmupCount)) || Steps > 100000000 || SampleCount == 0 || SampleCount > 1000 || WarmupCount > 1000)
    {
        Error.error("Usage: cpp_nbody_benchmark [STEPS=100000 (0..100000000)] [SAMPLES=11 (1..1000)] [WARMUP=3 (0..1000)]");
        return 2;
    }
    const auto Expected = ink::benchmark::nbody::reference(Steps);
    const auto ExpectedBodies = ink::benchmark::nbody::bodyWords(Expected.BodyValues);
    // A volatile runtime input prevents constant folding and reuse across timed simulations.
    volatile std::uint64_t Input = Steps;
    std::vector<double> Samples;
    Samples.reserve(static_cast<std::size_t>(SampleCount));
    ink::benchmark::nbody::Result Actual{};
    for (std::uint64_t Index = 0; Index < WarmupCount + SampleCount; ++Index)
    {
        // The same complete simulation as Ink main: initial bodies, momentum, both energies, and all time steps.
        const auto Start = std::chrono::steady_clock::now();
        Actual = ink::benchmark::nbody::reference(Input);
        const auto End = std::chrono::steady_clock::now();
        if (!ink::benchmark::nbody::nearlyEqual(Actual.InitialEnergy, Expected.InitialEnergy) || !ink::benchmark::nbody::nearlyEqual(Actual.FinalEnergy, Expected.FinalEnergy))
        {
            Error.error("Energy mismatch: initial {:.17g} / expected {:.17g}, final {:.17g} / expected {:.17g}", Actual.InitialEnergy, Expected.InitialEnergy, Actual.FinalEnergy, Expected.FinalEnergy);
            return 1;
        }
        const auto ActualBodies = ink::benchmark::nbody::bodyWords(Actual.BodyValues);
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
            Samples.push_back(std::chrono::duration<double, std::milli>(End - Start).count());
        }
    }
    std::string Timings;
    for (const auto Value : Samples)
    {
        Timings += fmt::format("{}{:.9f}", Timings.empty() ? "" : ",", Value);
    }
    std::string BodyValues;
    for (const auto Value : ink::benchmark::nbody::bodyWords(Actual.BodyValues))
    {
        BodyValues += fmt::format("{}{:.17g}", BodyValues.empty() ? "" : ",", Value);
    }
    Output.info("{{\"runtime\":\"C++\",\"steps\":{},\"initial_energy\":{:.17g},\"final_energy\":{:.17g},\"bodies\":[{}],\"samples_ms\":[{}],\"warmup\":{}}}", Steps, Actual.InitialEnergy, Actual.FinalEnergy, BodyValues, Timings, WarmupCount);
    return 0;
}
