#!/usr/bin/env python3
"""Format project-owned C/C++ sources with the repository's clang-format style.

Requires Python 3.10 or newer and clang-format 19 or newer.
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys


SOURCE_EXTENSIONS = frozenset({".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx", ".def"})
BATCH_SIZE = 64
MINIMUM_CLANG_FORMAT_VERSION = 19


class FormatError(RuntimeError):
    """An expected configuration or external-tool failure."""


def run_captured(command: list[str], repository: Path) -> subprocess.CompletedProcess:
    result = subprocess.run(command, cwd=repository, stdout=subprocess.PIPE, stderr=subprocess.PIPE, check=False)
    if result.returncode != 0:
        detail = (result.stderr or result.stdout).decode("utf-8", errors="replace").strip()
        raise FormatError(f"{Path(command[0]).name} failed with exit status {result.returncode}: {detail}")
    return result


def is_owned_source(relative_path: Path) -> bool:
    parts = relative_path.parts
    if not parts or parts[0] not in {"src", "examples"}:
        return False
    if any(part in {"third_party", ".git", ".idea", "build"} or part.startswith("cmake-build-") for part in parts[:-1]):
        return False
    return relative_path.suffix.lower() in SOURCE_EXTENSIONS


def collect_sources(repository: Path, requested_paths: list[str]) -> list[Path]:
    git = shutil.which("git")
    if git is None:
        raise FormatError("git was not found on PATH")
    result = run_captured([git, "-C", str(repository), "ls-files", "--cached", "--others", "--exclude-standard", "-z", "--", "src", "examples"], repository)
    sources = set()
    for entry in result.stdout.split(b"\0"):
        if not entry:
            continue
        relative_path = Path(os.fsdecode(entry))
        if not is_owned_source(relative_path):
            continue
        source = repository / relative_path
        if source.is_symlink() or not source.is_file():
            continue
        resolved = source.resolve()
        if not resolved.is_relative_to(repository) or not is_owned_source(resolved.relative_to(repository)):
            continue
        sources.add(resolved)
    ordered_sources = sorted(sources, key=lambda path: path.as_posix())
    if not requested_paths:
        return ordered_sources
    selected_sources = set()
    for value in requested_paths:
        requested = Path(value).expanduser()
        if not requested.is_absolute():
            requested = repository / requested
        requested = requested.resolve()
        if not requested.is_relative_to(repository):
            raise FormatError(f"requested path is outside the repository: {value}")
        matches = [source for source in ordered_sources if source == requested or requested in source.parents]
        if not matches:
            raise FormatError(f"no eligible project-owned source files match: {value}")
        selected_sources.update(matches)
    return sorted(selected_sources, key=lambda path: path.as_posix())


def find_clang_format(explicit_path: str | None) -> str:
    candidate = explicit_path or os.environ.get("CLANG_FORMAT") or "clang-format"
    executable = shutil.which(candidate)
    if executable is None:
        raise FormatError(f"clang-format executable was not found: {candidate}")
    return str(Path(executable).resolve())


def check_clang_format_version(executable: str, repository: Path) -> None:
    result = run_captured([executable, "--version"], repository)
    version_text = (result.stdout + result.stderr).decode("utf-8", errors="replace").strip()
    match = re.search(r"\bclang-format version\s+(\d+)", version_text)
    if match is None:
        raise FormatError(f"could not identify the clang-format version: {version_text}")
    if int(match.group(1)) < MINIMUM_CLANG_FORMAT_VERSION:
        raise FormatError(f"clang-format {MINIMUM_CLANG_FORMAT_VERSION} or newer is required; found: {version_text}")


def format_sources(executable: str, repository: Path, sources: list[Path], check: bool) -> int:
    command = [executable, f"--style=file:{repository / '.clang-format'}"]
    command.extend(["--dry-run", "--Werror"] if check else ["-i"])
    failed = False
    for start in range(0, len(sources), BATCH_SIZE):
        batch = sources[start:start + BATCH_SIZE]
        result = subprocess.run([*command, "--", *(str(source) for source in batch)], cwd=repository, check=False)
        if result.returncode != 0:
            if not check:
                raise FormatError(f"clang-format failed with exit status {result.returncode}")
            failed = True
    if failed:
        print("Formatting check failed; run this script without --check to apply the style.", file=sys.stderr)
        return 1
    action = "Checked" if check else "Formatted"
    print(f"{action} {len(sources)} project-owned source files.")
    return 0


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--check", action="store_true", help="Check formatting without modifying files; exit with status 1 if formatting is needed.")
    parser.add_argument("--clang-format", metavar="EXECUTABLE", help="clang-format executable; otherwise use CLANG_FORMAT, then PATH. Version 19 or newer is required.")
    parser.add_argument("paths", nargs="*", help="Optional files or directories relative to the repository root; only eligible Git-listed sources are selected.")
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    repository = Path(__file__).resolve().parent.parent
    try:
        if not (repository / ".clang-format").is_file():
            raise FormatError(f"repository style file is missing: {repository / '.clang-format'}")
        sources = collect_sources(repository, arguments.paths)
        if not sources:
            print("No project-owned source files found.")
            return 0
        executable = find_clang_format(arguments.clang_format)
        check_clang_format_version(executable, repository)
        return format_sources(executable, repository, sources, arguments.check)
    except (FormatError, OSError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
