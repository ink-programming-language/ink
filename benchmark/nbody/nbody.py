"""Time complete scalar five-body simulations with a monotonic clock.

Initial conditions and algorithm follow Christoph Bauer's Benchmarks Game
n-body program. See LICENSE.benchmarksgame for the redistribution notice.
"""

import json
import math
import sys
from time import perf_counter_ns


SOLAR_MASS = 4 * 3.141592653589793 * 3.141592653589793
DAYS_PER_YEAR = 365.24
TIME_STEP = 0.01


def initial_bodies() -> list[list[float]]:
    return [
        [0.0, 0.0, 0.0, 0.0, 0.0, 0.0, SOLAR_MASS],
        [
            4.84143144246472090, -1.16032004402742839,
            -0.103622044471123109,
            0.00166007664274403694 * DAYS_PER_YEAR,
            0.00769901118419740425 * DAYS_PER_YEAR,
            -0.0000690460016972063023 * DAYS_PER_YEAR,
            0.000954791938424326609 * SOLAR_MASS,
        ],
        [
            8.34336671824457987, 4.12479856412430479,
            -0.403523417114321381,
            -0.00276742510726862411 * DAYS_PER_YEAR,
            0.00499852801234917238 * DAYS_PER_YEAR,
            0.0000230417297573763929 * DAYS_PER_YEAR,
            0.000285885980666130812 * SOLAR_MASS,
        ],
        [
            12.8943695621391310, -15.1111514016986312,
            -0.223307578892655734,
            0.00296460137564761618 * DAYS_PER_YEAR,
            0.00237847173959480950 * DAYS_PER_YEAR,
            -0.0000296589568540237556 * DAYS_PER_YEAR,
            0.0000436624404335156298 * SOLAR_MASS,
        ],
        [
            15.3796971148509165, -25.9193146099879641,
            0.179258772950371181,
            0.00268067772490389322 * DAYS_PER_YEAR,
            0.00162824170038242295 * DAYS_PER_YEAR,
            -0.0000951592254519715870 * DAYS_PER_YEAR,
            0.0000515138902046611451 * SOLAR_MASS,
        ],
    ]


def offset_momentum(bodies: list[list[float]]) -> None:
    px = py = pz = 0.0
    for body in bodies:
        px += body[3] * body[6]
        py += body[4] * body[6]
        pz += body[5] * body[6]
    bodies[0][3] = -px / SOLAR_MASS
    bodies[0][4] = -py / SOLAR_MASS
    bodies[0][5] = -pz / SOLAR_MASS


def energy(bodies: list[list[float]]) -> float:
    total = 0.0
    for i in range(5):
        left = bodies[i]
        total += 0.5 * left[6] * (
            left[3] * left[3] + left[4] * left[4] + left[5] * left[5]
        )
        for j in range(i + 1, 5):
            right = bodies[j]
            dx = left[0] - right[0]
            dy = left[1] - right[1]
            dz = left[2] - right[2]
            distance = math.sqrt(dx * dx + dy * dy + dz * dz)
            total -= left[6] * right[6] / distance
    return total


def advance(bodies: list[list[float]]) -> None:
    for i in range(5):
        left = bodies[i]
        for j in range(i + 1, 5):
            right = bodies[j]
            dx = left[0] - right[0]
            dy = left[1] - right[1]
            dz = left[2] - right[2]
            distance = math.sqrt(dx * dx + dy * dy + dz * dz)
            magnitude = TIME_STEP / (distance * distance * distance)
            left[3] -= dx * right[6] * magnitude
            left[4] -= dy * right[6] * magnitude
            left[5] -= dz * right[6] * magnitude
            right[3] += dx * left[6] * magnitude
            right[4] += dy * left[6] * magnitude
            right[5] += dz * left[6] * magnitude
    for body in bodies:
        body[0] += TIME_STEP * body[3]
        body[1] += TIME_STEP * body[4]
        body[2] += TIME_STEP * body[5]


def simulate(steps: int) -> tuple[list[list[float]], float, float]:
    bodies = initial_bodies()
    offset_momentum(bodies)
    initial_energy = energy(bodies)
    for _ in range(steps):
        advance(bodies)
    return bodies, initial_energy, energy(bodies)


def nearly_equal(actual: float, expected: float) -> bool:
    return (
        math.isfinite(actual)
        and math.isfinite(expected)
        and abs(actual - expected) <= 1e-12 * max(1.0, abs(expected))
    )


def main() -> int:
    steps = int(sys.argv[1]) if len(sys.argv) > 1 else 100000
    samples = int(sys.argv[2]) if len(sys.argv) > 2 else 11
    warmup = int(sys.argv[3]) if len(sys.argv) > 3 else 3
    if (
        len(sys.argv) > 4
        or not 0 <= steps <= 100000000
        or not 1 <= samples <= 1000
        or not 0 <= warmup <= 1000
    ):
        raise ValueError(
            "Expected steps=0..100000000, samples=1..1000, warmup=0..1000"
        )
    expected_bodies, expected_initial, expected_final = simulate(steps)
    if not nearly_equal(expected_initial, -0.16907516382852447):
        raise ValueError("Incorrect initial energy")
    timings = []
    for index in range(warmup + samples):
        start = perf_counter_ns()
        bodies, initial_energy, final_energy = simulate(steps)
        end = perf_counter_ns()
        if (
            not nearly_equal(initial_energy, expected_initial)
            or not nearly_equal(final_energy, expected_final)
            or any(
                not nearly_equal(actual, expected)
                for body, expected_body in zip(bodies, expected_bodies)
                for actual, expected in zip(body, expected_body)
            )
        ):
            raise ValueError("Incorrect n-body result")
        if index >= warmup:
            timings.append((end - start) / 1_000_000)
    print(json.dumps({
        "runtime": "Python",
        "steps": steps,
        "initial_energy": initial_energy,
        "final_energy": final_energy,
        "bodies": [value for body in bodies for value in body],
        "samples_ms": timings,
        "warmup": warmup,
    }, allow_nan=False))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
