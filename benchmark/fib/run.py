"""Compare execution-only Fibonacci timings and save reproducible JSON.

Example (after building both C++ targets):
    python benchmark/fib/run.py --ink PATH --cpp PATH --lua PATH

All runtimes use fib(n) = n if n < 2 else fib(n - 1) + fib(n - 2).
Each runtime performs its own untimed warmup. Processes run sequentially.
JavaScript runs with both the default V8 settings and --jitless.
"""

from __future__ import annotations

import argparse
from contextlib import contextmanager
import ctypes
import json
import os
from pathlib import Path
import platform
import statistics
import subprocess
import sys


@contextmanager
def pin_cpu(cpu: int):
    """Pin the runner and inherited child processes, then restore affinity."""
    if cpu < 0:
        yield
        return
    if os.name == "nt":
        kernel = ctypes.WinDLL("kernel32", use_last_error=True)
        kernel.GetCurrentProcess.restype = ctypes.c_void_p
        kernel.GetProcessAffinityMask.argtypes = [
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_size_t),
            ctypes.POINTER(ctypes.c_size_t),
        ]
        kernel.SetProcessAffinityMask.argtypes = [
            ctypes.c_void_p, ctypes.c_size_t,
        ]
        process = kernel.GetCurrentProcess()
        old_mask = ctypes.c_size_t()
        system_mask = ctypes.c_size_t()
        if not kernel.GetProcessAffinityMask(
            process, ctypes.byref(old_mask), ctypes.byref(system_mask)
        ):
            raise ctypes.WinError(ctypes.get_last_error())
        if not (old_mask.value & (1 << cpu)):
            raise ValueError(f"CPU {cpu} is outside available affinity")
        if not kernel.SetProcessAffinityMask(process, 1 << cpu):
            raise ctypes.WinError(ctypes.get_last_error())
        try:
            yield
        finally:
            kernel.SetProcessAffinityMask(process, old_mask.value)
    else:
        old_affinity = os.sched_getaffinity(0)
        os.sched_setaffinity(0, {cpu})
        try:
            yield
        finally:
            os.sched_setaffinity(0, old_affinity)


def capture(
    command: list[str], env: dict[str, str] | None = None,
    priority: str = "normal",
) -> str:
    process_options = {}
    if priority != "normal":
        if os.name != "nt":
            raise ValueError("Child priority options require Windows")
        priority_flags = {
            "above-normal": subprocess.ABOVE_NORMAL_PRIORITY_CLASS,
            "high": subprocess.HIGH_PRIORITY_CLASS,
        }
        process_options["creationflags"] = priority_flags[priority]
    completed = subprocess.run(
        command, text=True, capture_output=True, check=True,
        encoding="utf-8", env=env, **process_options,
    )
    return completed.stdout.strip()


