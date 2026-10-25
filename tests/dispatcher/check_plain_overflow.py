#!/usr/bin/env python3
"""Run a Plain ROM program with an overflow module in openMSX."""

import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
APP = ROOT / "bin" / "Release" / "msxbas2rom"
SKILL = (
    ROOT
    / ".opencode"
    / "skills"
    / "msx-memory-debug-skill"
    / "scripts"
    / "debug-msx-memory.sh"
)

PROGRAM = """\
10 SCREEN 2
20 SET TILE ON
30 SET TILE PATTERN 126, (128, 128, 128, 240, 0, 0, 0, 0)
40 POKE 57344, 90
50 GOTO 50
"""


def main():
    if shutil.which("openmsx") is None and "OPENMSX_FLATPAK_APP" not in os.environ:
        print("[SKIP] openMSX is not installed")
        return 0
    if not APP.exists():
        print(f"[SKIP] release binary not found: {APP}")
        return 0

    with tempfile.TemporaryDirectory() as directory:
        workdir = Path(directory)
        source = workdir / "plain_overflow.bas"
        source.write_text(PROGRAM, encoding="utf-8")

        result = subprocess.run(
            [str(APP), "-d", "--force-overflow-module",
             "31-cmd-tile-sprite", str(source)],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )
        if result.returncode != 0:
            print("[FAIL] overflow program did not compile")
            print(result.stdout[-2000:])
            return 1
        if "placed in overflow" not in result.stdout:
            print("[FAIL] tile/sprite module was not placed in overflow")
            return 1

        rom = workdir / "plain_overflow.rom"
        if not rom.exists():
            print("[FAIL] ROM was not created")
            return 1

        env = os.environ.copy()
        env["MSX_DEBUG_DELAY"] = "10"
        env["MSX_DEBUG_MACHINE"] = "Sharp_HB-8000_1.2"
        if not env.get("DISPLAY") and not env.get("WAYLAND_DISPLAY"):
            env.setdefault("SDL_VIDEODRIVER", "dummy")

        subprocess.run(
            ["bash", str(SKILL), str(rom)],
            check=True,
            env=env,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

        ram = (workdir / "plain_overflow.ram-c000-ffff.bin").read_bytes()

    if ram[0x2000] != 0x5A:
        print(
            f"[FAIL] overflow routine did not run: "
            f"0xE000 = 0x{ram[0x2000]:02X}"
        )
        return 1

    print("[PASS] Plain ROM overflow module verified in openMSX")
    return 0


if __name__ == "__main__":
    sys.exit(main())
