#!/usr/bin/env python3
"""Trace BASIC compilation and reject external tools or module files."""

import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
APP = ROOT / "bin" / "Release" / "msxbas2rom"

FORBIDDEN = ("pasmo", "rasm", "ld ", "gcc", "g++", "linker", ".asm")


def main():
    if shutil.which("strace") is None:
        print("[SKIP] strace is not installed")
        return 0
    if not APP.exists():
        print(f"[SKIP] release binary not found: {APP}")
        return 0

    with tempfile.TemporaryDirectory() as directory:
        workdir = Path(directory)
        source = workdir / "probe.bas"
        trace = workdir / "trace.txt"
        source.write_text('10 PRINT "SELF"\n20 END\n', encoding="utf-8")

        subprocess.run(
            ["strace", "-f", "-e", "trace=execve,openat", "-o", str(trace),
             str(APP), "-q", str(source)],
            check=False,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

        lines = trace.read_text(encoding="utf-8", errors="replace").splitlines()

    execs = [line for line in lines if "execve(" in line]
    if len(execs) != 1:
        print(f"[FAIL] expected one execve, got {len(execs)}")
        return 1

    for line in lines:
        lowered = line.lower()
        if any(token in lowered for token in FORBIDDEN):
            print(f"[FAIL] external tool or module file referenced: {line}")
            return 1

    print("[PASS] BASIC compilation invoked no external tool or module file")
    return 0


if __name__ == "__main__":
    sys.exit(main())