def reference_fib(n: int) -> int:
    previous, current = 0, 1
    for _ in range(n):
        previous, current = current, previous + current
    return previous


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ink", required=True, type=Path)
    parser.add_argument("--ink-before", type=Path)
    parser.add_argument("--asm-before", type=Path)
    parser.add_argument("--cpp", required=True, type=Path)
    parser.add_argument("--lua", required=True, type=Path)
    parser.add_argument("--lua-clock", type=Path)
    parser.add_argument("--python", default=sys.executable)
    parser.add_argument("--node", default="node")
    parser.add_argument("--daslang", type=Path)
    parser.add_argument(
        "--daslang-mode", choices=["interpreter", "jit", "both"],
        default="interpreter",
    )
    parser.add_argument("--n", type=int, default=30)
    parser.add_argument("--samples", type=int, default=11)
    parser.add_argument("--warmup", type=int, default=3)
    parser.add_argument("--rounds", type=int, default=1)
    parser.add_argument("--cpu", type=int, default=2)
    parser.add_argument(
        "--priority", choices=["normal", "above-normal", "high"],
        default="normal",
        help="Temporary child process priority; elevated options need Windows",
    )
    parser.add_argument(
        "--output", type=Path, default=Path("build/benchmark/fib/results.json")
    )
    arguments = parser.parse_args()
    if bool(arguments.ink_before) != bool(arguments.asm_before):
        parser.error("--ink-before and --asm-before must be supplied together")
    if arguments.priority != "normal" and os.name != "nt":
        parser.error("Child priority options require Windows")
    if arguments.daslang_mode != "interpreter" and not arguments.daslang:
        parser.error("--daslang-mode requires --daslang")
    if (
        not 0 <= arguments.n <= 40
        or not 1 <= arguments.samples <= 1000
        or not 0 <= arguments.warmup <= 1000
        or not 1 <= arguments.rounds <= 100
    ):
        parser.error(
            "Expected n=0..40, samples=1..1000, warmup=0..1000, rounds=1..100"
        )
    directory = Path(__file__).resolve().parent
    output = arguments.output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    parameters = [
        str(arguments.n), str(arguments.samples), str(arguments.warmup),
    ]
    lua_environment = os.environ.copy()
    lua_clock = arguments.lua_clock or arguments.lua.resolve().with_name(
        "benchmark_clock.dll" if os.name == "nt" else "benchmark_clock.so"
    )
    lua_environment["FIB_BENCHMARK_CLOCK"] = str(lua_clock.resolve())
    ink_name = "Ink after" if arguments.ink_before else "Ink"
    commands = {}
    if arguments.ink_before:
        commands["Ink before"] = [
            str(arguments.ink_before.resolve()),
            str(arguments.asm_before.resolve()), *parameters,
        ]
    commands[ink_name] = [
        str(arguments.ink.resolve()),
        str(directory / "fib.asm"), *parameters,
    ]
    ink_names = set(commands)
    commands.update({
        "C++": [str(arguments.cpp.resolve()), *parameters],
        "Python": [arguments.python, str(directory / "fib.py"), *parameters],
        "Lua": [
            str(arguments.lua.resolve()), str(directory / "fib.lua"),
            *parameters,
        ],
        "JavaScript": [arguments.node, str(directory / "fib.js"), *parameters],
        "JavaScript jitless": [
            arguments.node, "--jitless", str(directory / "fib.js"), *parameters,
        ],
    })
    if arguments.daslang:
        daslang_command = [
            str(arguments.daslang.resolve()), str(directory / "fib.das"),
            "-no-module-cache",
        ]
        if arguments.daslang_mode in {"interpreter", "both"}:
            commands["Daslang interpreter"] = [
                *daslang_command, "--", *parameters,
            ]
        if arguments.daslang_mode in {"jit", "both"}:
            commands["Daslang LLVM JIT"] = [
                *daslang_command, "-jit", "-jit-no-cache", "--", *parameters,
            ]
    versions = {
        "Python": capture([arguments.python, "--version"]),
        "Lua": capture([str(arguments.lua.resolve()), "-v"]),
        "JavaScript": capture([
            arguments.node, "-p", "JSON.stringify(process.versions)",
        ]),
    }
    if arguments.daslang:
        versions["Daslang"] = capture([
            str(arguments.daslang.resolve()), "--version",
        ])
    runs = []
    expected = reference_fib(arguments.n)
    with pin_cpu(arguments.cpu):
        for round_index in range(arguments.rounds):
            # Rotate process order to reduce order and temperature bias.
            names = list(commands)
            shift = round_index % len(names)
            for name in names[shift:] + names[:shift]:
                command = list(commands[name])
                if name in ink_names:
                    trace_prefix = "execute"
                    if arguments.ink_before:
                        trace_prefix += "-" + name.lower().replace(" ", "-")
                    trace = output.with_name(
                        f"{trace_prefix}-round-{round_index + 1}.trace.json"
                    )
                    command.append(str(trace))
                print(
                    f"Running {name}, round {round_index + 1}...", flush=True
                )
                child_environment = lua_environment if name == "Lua" else None
                value = json.loads(capture(
                    command, child_environment, arguments.priority,
                ))
                if (
                    value["n"] != arguments.n
                    or value["result"] != expected
                    or value["warmup"] != arguments.warmup
                    or len(value["samples_ms"]) != arguments.samples
                ):
                    raise ValueError(
                        f"Invalid result or sample count from {name}: {value}"
                    )
                if name in ink_names:
                    if (
                        value["checks_enabled"]
                        or value["build_mode"] != "shipping"
                    ):
                        raise ValueError(
                            "Ink must use shipping with checks disabled"
                        )
                    trace_data = json.loads(trace.read_text(encoding="utf-8"))
                    events = trace_data["traceEvents"]
                    if len(events) != arguments.samples or any(
                        event["name"] != "Interpreter::execute"
                        or event["ph"] != "X"
                        for event in events
                    ):
                        raise ValueError(
                            "Trace scope or sample count is incorrect"
                        )
                    for event, sample_ms in zip(events, value["samples_ms"]):
                        if abs(event["dur"] / 1000 - sample_ms) > 0.000002:
                            raise ValueError(
                                "Trace differs from measured execute samples"
                            )
                    value["trace"] = str(trace)
                value.update(
                    name=name, round=round_index + 1, command=command,
                    priority=arguments.priority,
                )
                if name == "JavaScript":
                    value["execution_mode"] = "V8 default JIT"
                elif name == "JavaScript jitless":
                    value["execution_mode"] = "V8 jitless"
                elif name.startswith("Daslang "):
                    value["execution_mode"] = (
                        "LLVM JIT" if name.endswith("JIT") else "interpreter"
                    )
                runs.append(value)
    summary = []
    for name in commands:
        samples = [
            sample for run in runs if run["name"] == name
            for sample in run["samples_ms"]
        ]
        summary.append({
            "name": name,
            "median_ms": statistics.median(samples),
            "min_ms": min(samples),
            "max_ms": max(samples),
            "samples": len(samples),
        })
    cpp_median = next(
        row["median_ms"] for row in summary if row["name"] == "C++"
    )
    ink_median = next(
        row["median_ms"] for row in summary if row["name"] == ink_name
    )
    if cpp_median <= 0 or ink_median <= 0:
        raise ValueError("Timing below clock resolution; increase n")
    for row in summary:
        row["relative_to_cpp"] = row["median_ms"] / cpp_median
        row["relative_to_ink"] = row["median_ms"] / ink_median
    if arguments.ink_before:
        before_median = next(
            row["median_ms"] for row in summary if row["name"] == "Ink before"
        )
        if before_median <= 0:
            raise ValueError("Timing below clock resolution; increase n")
        for row in summary:
            row["relative_to_ink_before"] = row["median_ms"] / before_median
            row["speedup_vs_ink_before"] = before_median / row["median_ms"]
    result = {
        "algorithm": "fib(n) = n if n < 2 else fib(n - 1) + fib(n - 2)",
        "memoization": False,
        "n": arguments.n,
        "expected": expected,
        "cpu_affinity": arguments.cpu,
        "priority": arguments.priority,
        "ink_reference_name": ink_name,
        "platform": platform.platform(),
        "processor": platform.processor(),
        "versions": versions,
        "timing": {
            "scope": "Interpreter::execute (Ink); fib(n) (other runtimes)",
            "clock": "In-process monotonic high-resolution elapsed time",
            "excluded": [
                "parsing", "emission", "loading", "warmup",
                "validation", "trace serialization",
            ],
        },
        "runs": runs,
        "summary": summary,
    }
    output_text = json.dumps(result, indent=2, ensure_ascii=False) + "\n"
    output.write_text(output_text, encoding="utf-8")
    print(json.dumps(summary, indent=2))
    print(f"Saved {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
