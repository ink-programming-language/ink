-- Initial conditions and algorithm follow Christoph Bauer's Benchmarks Game
-- n-body program. See LICENSE.benchmarksgame for the redistribution notice.
local clock_path = assert(os.getenv("NBODY_BENCHMARK_CLOCK"), "Set NBODY_BENCHMARK_CLOCK to the compiled benchmark_clock module")
local now = assert(package.loadlib(clock_path, "luaopen_benchmark_clock"))()
assert(type(now) == "function", "A monotonic high-resolution clock is required")

local solar_mass = 4 * 3.141592653589793 * 3.141592653589793
local days_per_year = 365.24
local time_step = 0.01

local function initial_bodies()
    return {
        {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, solar_mass},
        {4.84143144246472090, -1.16032004402742839, -0.103622044471123109, 0.00166007664274403694 * days_per_year, 0.00769901118419740425 * days_per_year, -0.0000690460016972063023 * days_per_year, 0.000954791938424326609 * solar_mass},
        {8.34336671824457987, 4.12479856412430479, -0.403523417114321381, -0.00276742510726862411 * days_per_year, 0.00499852801234917238 * days_per_year, 0.0000230417297573763929 * days_per_year, 0.000285885980666130812 * solar_mass},
        {12.8943695621391310, -15.1111514016986312, -0.223307578892655734, 0.00296460137564761618 * days_per_year, 0.00237847173959480950 * days_per_year, -0.0000296589568540237556 * days_per_year, 0.0000436624404335156298 * solar_mass},
        {15.3796971148509165, -25.9193146099879641, 0.179258772950371181, 0.00268067772490389322 * days_per_year, 0.00162824170038242295 * days_per_year, -0.0000951592254519715870 * days_per_year, 0.0000515138902046611451 * solar_mass},
    }
end

local function offset_momentum(bodies)
    local px, py, pz = 0.0, 0.0, 0.0
    for i = 1, 5 do
        local body = bodies[i]
        px = px + body[4] * body[7]
        py = py + body[5] * body[7]
        pz = pz + body[6] * body[7]
    end
    bodies[1][4] = -px / solar_mass
    bodies[1][5] = -py / solar_mass
    bodies[1][6] = -pz / solar_mass
end

local function energy(bodies)
    local total = 0.0
    for i = 1, 5 do
        local left = bodies[i]
        total = total + 0.5 * left[7] * (left[4] * left[4] + left[5] * left[5] + left[6] * left[6])
        for j = i + 1, 5 do
            local right = bodies[j]
            local dx = left[1] - right[1]
            local dy = left[2] - right[2]
            local dz = left[3] - right[3]
            local distance = math.sqrt(dx * dx + dy * dy + dz * dz)
            total = total - left[7] * right[7] / distance
        end
    end
    return total
end

local function advance(bodies)
    for i = 1, 5 do
        local left = bodies[i]
        for j = i + 1, 5 do
            local right = bodies[j]
            local dx = left[1] - right[1]
            local dy = left[2] - right[2]
            local dz = left[3] - right[3]
            local distance = math.sqrt(dx * dx + dy * dy + dz * dz)
            local magnitude = time_step / (distance * distance * distance)
            left[4] = left[4] - dx * right[7] * magnitude
            left[5] = left[5] - dy * right[7] * magnitude
            left[6] = left[6] - dz * right[7] * magnitude
            right[4] = right[4] + dx * left[7] * magnitude
            right[5] = right[5] + dy * left[7] * magnitude
            right[6] = right[6] + dz * left[7] * magnitude
        end
    end
    for i = 1, 5 do
        local body = bodies[i]
        body[1] = body[1] + time_step * body[4]
        body[2] = body[2] + time_step * body[5]
        body[3] = body[3] + time_step * body[6]
    end
end

local function simulate(steps)
    local bodies = initial_bodies()
    offset_momentum(bodies)
    local initial_energy = energy(bodies)
    for _ = 1, steps do
        advance(bodies)
    end
    return bodies, initial_energy, energy(bodies)
end

local function nearly_equal(actual, expected)
    return actual == actual and expected == expected and math.abs(actual) < math.huge and math.abs(expected) < math.huge and math.abs(actual - expected) <= 1e-12 * math.max(1.0, math.abs(expected))
end

local steps = arg[1] == nil and 100000 or assert(tonumber(arg[1]), "Invalid steps")
local samples = arg[2] == nil and 11 or assert(tonumber(arg[2]), "Invalid samples")
local warmup = arg[3] == nil and 3 or assert(tonumber(arg[3]), "Invalid warmup")
assert(#arg <= 3 and math.type(steps) == "integer" and steps >= 0 and steps <= 100000000 and math.type(samples) == "integer" and samples >= 1 and samples <= 1000 and math.type(warmup) == "integer" and warmup >= 0 and warmup <= 1000, "Expected steps=0..100000000, samples=1..1000, warmup=0..1000")
local expected_bodies, expected_initial, expected_final = simulate(steps)
assert(nearly_equal(expected_initial, -0.16907516382852447), "Incorrect initial energy")
local bodies, initial_energy, final_energy
local timings = {}
now()
for index = 1, warmup + samples do
    local start = now()
    bodies, initial_energy, final_energy = simulate(steps)
    local finish = now()
    assert(nearly_equal(initial_energy, expected_initial) and nearly_equal(final_energy, expected_final), "Incorrect n-body energy")
    for body = 1, 5 do
        for field = 1, 7 do
            assert(nearly_equal(bodies[body][field], expected_bodies[body][field]), "Incorrect n-body state")
        end
    end
    if index > warmup then
        timings[#timings + 1] = string.format("%.9f", (finish - start) * 1000)
    end
end
local body_words = {}
for body = 1, 5 do
    for field = 1, 7 do
        body_words[#body_words + 1] = string.format("%.17g", bodies[body][field])
    end
end
io.write(string.format('{"runtime":"Lua","steps":%d,"initial_energy":%.17g,"final_energy":%.17g,"bodies":[%s],"samples_ms":[%s],"warmup":%d}\n', steps, initial_energy, final_energy, table.concat(body_words, ","), table.concat(timings, ","), warmup))
