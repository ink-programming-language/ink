"""Time only the naive recursive Fibonacci function using a monotonic clock."""

import json
import sys
from time import perf_counter_ns


def fib(n: int) -> int:
    if n < 2:
        return n
    return fib(n - 1) + fib(n - 2)


def main() -> int:
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 30
    samples = int(sys.argv[2]) if len(sys.argv) > 2 else 11
    warmup = int(sys.argv[3]) if len(sys.argv) > 3 else 3
    if (
        len(sys.argv) > 4
        or not 0 <= n <= 40
        or not 1 <= samples <= 1000
        or not 0 <= warmup <= 1000
    ):
        raise ValueError("Expected n=0..40, samples=1..1000, warmup=0..1000")
    previous, current = 0, 1
    for _ in range(n):
        previous, current = current, previous + current
    timings = []
    result = 0
    for index in range(warmup + samples):
        start = perf_counter_ns()
        result = fib(n)
        end = perf_counter_ns()
        if result != previous:
            raise ValueError(f"Expected {previous}, got {result}")
        if index >= warmup:
            timings.append((end - start) / 1_000_000)
    print(json.dumps({
        "runtime": "Python",
        "n": n,
        "result": result,
        "samples_ms": timings,
        "warmup": warmup,
    }))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
