using System;

internal static class Fibonacci
{
    private static ulong Fib(int n)
    {
        if (n < 2)
        {
            return (ulong)n;
        }
        return Fib(n - 1) + Fib(n - 2);
    }

    private static ulong ReferenceFib(int n)
    {
        ulong previous = 0;
        ulong current = 1;
        for (int index = 0; index < n; ++index)
        {
            ulong next = previous + current;
            previous = current;
            current = next;
        }
        return previous;
    }

    private static int Main(string[] args)
    {
        if (!Benchmark.ReadArguments(args, 30, 40, out int n, out int samples, out int warmup))
        {
            Console.Error.WriteLine("Usage: fib.exe [N=30 (0..40)] [SAMPLES=11 (1..1000)] [WARMUP=3 (0..1000)]");
            return 2;
        }

        ulong expected = ReferenceFib(n);
        if (!Benchmark.Measure(n, samples, warmup, Fib, value => value == expected, out ulong result, out double[] timings))
        {
            Console.Error.WriteLine("fib({0}) failed: expected {1}, got {2}", n, expected, result);
            return 1;
        }

        Console.WriteLine("{{\"runtime\":\"C#\",\"n\":{0},\"result\":{1},\"samples_ms\":[{2}],\"warmup\":{3}}}", n, result, Benchmark.Numbers(timings), warmup);
        return 0;
    }
}
