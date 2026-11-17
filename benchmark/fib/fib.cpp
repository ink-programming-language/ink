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
    std::uint64_t fib(std::uint64_t N)
    {
        if (N < 2)
        {
            return N;
        }
        return fib(N - 1) + fib(N - 2);
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

    bool parseUnsigned(std::string_view Text, std::uint64_t& Value)
    {
        const auto Parsed = std::from_chars(Text.data(), Text.data() + Text.size(), Value);
        return Parsed.ec == std::errc{} && Parsed.ptr == Text.data() + Text.size();
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
    if (ArgumentCount > 4 || (ArgumentCount > 1 && !parseUnsigned(ArgumentValues[1], N)) || (ArgumentCount > 2 && !parseUnsigned(ArgumentValues[2], SampleCount)) || (ArgumentCount > 3 && !parseUnsigned(ArgumentValues[3], WarmupCount)) || N > 40 || SampleCount == 0 || SampleCount > 1000 || WarmupCount > 1000)
    {
        Error.error("Usage: cpp_fib_benchmark [N=30 (0..40)] [SAMPLES=11 (1..1000)] [WARMUP=3 (0..1000)]");
        return 2;
    }
    const auto Expected = referenceFib(N);
    // A volatile runtime input prevents constant folding and reuse across timed calls.
    volatile std::uint64_t Input = N;
    std::vector<double> Samples;
    Samples.reserve(static_cast<std::size_t>(SampleCount));
    std::uint64_t Result = 0;
    for (std::uint64_t Index = 0; Index < WarmupCount + SampleCount; ++Index)
    {
        const auto Start = std::chrono::steady_clock::now();
        Result = fib(Input);
        const auto End = std::chrono::steady_clock::now();
        if (Result != Expected)
        {
            Error.error("fib({}) failed: expected {}, got {}", N, Expected, Result);
            return 1;
        }
        if (Index >= WarmupCount)
        {
            Samples.push_back(std::chrono::duration<double, std::milli>(End - Start).count());
        }
    }
    std::string Timings;
    for (const auto Sample : Samples)
    {
        Timings += fmt::format("{}{:.9f}", Timings.empty() ? "" : ",", Sample);
    }
    Output.info("{{\"runtime\":\"C++\",\"n\":{},\"result\":{},\"samples_ms\":[{}],\"warmup\":{}}}", N, Result, Timings, WarmupCount);
    return 0;
}
