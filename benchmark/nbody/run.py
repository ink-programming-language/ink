"""Compare execution-only scalar FP64 n-body timings and save all samples.

Each measured call resets five bodies, offsets momentum, computes the initial
energy, advances the requested steps, and computes the final energy. Processes
run sequentially; each round rotates their order to reduce ordering bias.
"""

from __future__ import annotations

import argparse
from contextlib import contextmanager
import ctypes
from datetime import datetime
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import statistics
import subprocess
import sys


@contextmanager
def pin_cpu(cpu: int):
    """Pin this process and inherited children, restoring the original mask."""
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
        flags = {
            "above-normal": subprocess.ABOVE_NORMAL_PRIORITY_CLASS,
            "high": subprocess.HIGH_PRIORITY_CLASS,
        }
        process_options["creationflags"] = flags[priority]
    result = subprocess.run(
        command, text=True, capture_output=True, encoding="utf-8",
        env=env, **process_options,
    )
    if result.returncode:
        raise RuntimeError(
            f"Command failed ({result.returncode}): {command}\n"
            f"{result.stdout}\n{result.stderr}"
        )
    return result.stdout.strip()


def nearly_equal(actual: float, expected: float) -> bool:
    return (
        math.isfinite(actual) and math.isfinite(expected)
        and abs(actual - expected) <= 1.0e-12 * max(1.0, abs(expected))
    )


def validate_result(
    value: dict, expected: dict, steps: int, samples: int, warmup: int,
) -> None:
    if (
        value["steps"] != steps or value["warmup"] != warmup
        or len(value["bodies"]) != 35
        or len(value["samples_ms"]) != samples
        or any(
            not math.isfinite(sample) or sample < 0
            for sample in value["samples_ms"]
        )
    ):
        raise ValueError(f"Incorrect parameters or sample count: {value}")
    for key in ("initial_energy", "final_energy"):
        if not nearly_equal(value[key], expected[key]):
            raise ValueError(
                f"{key} mismatch: {value[key]} / expected {expected[key]}"
            )
    for index, (actual, wanted) in enumerate(
        zip(value["bodies"], expected["bodies"])
    ):
        if not nearly_equal(actual, wanted):
            raise ValueError(
                f"Body word {index} mismatch: {actual} / expected {wanted}"
            )


