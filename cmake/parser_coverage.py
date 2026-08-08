#!/usr/bin/env python3

import argparse
import gzip
import html
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys


METRICS = ("lines", "functions", "branches")


def parse_arguments():
    parser = argparse.ArgumentParser(description="Run focused Ink parser tests, build a coverage report, and enforce coverage thresholds.")
    parser.add_argument("--backend", choices=("gcc", "clang"), required=True)
    parser.add_argument("--test-executable", type=Path, required=True)
    parser.add_argument("--test-filter", required=True)
    parser.add_argument("--source-root", type=Path, required=True)
    parser.add_argument("--build-root", type=Path, required=True)
    parser.add_argument("--report-directory", type=Path, required=True)
    parser.add_argument("--minimum-lines", type=percentage, required=True)
    parser.add_argument("--minimum-functions", type=percentage, required=True)
    parser.add_argument("--minimum-branches", type=percentage, required=True)
    parser.add_argument("--gcov", type=Path)
    parser.add_argument("--llvm-profdata", type=Path)
    parser.add_argument("--llvm-cov", type=Path)
    return parser.parse_args()


def percentage(value):
    try:
        result = float(value)
    except ValueError as error:
        raise argparse.ArgumentTypeError(f"expected a percentage, got {value!r}") from error
    if result < 0.0 or result > 100.0:
        raise argparse.ArgumentTypeError(f"percentage must be between 0 and 100, got {result}")
    return result


def resolved(path):
    return path.expanduser().resolve()


def is_strict_child(path, parent):
    try:
        return path.relative_to(parent) != Path(".")
    except ValueError:
        return False


def prepare_report_directory(report_directory, build_root):
    if not is_strict_child(report_directory, build_root):
        raise RuntimeError(f"refusing to replace report directory outside the build tree: {report_directory}")
    if report_directory.exists():
        shutil.rmtree(report_directory)
    report_directory.mkdir(parents=True)


def run_command(command, *, cwd=None, env=None, capture_output=False):
    result = subprocess.run([str(argument) for argument in command], cwd=cwd, env=env, text=True, stdout=subprocess.PIPE if capture_output else None, stderr=subprocess.PIPE if capture_output else None)
    if result.returncode != 0:
        if capture_output:
            if result.stdout:
                print(result.stdout, end="", file=sys.stderr)
            if result.stderr:
                print(result.stderr, end="", file=sys.stderr)
        raise RuntimeError(f"command failed with exit code {result.returncode}: {' '.join(str(argument) for argument in command)}")
    return result


def run_focused_tests(test_executable, test_filter, build_root, environment=None):
    if not test_executable.is_file():
        raise RuntimeError(f"parser test executable does not exist: {test_executable}")
    print(f"Running parser tests with filter {test_filter}")
    run_command([test_executable, f"--gtest_filter={test_filter}", "--gtest_color=no"], cwd=build_root, env=environment)


def is_instrumented_parser_object(path):
    spelling = path.as_posix()
    return "/CMakeFiles/ink_parser.dir/" in spelling or "/CMakeFiles/ink_tests.dir/parser/" in spelling


def find_gcc_objects(build_root):
    objects = sorted(path for path in build_root.rglob("*.gcno") if is_instrumented_parser_object(path))
    if not objects:
        raise RuntimeError("no parser .gcno files were found; build ink_tests after enabling INK_ENABLE_PARSER_COVERAGE")
    return objects


def clean_gcc_counts(objects):
    for object_path in objects:
        count_path = object_path.with_suffix(".gcda")
        if count_path.exists():
            count_path.unlink()


def parser_relative_path(raw_path, source_root):
    source_path = Path(raw_path)
    if not source_path.is_absolute():
        source_path = source_root / source_path
    source_path = resolved(source_path)
    parser_roots = (source_root / "src" / "lib" / "parser", source_root / "src" / "include" / "ink" / "parser")
    for parser_root in parser_roots:
        try:
            source_path.relative_to(parser_root)
            return source_path.relative_to(source_root).as_posix()
        except ValueError:
            continue
    return None


def empty_gcc_file():
    return {"lines": {}, "functions": {}, "branches": {}}


