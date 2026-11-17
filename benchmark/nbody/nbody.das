options gen2

require daslib/fio
require strings
require math

// Initial conditions and scalar algorithm follow Christoph Bauer's Benchmarks Game program.
// See LICENSE.benchmarksgame for the upstream redistribution notice.
let solar_mass = 4.0lf * 3.141592653589793lf * 3.141592653589793lf
let days_per_year = 365.24lf
let time_step = 0.01lf

struct Simulation
{
    bodies : double[35]
    initial_energy : double
    final_energy : double
}

def reset_bodies(var bodies : double[35])
{
    for (index in 0..35)
    {
        bodies[index] = 0.0lf
    }
    bodies[6] = solar_mass
    bodies[7] = 4.84143144246472090lf
    bodies[8] = -1.16032004402742839lf
    bodies[9] = -0.103622044471123109lf
    bodies[10] = 0.00166007664274403694lf * days_per_year
    bodies[11] = 0.00769901118419740425lf * days_per_year
    bodies[12] = -0.0000690460016972063023lf * days_per_year
    bodies[13] = 0.000954791938424326609lf * solar_mass
    bodies[14] = 8.34336671824457987lf
    bodies[15] = 4.12479856412430479lf
    bodies[16] = -0.403523417114321381lf
    bodies[17] = -0.00276742510726862411lf * days_per_year
    bodies[18] = 0.00499852801234917238lf * days_per_year
    bodies[19] = 0.0000230417297573763929lf * days_per_year
    bodies[20] = 0.000285885980666130812lf * solar_mass
    bodies[21] = 12.8943695621391310lf
    bodies[22] = -15.1111514016986312lf
    bodies[23] = -0.223307578892655734lf
    bodies[24] = 0.00296460137564761618lf * days_per_year
    bodies[25] = 0.00237847173959480950lf * days_per_year
    bodies[26] = -0.0000296589568540237556lf * days_per_year
    bodies[27] = 0.0000436624404335156298lf * solar_mass
    bodies[28] = 15.3796971148509165lf
    bodies[29] = -25.9193146099879641lf
    bodies[30] = 0.179258772950371181lf
    bodies[31] = 0.00268067772490389322lf * days_per_year
    bodies[32] = 0.00162824170038242295lf * days_per_year
    bodies[33] = -0.0000951592254519715870lf * days_per_year
    bodies[34] = 0.0000515138902046611451lf * solar_mass
}

def offset_momentum(var bodies : double[35])
{
    var px = 0.0lf
    var py = 0.0lf
    var pz = 0.0lf
    for (index in 0..5)
    {
        let body = index * 7
        px += bodies[body + 3] * bodies[body + 6]
        py += bodies[body + 4] * bodies[body + 6]
        pz += bodies[body + 5] * bodies[body + 6]
    }
    bodies[3] = -px / solar_mass
    bodies[4] = -py / solar_mass
    bodies[5] = -pz / solar_mass
}

def energy(bodies : double[35]) : double
{
    var result = 0.0lf
    for (i in 0..5)
    {
        let left = i * 7
        let vx = bodies[left + 3]
        let vy = bodies[left + 4]
        let vz = bodies[left + 5]
        result += 0.5lf * bodies[left + 6] * (vx * vx + vy * vy + vz * vz)
        for (j in i + 1..5)
        {
            let right = j * 7
            let dx = bodies[left] - bodies[right]
            let dy = bodies[left + 1] - bodies[right + 1]
            let dz = bodies[left + 2] - bodies[right + 2]
            let distance = sqrt(dx * dx + dy * dy + dz * dz)
            result -= bodies[left + 6] * bodies[right + 6] / distance
        }
    }
    return result
}

def advance(var bodies : double[35])
{
    for (i in 0..5)
    {
        let left = i * 7
        for (j in i + 1..5)
        {
            let right = j * 7
            let dx = bodies[left] - bodies[right]
            let dy = bodies[left + 1] - bodies[right + 1]
            let dz = bodies[left + 2] - bodies[right + 2]
            let distance = sqrt(dx * dx + dy * dy + dz * dz)
            let magnitude = time_step / (distance * distance * distance)
            bodies[left + 3] -= dx * bodies[right + 6] * magnitude
            bodies[left + 4] -= dy * bodies[right + 6] * magnitude
            bodies[left + 5] -= dz * bodies[right + 6] * magnitude
            bodies[right + 3] += dx * bodies[left + 6] * magnitude
            bodies[right + 4] += dy * bodies[left + 6] * magnitude
            bodies[right + 5] += dz * bodies[left + 6] * magnitude
        }
    }
    for (index in 0..5)
    {
        let body = index * 7
        bodies[body] += time_step * bodies[body + 3]
        bodies[body + 1] += time_step * bodies[body + 4]
        bodies[body + 2] += time_step * bodies[body + 5]
    }
}

[sideeffects]
def simulate(steps : int; var result : Simulation)
{
    reset_bodies(result.bodies)
    offset_momentum(result.bodies)
    result.initial_energy = energy(result.bodies)
    for (step in 0..steps)
    {
        advance(result.bodies)
    }
    result.final_energy = energy(result.bodies)
}

def nearly_equal(actual, expected : double) : bool
{
    // The finite guard also rejects NaN and infinity without relying on narrowing casts.
    return abs(actual) <= 1.7976931348623157e308lf && abs(expected) <= 1.7976931348623157e308lf && abs(actual - expected) <= 1.0e-12lf * max(1.0lf, abs(expected))
}

def validate(actual, expected : Simulation)
{
    assert(nearly_equal(actual.initial_energy, -0.16907516382852447lf), "Incorrect initial n-body energy")
    assert(nearly_equal(actual.initial_energy, expected.initial_energy), "Initial n-body energy mismatch")
    assert(nearly_equal(actual.final_energy, expected.final_energy), "Final n-body energy mismatch")
    for (index in 0..35)
    {
        assert(nearly_equal(actual.bodies[index], expected.bodies[index]), "N-body state mismatch")
    }
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
    assert(separator >= 0 && length(args) == separator + 4, "Expected -- steps samples warmup")
    let steps = to_int(args[separator + 1])
    let samples = to_int(args[separator + 2])
    let warmup = to_int(args[separator + 3])
    assert(steps >= 0 && steps <= 100000000 && samples >= 1 && samples <= 1000 && warmup >= 0 && warmup <= 1000, "Expected steps=0..100000000, samples=1..1000, warmup=0..1000")
    var expected : Simulation
    simulate(steps, expected)
    var result : Simulation
    var timings : array<double>
    ref_time_ticks()
    for (index in 0..warmup + samples)
    {
        let start = ref_time_ticks()
        simulate(steps, result)
        let elapsed = get_time_nsec(start)
        validate(result, expected)
        if (index >= warmup)
        {
            timings |> push(double(elapsed) / 1000000.0lf)
        }
    }
    print("\{\"runtime\":\"Daslang\",\"steps\":{steps},\"initial_energy\":{result.initial_energy:.17g},\"final_energy\":{result.final_energy:.17g},\"warmup\":{warmup},\"bodies\":[")
    for (index in 0..35)
    {
        if (index != 0)
        {
            print(",")
        }
        print("{result.bodies[index]:.17g}")
    }
    print("],\"samples_ms\":[")
    for (index in 0..length(timings))
    {
        if (index != 0)
        {
            print(",")
        }
        print("{timings[index]:.9f}")
    }
    print("]\}\n")
}