def processor_name() -> str:
    if os.name == "nt":
        import winreg

        with winreg.OpenKey(
            winreg.HKEY_LOCAL_MACHINE,
            r"HARDWARE\DESCRIPTION\System\CentralProcessor\0",
        ) as key:
            return winreg.QueryValueEx(key, "ProcessorNameString")[0].strip()
    return platform.processor()


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--ink", required=True, type=Path)
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
    parser.add_argument("--steps", type=int, default=100000)
    parser.add_argument("--samples", type=int, default=11)
    parser.add_argument("--warmup", type=int, default=3)
    parser.add_argument("--rounds", type=int, default=3)
    parser.add_argument("--cpu", type=int, default=2)
    parser.add_argument(
        "--priority", choices=["normal", "above-normal", "high"],
        default="normal",
    )
    parser.add_argument(
        "--output", type=Path,
        default=Path("build/benchmark/nbody/results.json"),
    )
    args = parser.parse_args()
    if args.priority != "normal" and os.name != "nt":
        parser.error("Child priority options require Windows")
    if args.daslang_mode != "interpreter" and not args.daslang:
        parser.error("--daslang-mode requires --daslang")
    if (
        not 0 <= args.steps <= 100000000
        or not 1 <= args.samples <= 1000
        or not 0 <= args.warmup <= 1000
        or not 1 <= args.rounds <= 100
        or args.cpu < -1
    ):
        parser.error(
            "Expected steps=0..100000000, samples=1..1000, "
            "warmup=0..1000, rounds=1..100, cpu>=-1"
        )
    directory = Path(__file__).resolve().parent
    repository = directory.parent.parent
    output = args.output.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    parameters = [str(args.steps), str(args.samples), str(args.warmup)]
    python_environment = os.environ.copy()
    python_environment["PYTHON_JIT"] = "0"
    lua_environment = os.environ.copy()
    lua_clock = args.lua_clock or args.lua.resolve().with_name(
        "benchmark_clock.dll" if os.name == "nt" else "benchmark_clock.so"
    )
    lua_environment["NBODY_BENCHMARK_CLOCK"] = str(lua_clock.resolve())
    commands = {
        "Ink": [
            str(args.ink.resolve()), str(directory / "nbody.asm"), *parameters,
        ],
        "C++": [str(args.cpp.resolve()), *parameters],
        "Python": [args.python, str(directory / "nbody.py"), *parameters],
        "Lua": [
            str(args.lua.resolve()), str(directory / "nbody.lua"), *parameters,
        ],
        "JavaScript": [args.node, str(directory / "nbody.js"), *parameters],
        "JavaScript jitless": [
            args.node, "--jitless", str(directory / "nbody.js"), *parameters,
        ],
    }
    if args.daslang:
        daslang_command = [
            str(args.daslang.resolve()), str(directory / "nbody.das"),
            "-no-module-cache",
        ]
        if args.daslang_mode in {"interpreter", "both"}:
            commands["Daslang interpreter"] = [
                *daslang_command, "--", *parameters,
            ]
        if args.daslang_mode in {"jit", "both"}:
            commands["Daslang LLVM JIT"] = [
                *daslang_command, "-jit", "-jit-no-cache", "--", *parameters,
            ]
    versions = {
        "Python": capture(
            [args.python, "-c", "import sys; print(sys.version)"],
            python_environment,
        ),
        "Python JIT": "disabled via PYTHON_JIT=0",
        "Lua": capture([str(args.lua.resolve()), "-v"]),
        "JavaScript": capture([
            args.node, "-p", "JSON.stringify(process.versions)",
        ]),
    }
    if args.daslang:
        versions["Daslang"] = capture([
            str(args.daslang.resolve()), "--version",
        ])
    # Obtain a common numerical reference outside the measured processes/runs.
    expected = json.loads(capture([
        str(args.cpp.resolve()), str(args.steps), "1", "0",
    ]))
    if not nearly_equal(expected["initial_energy"], -0.16907516382852447):
        raise ValueError(
            "C++ reference initial energy differs from the golden value"
        )
    validate_result(expected, expected, args.steps, 1, 0)
    runs = []
    with pin_cpu(args.cpu):
        for round_index in range(args.rounds):
            names = list(commands)
            shift = round_index % len(names)
            for name in names[shift:] + names[:shift]:
                command = list(commands[name])
                if name == "Ink":
                    trace = output.with_name(
                        f"execute-round-{round_index + 1}.trace.json"
                    )
                    command.append(str(trace))
                print(
                    f"Running {name}, round {round_index + 1}...", flush=True
                )
                environment = None
                if name == "Python":
                    environment = python_environment
                elif name == "Lua":
                    environment = lua_environment
                value = json.loads(capture(
                    command, environment, args.priority,
                ))
                validate_result(
                    value, expected, args.steps, args.samples, args.warmup,
                )
                if name == "Ink":
                    if (
                        value["checks_enabled"]
                        or value["build_mode"] != "shipping"
                    ):
                        raise ValueError(
                            "Ink must use shipping with checks disabled"
                        )
                    events = json.loads(
                        trace.read_text(encoding="utf-8")
                    )["traceEvents"]
                    if len(events) != args.samples or any(
                        event["name"] != "Interpreter::execute"
                        or event["ph"] != "X"
                        or not math.isfinite(event["dur"])
                        or event["dur"] < 0
                        or event["args"]["steps"] != args.steps
                        for event in events
                    ):
                        raise ValueError(
                            "Incorrect execute trace scope or count"
                        )
                    for event, sample_ms in zip(events, value["samples_ms"]):
                        if abs(event["dur"] / 1000 - sample_ms) > 0.000002:
                            raise ValueError(
                                "Trace duration differs from execute timing"
                            )
                    value["trace"] = str(trace)
                if name == "JavaScript":
                    value["execution_mode"] = "V8 default JIT"
                elif name == "JavaScript jitless":
                    value["execution_mode"] = "V8 jitless"
                elif name.startswith("Daslang "):
                    value["execution_mode"] = (
                        "LLVM JIT" if name.endswith("JIT") else "interpreter"
                    )
                value.update(
                    name=name, round=round_index + 1, command=command,
                    priority=args.priority,
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
        row["median_ms"] for row in summary if row["name"] == "Ink"
    )
    if cpp_median <= 0 or ink_median <= 0:
        raise ValueError("Timing below clock resolution; increase steps")
    for row in summary:
        row["relative_to_cpp"] = row["median_ms"] / cpp_median
        row["relative_to_ink"] = row["median_ms"] / ink_median
    source_files = [
        path for path in directory.iterdir()
        if path.is_file() and path.suffix in {
            ".asm", ".cpp", ".h", ".py", ".js", ".lua", ".das",
        }
    ]
    for folder in ("src/include/ink/backend/vm", "src/lib/backend/vm"):
        source_files.extend(
            path for path in (repository / folder).rglob("*")
            if path.is_file() and path.suffix in {".cpp", ".h", ".def"}
        )
    source_hashes = {
        path.relative_to(repository).as_posix():
            hashlib.sha256(path.read_bytes()).hexdigest()
        for path in sorted(source_files)
    }
    result = {
        "algorithm": "scalar FP64 five-body n-body, dt=0.01",
        "measured_at": datetime.now().astimezone().isoformat(),
        "steps": args.steps,
        "rounds": args.rounds,
        "warmup_per_process": args.warmup,
        "samples_per_process": args.samples,
        "cpu_affinity": args.cpu,
        "priority": args.priority,
        "platform": platform.platform(),
        "processor": processor_name(),
        "versions": versions,
        "source_head": capture([
            "git", "-C", str(repository), "rev-parse", "HEAD",
        ]),
        "source_worktree_status": capture([
            "git", "-C", str(repository), "status", "--short", "--",
            "benchmark/nbody", "src/include/ink/backend/vm",
            "src/lib/backend/vm",
        ]),
        "source_hashes_sha256": source_hashes,
        "expected": {
            key: expected[key]
            for key in ("initial_energy", "final_energy", "bodies")
        },
        "timing": {
            "scope": "complete fresh simulation; Interpreter::execute for Ink",
            "included": [
                "state reset", "offset momentum", "initial energy",
                "advance steps", "final energy",
            ],
            "clock": "in-process monotonic high-resolution elapsed time",
            "excluded": [
                "parsing", "emission", "loading",
                "Daslang startup LLVM JIT compilation", "warmup", "reference",
                "validation", "output", "trace serialization",
            ],
            "jit_note": (
                "V8 manages its own tiering; warmup does not guarantee all "
                "V8 compilation is outside the timed interval."
            ),
        },
        "validation": (
            "both energies and 35 body values, "
            "tolerance 1e-12 * max(1, abs(expected))"
        ),
        "runs": runs,
        "summary": summary,
    }
    output.write_text(
        json.dumps(result, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
    )
    print(json.dumps(summary, indent=2))
    print(f"Saved {output}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