def read_gcc_coverage(gcov, objects, report_directory, source_root, environment):
    for object_path in objects:
        if not object_path.with_suffix(".gcda").exists():
            continue
        run_command([gcov, "--json-format", "--branch-probabilities", "--branch-counts", "--demangled-names", "--preserve-paths", "--hash-filenames", object_path], cwd=report_directory, env=environment, capture_output=True)

    json_paths = sorted(report_directory.glob("*.gcov.json.gz"))
    if not json_paths:
        raise RuntimeError("gcov did not produce any JSON coverage data")

    coverage = {}
    for json_path in json_paths:
        with gzip.open(json_path, "rt", encoding="utf-8") as stream:
            document = json.load(stream)
        for file_entry in document.get("files", []):
            relative_path = parser_relative_path(file_entry.get("file", ""), source_root)
            if relative_path is None:
                continue
            file_coverage = coverage.setdefault(relative_path, empty_gcc_file())
            for line in file_entry.get("lines", []):
                line_number = int(line["line_number"])
                file_coverage["lines"][line_number] = file_coverage["lines"].get(line_number, 0) + int(line.get("count", 0))
                for branch_index, branch in enumerate(line.get("branches", [])):
                    branch_key = (line_number, branch_index)
                    file_coverage["branches"][branch_key] = file_coverage["branches"].get(branch_key, 0) + int(branch.get("count", 0))
            for function in file_entry.get("functions", []):
                function_key = (function.get("demangled_name", function.get("name", "<unknown>")), int(function.get("start_line", 0)))
                file_coverage["functions"][function_key] = file_coverage["functions"].get(function_key, 0) + int(function.get("execution_count", 0))

    summaries = {}
    for relative_path, file_coverage in coverage.items():
        summaries[relative_path] = {"lines": summarize_counts(file_coverage["lines"].values()), "functions": summarize_counts(file_coverage["functions"].values()), "branches": summarize_counts(file_coverage["branches"].values())}
    return summaries


def run_gcc_backend(arguments):
    if arguments.gcov is None:
        raise RuntimeError("the GCC backend requires --gcov")
    objects = find_gcc_objects(arguments.build_root)
    clean_gcc_counts(objects)
    environment = os.environ.copy()
    environment["PATH"] = str(resolved(arguments.gcov).parent) + os.pathsep + environment.get("PATH", "")
    run_focused_tests(arguments.test_executable, arguments.test_filter, arguments.build_root, environment)
    return read_gcc_coverage(arguments.gcov, objects, arguments.report_directory, arguments.source_root, environment)


def read_clang_coverage(document, source_root):
    coverage = {}
    for data_entry in document.get("data", []):
        for file_entry in data_entry.get("files", []):
            relative_path = parser_relative_path(file_entry.get("filename", ""), source_root)
            if relative_path is None:
                continue
            summary = file_entry.get("summary", {})
            coverage[relative_path] = {metric: {"covered": int(summary.get(metric, {}).get("covered", 0)), "total": int(summary.get(metric, {}).get("count", 0))} for metric in METRICS}
    return coverage


def run_clang_backend(arguments):
    if arguments.llvm_profdata is None or arguments.llvm_cov is None:
        raise RuntimeError("the Clang backend requires --llvm-profdata and --llvm-cov")
    raw_profile = arguments.report_directory / "parser-%p.profraw"
    environment = os.environ.copy()
    environment["LLVM_PROFILE_FILE"] = str(raw_profile)
    run_focused_tests(arguments.test_executable, arguments.test_filter, arguments.build_root, environment)
    raw_profiles = sorted(arguments.report_directory.glob("parser-*.profraw"))
    if not raw_profiles:
        raise RuntimeError("the parser test run did not produce any Clang raw profiles")
    merged_profile = arguments.report_directory / "parser.profdata"
    run_command([arguments.llvm_profdata, "merge", "-sparse", *raw_profiles, "-o", merged_profile], cwd=arguments.report_directory)
    export = run_command([arguments.llvm_cov, "export", arguments.test_executable, f"-instr-profile={merged_profile}", "-format=text"], cwd=arguments.report_directory, capture_output=True)
    return read_clang_coverage(json.loads(export.stdout), arguments.source_root)


def summarize_counts(counts):
    counts = list(counts)
    return {"covered": sum(1 for count in counts if count > 0), "total": len(counts)}


def metric_percent(summary):
    if summary["total"] == 0:
        return 100.0
    return 100.0 * summary["covered"] / summary["total"]


def total_summary(files):
    return {metric: {"covered": sum(file_summary[metric]["covered"] for file_summary in files.values()), "total": sum(file_summary[metric]["total"] for file_summary in files.values())} for metric in METRICS}


