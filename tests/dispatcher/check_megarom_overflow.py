#!/usr/bin/env python3
"""Run MegaROM overflow programs for every supported mapper in openMSX."""

import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

from emulator import APP, line_breakpoint, mapper_block_size, run_capture

PROGRAM = """\
10 SCREEN 2
20 SET TILE ON
30 SET TILE PATTERN 126, (128, 128, 128, 240, 0, 0, 0, 0)
40 POKE 57344, 90
50 GOTO 50
"""

MODES = (
    ("-8", "ASCII8"),
    ("--ascii16", "ASCII16"),
    ("--ascii16x", "ASCII16X"),
    ("--konami", "Konami"),
    ("--scc", "KonamiSCC"),
)


def main():
    if shutil.which("openmsx") is None and "OPENMSX_FLATPAK_APP" not in os.environ:
        print("[SKIP] openMSX is not installed")
        return 0
    if not APP.exists():
        print(f"[SKIP] release binary not found: {APP}")
        return 0

    with tempfile.TemporaryDirectory() as directory:
        workdir = Path(directory)
        source = workdir / "megarom_overflow.bas"
        source.write_text(PROGRAM, encoding="utf-8")

        for flag, name in MODES:
            result = subprocess.run(
                [str(APP), "-d", "-s", "--noi", "--force-overflow-module",
             "31-cmd-tile-sprite", flag, str(source)],
                check=False,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
            )
            if result.returncode != 0:
                print(f"[FAIL] {name}: compilation failed")
                print(result.stdout[-2000:])
                return 1
            if "placed in overflow" not in result.stdout:
                print(f"[FAIL] {name}: module was not placed in overflow")
                return 1

            rom = workdir / f"megarom_overflow[{name}].rom"
            if not rom.exists():
                print(f"[FAIL] {name}: ROM was not created")
                return 1

            noi = workdir / f"megarom_overflow[{name}].noi"
            ram = run_capture(
                rom,
                12,
                line_breakpoint(noi, 50, mapper_block_size(flag)),
            )
            if not ram.exists() or ram.read_bytes()[0x2000] != 0x5A:
                print(f"[FAIL] {name}: overflow routine did not run")
                return 1

    print("[PASS] MegaROM overflow verified for all supported mappers")
    return 0


if __name__ == "__main__":
    sys.exit(main())
