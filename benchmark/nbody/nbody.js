"use strict";

// Initial conditions and algorithm follow Christoph Bauer's Benchmarks Game
// n-body program. See LICENSE.benchmarksgame for the redistribution notice.
const solarMass = 4 * 3.141592653589793 * 3.141592653589793;
const daysPerYear = 365.24;
const timeStep = 0.01;

function initialBodies() {
    return [
        [0, 0, 0, 0, 0, 0, solarMass],
        [4.84143144246472090, -1.16032004402742839, -0.103622044471123109, 0.00166007664274403694 * daysPerYear, 0.00769901118419740425 * daysPerYear, -0.0000690460016972063023 * daysPerYear, 0.000954791938424326609 * solarMass],
        [8.34336671824457987, 4.12479856412430479, -0.403523417114321381, -0.00276742510726862411 * daysPerYear, 0.00499852801234917238 * daysPerYear, 0.0000230417297573763929 * daysPerYear, 0.000285885980666130812 * solarMass],
        [12.8943695621391310, -15.1111514016986312, -0.223307578892655734, 0.00296460137564761618 * daysPerYear, 0.00237847173959480950 * daysPerYear, -0.0000296589568540237556 * daysPerYear, 0.0000436624404335156298 * solarMass],
        [15.3796971148509165, -25.9193146099879641, 0.179258772950371181, 0.00268067772490389322 * daysPerYear, 0.00162824170038242295 * daysPerYear, -0.0000951592254519715870 * daysPerYear, 0.0000515138902046611451 * solarMass],
    ];
}

function offsetMomentum(bodies) {
    let px = 0;
    let py = 0;
    let pz = 0;
    for (const body of bodies) {
        px += body[3] * body[6];
        py += body[4] * body[6];
        pz += body[5] * body[6];
    }
    bodies[0][3] = -px / solarMass;
    bodies[0][4] = -py / solarMass;
    bodies[0][5] = -pz / solarMass;
}

function energy(bodies) {
    let total = 0;
    for (let i = 0; i < 5; ++i) {
        const left = bodies[i];
        total += 0.5 * left[6] * (left[3] * left[3] + left[4] * left[4] + left[5] * left[5]);
        for (let j = i + 1; j < 5; ++j) {
            const right = bodies[j];
            const dx = left[0] - right[0];
            const dy = left[1] - right[1];
            const dz = left[2] - right[2];
            const distance = Math.sqrt(dx * dx + dy * dy + dz * dz);
            total -= left[6] * right[6] / distance;
        }
    }
    return total;
}

function advance(bodies) {
    for (let i = 0; i < 5; ++i) {
        const left = bodies[i];
        for (let j = i + 1; j < 5; ++j) {
            const right = bodies[j];
            const dx = left[0] - right[0];
            const dy = left[1] - right[1];
            const dz = left[2] - right[2];
            const distance = Math.sqrt(dx * dx + dy * dy + dz * dz);
            const magnitude = timeStep / (distance * distance * distance);
            left[3] -= dx * right[6] * magnitude;
            left[4] -= dy * right[6] * magnitude;
            left[5] -= dz * right[6] * magnitude;
            right[3] += dx * left[6] * magnitude;
            right[4] += dy * left[6] * magnitude;
            right[5] += dz * left[6] * magnitude;
        }
    }
    for (const body of bodies) {
        body[0] += timeStep * body[3];
        body[1] += timeStep * body[4];
        body[2] += timeStep * body[5];
    }
}

function simulate(steps) {
    const bodies = initialBodies();
    offsetMomentum(bodies);
    const initialEnergy = energy(bodies);
    for (let step = 0; step < steps; ++step) {
        advance(bodies);
    }
    return { bodies, initialEnergy, finalEnergy: energy(bodies) };
}

function nearlyEqual(actual, expected) {
    return Number.isFinite(actual) && Number.isFinite(expected) && Math.abs(actual - expected) <= 1e-12 * Math.max(1, Math.abs(expected));
}

const steps = process.argv.length > 2 ? Number(process.argv[2]) : 100000;
const samples = process.argv.length > 3 ? Number(process.argv[3]) : 11;
const warmup = process.argv.length > 4 ? Number(process.argv[4]) : 3;
if (process.argv.length > 5 || !Number.isInteger(steps) || steps < 0 || steps > 100000000 || !Number.isInteger(samples) || samples < 1 || samples > 1000 || !Number.isInteger(warmup) || warmup < 0 || warmup > 1000) {
    throw new Error("Expected steps=0..100000000, samples=1..1000, warmup=0..1000");
}
const expected = simulate(steps);
if (!nearlyEqual(expected.initialEnergy, -0.16907516382852447)) {
    throw new Error("Incorrect initial energy");
}
let result;
const timings = [];
for (let index = 0; index < warmup + samples; ++index) {
    const start = process.hrtime.bigint();
    result = simulate(steps);
    const end = process.hrtime.bigint();
    if (!nearlyEqual(result.initialEnergy, expected.initialEnergy) || !nearlyEqual(result.finalEnergy, expected.finalEnergy)) {
        throw new Error("Incorrect n-body energy");
    }
    for (let body = 0; body < 5; ++body) {
        for (let field = 0; field < 7; ++field) {
            if (!nearlyEqual(result.bodies[body][field], expected.bodies[body][field])) {
                throw new Error(`Incorrect n-body state at body ${body}, field ${field}`);
            }
        }
    }
    if (index >= warmup) {
        timings.push(Number(end - start) / 1_000_000);
    }
}
console.log(JSON.stringify({ runtime: "JavaScript", steps, initial_energy: result.initialEnergy, final_energy: result.finalEnergy, bodies: result.bodies.flat(), samples_ms: timings, warmup }));