def write_json_report(path, backend, test_filter, files, totals, thresholds):
    document = {"backend": backend, "test_filter": test_filter, "thresholds": thresholds, "totals": totals, "files": files}
    path.write_text(json.dumps(document, ensure_ascii=False, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def format_metric(label, summary):
    return f"{label:<10} {summary['covered']:>6}/{summary['total']:<6} {metric_percent(summary):>7.2f}%"


def write_text_report(path, backend, test_filter, files, totals, thresholds):
    lines = ["Ink parser coverage", f"Backend: {backend}", f"GoogleTest filter: {test_filter}", "", format_metric("Lines", totals["lines"]), format_metric("Functions", totals["functions"]), format_metric("Branches", totals["branches"]), "", "Thresholds", f"Lines      {thresholds['lines']:.2f}%", f"Functions  {thresholds['functions']:.2f}%", f"Branches   {thresholds['branches']:.2f}%", "", "Files"]
    for relative_path, summary in sorted(files.items()):
        lines.append(relative_path)
        lines.append(f"  {format_metric('Lines', summary['lines']).strip()}")
        lines.append(f"  {format_metric('Functions', summary['functions']).strip()}")
        lines.append(f"  {format_metric('Branches', summary['branches']).strip()}")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def html_metric(summary):
    return f"{summary['covered']} / {summary['total']} ({metric_percent(summary):.2f}%)"


def write_html_report(path, backend, test_filter, files, totals, thresholds):
    rows = []
    for relative_path, summary in sorted(files.items()):
        rows.append(f"<tr><td>{html.escape(relative_path)}</td><td>{html_metric(summary['lines'])}</td><td>{html_metric(summary['functions'])}</td><td>{html_metric(summary['branches'])}</td></tr>")
    body = "\n".join(rows)
    document = f"""<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Ink parser coverage</title>
<style>
body {{ color: #1f2937; font-family: system-ui, sans-serif; margin: 2rem auto; max-width: 1100px; padding: 0 1rem; }}
table {{ border-collapse: collapse; width: 100%; }}
th, td {{ border-bottom: 1px solid #d1d5db; padding: .55rem; text-align: right; }}
th:first-child, td:first-child {{ text-align: left; }}
.summary {{ display: flex; flex-wrap: wrap; gap: 1rem; margin: 1.5rem 0; }}
.metric {{ background: #f3f4f6; border-radius: .5rem; padding: .8rem 1rem; }}
</style>
</head>
<body>
<h1>Ink parser coverage</h1>
<p>Backend: {html.escape(backend)}<br>GoogleTest filter: <code>{html.escape(test_filter)}</code></p>
<div class="summary"><div class="metric"><strong>Lines</strong><br>{html_metric(totals['lines'])}<br>minimum {thresholds['lines']:.2f}%</div><div class="metric"><strong>Functions</strong><br>{html_metric(totals['functions'])}<br>minimum {thresholds['functions']:.2f}%</div><div class="metric"><strong>Branches</strong><br>{html_metric(totals['branches'])}<br>minimum {thresholds['branches']:.2f}%</div></div>
<table><thead><tr><th>File</th><th>Lines</th><th>Functions</th><th>Branches</th></tr></thead><tbody>
{body}
</tbody></table>
</body>
</html>
"""
    path.write_text(document, encoding="utf-8")


def enforce_thresholds(totals, thresholds):
    failures = []
    for metric in METRICS:
        if totals[metric]["total"] == 0:
            failures.append(f"{metric} coverage contains no measurable entries")
        elif metric_percent(totals[metric]) + 1e-9 < thresholds[metric]:
            failures.append(f"{metric} coverage {metric_percent(totals[metric]):.2f}% is below {thresholds[metric]:.2f}%")
    if failures:
        raise RuntimeError("; ".join(failures))


def main():
    arguments = parse_arguments()
    arguments.source_root = resolved(arguments.source_root)
    arguments.build_root = resolved(arguments.build_root)
    arguments.report_directory = resolved(arguments.report_directory)
    arguments.test_executable = resolved(arguments.test_executable)
    prepare_report_directory(arguments.report_directory, arguments.build_root)

    if arguments.backend == "gcc":
        files = run_gcc_backend(arguments)
    else:
        files = run_clang_backend(arguments)
    if not files:
        raise RuntimeError("coverage data did not contain any src/lib/parser or src/include/ink/parser files")

    totals = total_summary(files)
    thresholds = {"lines": arguments.minimum_lines, "functions": arguments.minimum_functions, "branches": arguments.minimum_branches}
    write_json_report(arguments.report_directory / "parser-coverage.json", arguments.backend, arguments.test_filter, files, totals, thresholds)
    write_text_report(arguments.report_directory / "parser-coverage.txt", arguments.backend, arguments.test_filter, files, totals, thresholds)
    write_html_report(arguments.report_directory / "index.html", arguments.backend, arguments.test_filter, files, totals, thresholds)

    print("")
    print(format_metric("Lines", totals["lines"]))
    print(format_metric("Functions", totals["functions"]))
    print(format_metric("Branches", totals["branches"]))
    print(f"Reports: {arguments.report_directory}")
    enforce_thresholds(totals, thresholds)
    print("Parser coverage thresholds passed")


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, json.JSONDecodeError) as error:
        print(f"parser coverage failed: {error}", file=sys.stderr)
        sys.exit(1)
