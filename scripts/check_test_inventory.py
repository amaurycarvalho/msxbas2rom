#!/usr/bin/env python3
"""Enforce the boundary between unit tests and real filesystem work.

Unit tests must use in-memory adapters. A test file may opt into explicit
file-adapter coverage with the marker:

    // test-inventory: file-adapter

Marked adapter tests may use the filesystem, but only through an OS temporary
directory. Project-relative temporary paths are rejected so mutation runs never
pollute or depend on the repository tree.
"""

import argparse
import re
import sys
from pathlib import Path

ADAPTER_MARKER = "test-inventory: file-adapter"
DISK_IO_PATTERN = re.compile(
    r"\b(ofstream|ifstream|fopen|freopen|createPath|unlink|mkdir|_mkdir|rmdir|RemoveDirectoryA|CreateDirectoryA)\b"
)
DELETE_PATTERN = re.compile(r"\bstd::remove\b")
PROJECT_TMP_PATTERN = re.compile(r'["\']tmp[/\\]')
BAS_LITERAL_PATTERN = re.compile(r"\.bas\b")


def adapter_errors(path: Path, lines):
    errors = []
    for index, line in enumerate(lines):
        if not (DISK_IO_PATTERN.search(line) or DELETE_PATTERN.search(line)):
            continue
        if PROJECT_TMP_PATTERN.search(line):
            errors.append(
                "%s:%d: file-adapter tests must use an OS temporary "
                "directory, not project-relative tmp/" % (path, index + 1)
            )
    return errors


def unit_errors(path: Path, lines):
    errors = []
    for index, line in enumerate(lines):
        disk_io = DISK_IO_PATTERN.search(line)
        delete = DELETE_PATTERN.search(line)
        if disk_io:
            errors.append(
                "%s:%d: filesystem access requires the '%s' marker"
                % (path, index + 1, ADAPTER_MARKER)
            )
        elif delete and BAS_LITERAL_PATTERN.search(line):
            errors.append(
                "%s:%d: temporary .bas file use requires the '%s' marker"
                % (path, index + 1, ADAPTER_MARKER)
            )
    return errors


def scan_file(path: Path):
    raw = path.read_text(encoding="utf-8", errors="replace")
    lines = raw.splitlines()
    if ADAPTER_MARKER in raw:
        return adapter_errors(path, lines)
    return unit_errors(path, lines)


def check(root: Path):
    tests = root / "tests" / "unit" / "src"
    if not tests.exists():
        return []
    errors = []
    for path in sorted(tests.glob("test_*.cpp")):
        errors.extend(scan_file(path))
    return errors


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path,
                        default=Path(__file__).resolve().parent.parent)
    args = parser.parse_args(argv)
    root = args.root.resolve()

    errors = check(root)
    if errors:
        print("Test inventory check failed:")
        for error in errors:
            print("  - " + error)
        return 1

    print("Test inventory check passed: unit tests use in-memory adapters; "
          "file-adapter tests use OS temporary directories.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
