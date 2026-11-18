using System;
using System.Diagnostics;
using System.Globalization;

internal static class Benchmark
{
    private static bool ReadArgument(string[] args, int index, int defaultValue, int minimum, int maximum, out int value)
    {
        value = defaultValue;
        return index >= args.Length || (int.TryParse(args[index], NumberStyles.None, CultureInfo.InvariantCulture, out value) && value >= minimum && value <= maximum);
    }

    internal static bool ReadArguments(string[] args, int defaultInput, int maximumInput, out int input, out int samples, out int warmup)
    {
        bool validInput = ReadArgument(args, 0, defaultInput, 0, maximumInput, out input);
        bool validSamples = ReadArgument(args, 1, 11, 1, 1000, out samples);
        bool validWarmup = ReadArgument(args, 2, 3, 0, 1000, out warmup);
        return args.Length <= 3 && validInput && validSamples && validWarmup;
    }

    internal static bool Measure<T>(int input, int samples, int warmup, Func<int, T> execute, Predicate<T> isCorrect, out T result, out double[] timings)
    {
        result = default(T);
        timings = new double[samples];
        for (int index = 0; index < warmup + samples; ++index)
        {
            long start = Stopwatch.GetTimestamp();
            result = execute(input);
            long end = Stopwatch.GetTimestamp();

            if (!isCorrect(result))
            {
                return false;
            }
            if (index >= warmup)
            {
                timings[index - warmup] = (end - start) * (1000.0 / Stopwatch.Frequency);
            }
        }
        return true;
    }

    internal static string Number(double value)
    {
        return value.ToString("G17", CultureInfo.InvariantCulture);
    }

    internal static string Numbers(double[] values)
    {
        return string.Join(",", Array.ConvertAll(values, Number));
    }
}
