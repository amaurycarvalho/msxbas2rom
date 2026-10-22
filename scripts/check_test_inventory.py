#!/usr/bin/env python3
"""Reject temporary .bas file creation in headless unit tests.

Compilation unit tests must supply source text through an in-memory resolver.
A test file may opt into filesystem source coverage with the marker:

    // test-inventory: file-adapter

Those files are the documented explicit adapter exceptions.
"""

import argparse
import re
import sys
from pathlib import Path

ADAPTER_MARKER = "test-inventory: file-adapter"
FILE_ACCESS_PATTERN = re.compile(
    r"\b(ofstream|fopen|std::remove|unlink)\b"
)
BAS_LITERAL_PATTERN = re.compile(r"\.bas\b")


def scan_file(path: Path):
    raw = path.read_text(encoding="utf-8", errors="replace")
    if ADAPTER_MARKER in raw:
        return []
    errors = []
    lines = raw.splitlines()
    for index, line in enumerate(lines):
        if BAS_LITERAL_PATTERN.search(line) and FILE_ACCESS_PATTERN.search(line):
            errors.append(
                "%s:%d: temporary .bas file use requires the '%s' marker"
                % (path, index + 1, ADAPTER_MARKER)
            )
    return errors


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

    print("Test inventory check passed: no temporary .bas files outside "
          "explicit adapter tests.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
