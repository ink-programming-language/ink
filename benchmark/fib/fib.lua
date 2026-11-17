local clock_path = assert(os.getenv("FIB_BENCHMARK_CLOCK"), "Set FIB_BENCHMARK_CLOCK to the compiled benchmark_clock module")
local now = assert(package.loadlib(clock_path, "luaopen_benchmark_clock"))()
assert(type(now) == "function", "A monotonic high-resolution clock is required")

local function fib(n)
    if n < 2 then
        return n
    end
    return fib(n - 1) + fib(n - 2)
end

local n = arg[1] == nil and 30 or assert(tonumber(arg[1]), "Invalid n")
local samples = arg[2] == nil and 11 or assert(tonumber(arg[2]), "Invalid samples")
local warmup = arg[3] == nil and 3 or assert(tonumber(arg[3]), "Invalid warmup")
assert(#arg <= 3 and math.type(n) == "integer" and n >= 0 and n <= 40 and math.type(samples) == "integer" and samples >= 1 and samples <= 1000 and math.type(warmup) == "integer" and warmup >= 0 and warmup <= 1000, "Expected n=0..40, samples=1..1000, warmup=0..1000")
local previous, current = 0, 1
for _ = 1, n do
    previous, current = current, previous + current
end
local result = 0
local timings = {}
now()
for index = 1, warmup + samples do
    local start = now()
    result = fib(n)
    local finish = now()
    assert(result == previous, "Incorrect Fibonacci result")
    if index > warmup then
        timings[#timings + 1] = string.format("%.9f", (finish - start) * 1000)
    end
end
io.write(string.format('{"runtime":"Lua","n":%d,"result":%d,"samples_ms":[%s],"warmup":%d}\n', n, result, table.concat(timings, ","), warmup))
