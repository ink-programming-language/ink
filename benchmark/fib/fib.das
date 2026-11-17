options gen2

require daslib/fio
require strings

// Keep recursive work observable to both the interpreter and LLVM JIT.
[sideeffects]
def fib(n : uint64) : uint64
{
    if (n < 2ul)
    {
        return n
    }
    return fib(n - 1ul) + fib(n - 2ul)
}

[export]
def main()
{
    let args <- get_command_line_arguments()
    var separator = -1
    for (index in 0..length(args))
    {
        if (args[index] == "--")
        {
            separator = index
            break
        }
    }
    assert(separator >= 0 && length(args) == separator + 4, "Expected -- n samples warmup")
    let n = uint64(to_int(args[separator + 1]))
    let samples = to_int(args[separator + 2])
    let warmup = to_int(args[separator + 3])
    assert(n <= 40ul && samples >= 1 && samples <= 1000 && warmup >= 0 && warmup <= 1000, "Expected n=0..40, samples=1..1000, warmup=0..1000")
    var previous = 0ul
    var current = 1ul
    for (index in 0..int(n))
    {
        let next = previous + current
        previous = current
        current = next
    }
    var result = 0ul
    var timings : array<double>
    ref_time_ticks()
    for (index in 0..warmup + samples)
    {
        let start = ref_time_ticks()
        result = fib(n)
        let elapsed = get_time_nsec(start)
        assert(result == previous, "Incorrect Fibonacci result")
        if (index >= warmup)
        {
            timings |> push(double(elapsed) / double(1000000))
        }
    }
    print("\{\"runtime\":\"Daslang\",\"n\":{n:d},\"result\":{result:d},\"warmup\":{warmup},\"samples_ms\":[")
    for (index in 0..length(timings))
    {
        if (index != 0)
        {
            print(",")
        }
        print("{timings[index]}")
    }
    print("]\}\n")
}
