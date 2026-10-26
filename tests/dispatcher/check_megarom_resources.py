#!/usr/bin/env python3
"""Run a MegaROM DATA/IDATA program in openMSX.

Exercises MR_CALL/MR_GET_DATA segment handling through the resource map.
"""

import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

from emulator import APP, MAPPER_BLOCK_8K, line_breakpoint, run_capture

ROOT = Path(__file__).resolve().parent.parent.parent
SOURCE = ROOT / "tests" / "integration" / "DATA" / "test1.bas"


def main():
    if shutil.which("openmsx") is None and "OPENMSX_FLATPAK_APP" not in os.environ:
        print("[SKIP] openMSX is not installed")
        return 0
    if not APP.exists():
        print(f"[SKIP] release binary not found: {APP}")
        return 0

    with tempfile.TemporaryDirectory() as directory:
        workdir = Path(directory)
        source = workdir / "megarom_resources.bas"
        source.write_text(
            SOURCE.read_text(encoding="utf-8") +
            "\n70 POKE 57344,90\n80 GOTO 80\n",
            encoding="utf-8",
        )

        result = subprocess.run(
            [str(APP), "-q", "-8", "-s", "--noi", str(source)],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        if result.returncode != 0:
            print("[FAIL] MegaROM resource program did not compile")
            print(result.stdout[-2000:])
            return 1

        rom = workdir / "megarom_resources[ASCII8].rom"
        if not rom.exists():
            print("[FAIL] ROM was not created")
            return 1

        noi = workdir / "megarom_resources[ASCII8].noi"
        ram = run_capture(rom, 12, line_breakpoint(noi, 80, MAPPER_BLOCK_8K))
        if not ram.exists() or ram.read_bytes()[0x2000] != 0x5A:
            print("[FAIL] MegaROM resource program did not reach its marker")
            return 1

    print("[PASS] MegaROM resource/MR segment handling verified in openMSX")
    return 0


if __name__ == "__main__":
    sys.exit(main())
