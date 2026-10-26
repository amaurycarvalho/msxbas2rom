#!/usr/bin/env python3
"""Runtime checks for page-safe overflow modules and fixed resource modules."""

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
MARKER = 57345

MATH_PROGRAM = """\
10 A%=1+2*3
20 B%=A% AND 1
30 C$=LEFT$("ABC",1)
40 D!=SIN(0)
50 PRINT USING "##"; A%
60 PRINT TAB(5); "X"
70 POKE 57345,90
80 GOTO 80
"""

FILE_PROGRAM = """\
10 MAXFILES=2
20 POKE 57345,90
30 GOTO 30
"""

INPUT_PROGRAM = """\
10 A=PAD(0)
20 POKE 57345,90
30 GOTO 30
"""

TILE_PROGRAM = """\
10 SCREEN 2
20 SET TILE ON
30 SET TILE PATTERN 126, (128, 128, 128, 240, 0, 0, 0, 0)
40 POKE 57345,90
50 GOTO 50
"""

USR_PROGRAM = """\
FILE "string.txt.plet5"
10 POKE 57344,0
20 A=USR0(0)
30 POKE 57345,90
40 GOTO 40
"""

MTF_PROGRAM = """\
FILE "mtf.SC4Pal"
FILE "mtf.SC4Tiles"
FILE "mtf.SC4Map"
10 SCREEN 2
20 CMD MTF 0
30 CMD MTF 1
40 CMD MTF 2
50 POKE 57345,90
60 GOTO 60
"""

PLETTER_PROGRAM = """\
FILE "string.txt.plet5"
10 A$=SPACE$(100)
20 CMD RSCTORAM 0, VARPTR(A$)+1, 1
30 POKE 57345,90
40 GOTO 40
"""

COLD_PROGRAM = """\
10 CLS
20 GET TIME h%,m%,s%
30 POKE 57345,90
40 GOTO 40
"""

GAME_PROGRAM = """\
10 SCREEN 2
20 CMD DISSCR
30 CMD ENASCR
40 POKE 57345,90
50 GOTO 50
"""

RSCTORAM_PROGRAM = """\
FILE "string.bin"
10 A$=SPACE$(20)
20 CMD RSCTORAM 0, VARPTR(A$)+1, 0, 2, 3
30 POKE 57345,90
40 GOTO 40
"""

DATA_PROGRAM = """\
FILE "test69.csv"
FILE "test70.txt"
10 CMD RESTORE 1
20 RESTORE 2
30 READ R$
40 POKE 57345,90
50 GOTO 50
"""

RESOURCE_FILES = {
    "string.bin": ROOT / "tests" / "integration" / "MISC" / "string.bin",
    "string.txt.plet5": ROOT / "tests" / "integration" / "MISC" / "string.txt.plet5",
    "mtf.SC4Pal": ROOT / "tests" / "integration" / "MTF" / "mtf.SC4Pal",
    "mtf.SC4Tiles": ROOT / "tests" / "integration" / "MTF" / "mtf.SC4Tiles",
    "mtf.SC4Map": ROOT / "tests" / "integration" / "MTF" / "mtf.SC4Map",
    "mtf.SC4Super": ROOT / "tests" / "integration" / "MTF" / "mtf.SC4Super",
    "test69.csv": ROOT / "tests" / "integration" / "DATA" / "test69.csv",
    "test70.txt": ROOT / "tests" / "integration" / "DATA" / "test70.txt",
}

MODES = (
    ("", "Plain"),
    ("-8", "ASCII8"),
    ("--ascii16", "ASCII16"),
    ("--ascii16x", "ASCII16X"),
    ("--konami", "Konami"),
    ("--scc", "KonamiSCC"),
)

SAFE_CASES = (
    ("34_file_handling", FILE_PROGRAM, "file.bas", 12),
    ("90-math", MATH_PROGRAM, "math.bas", 12),
    ("90-strings", MATH_PROGRAM, "strings.bas", 12),
    ("21_logic_pack", MATH_PROGRAM, "logic.bas", 12),
    ("90-print", MATH_PROGRAM, "print.bas", 12),
    ("31-cmd-tile-sprite", TILE_PROGRAM, "tile.bas", 12),
    ("31-cmd-input", INPUT_PROGRAM, "input.bas", 12),
)

FIXED_CASES = (
    ("32_usr", USR_PROGRAM, "usr.bas", 10),
    ("31-cmd-mtf", MTF_PROGRAM, "mtf.bas", 12),
    ("70_pletter", PLETTER_PROGRAM, "pletter.bas", 10),
    ("31-cmd-cold-graphics", COLD_PROGRAM, "cold.bas", 8),
    ("31-cmd-game-graphics", GAME_PROGRAM, "game.bas", 10),
    ("31-cmd-rsctoram", RSCTORAM_PROGRAM, "rsctoram.bas", 10),
    ("90-graphics", GAME_PROGRAM, "graphics.bas", 10),
)


def prepare(workdir, program, name):
    source = workdir / name
    source.write_text(program, encoding="utf-8")
    requested = {
        filename
        for filename in RESOURCE_FILES
        if f'FILE "{filename}"' in program
    }
    if "mtf.SC4Map" in requested:
        requested.add("mtf.SC4Super")
    for filename in requested:
        source_file = RESOURCE_FILES[filename]
        if not source_file.exists():
            raise RuntimeError(f"missing resource fixture: {source_file}")
        shutil.copy2(source_file, workdir / filename)
    return source


def compile_rom(workdir, source, flags):
    result = subprocess.run(
        [str(APP), "-d"] + flags + [str(source)],
        check=False,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    return result


def find_rom(workdir):
    for path in workdir.glob("*.rom"):
        return path
    return None


def run_emulator(rom, delay):
    env = os.environ.copy()
    env["MSX_DEBUG_DELAY"] = str(delay)
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
    return rom.with_name(rom.stem + ".ram-c000-ffff.bin")


def check_safe_case(module_id, program, name, delay):
    for flag, mode_name in MODES:
        with tempfile.TemporaryDirectory() as directory:
            workdir = Path(directory)
            source = prepare(workdir, program, name)
            result = compile_rom(
                workdir, source, [flag, "--force-overflow-module", module_id]
            )
            if result.returncode != 0:
                print(f"[FAIL] {module_id} {mode_name}: compilation failed")
                print(result.stdout[-2000:])
                return False
            if "placed in overflow" not in result.stdout:
                print(f"[FAIL] {module_id} {mode_name}: not placed in overflow")
                return False
            rom = find_rom(workdir)
            if rom is None:
                print(f"[FAIL] {module_id} {mode_name}: ROM not created")
                return False
            ram = run_emulator(rom, delay)
            if not ram.exists() or ram.read_bytes()[0x2001] != 0x5A:
                print(f"[FAIL] {module_id} {mode_name}: marker not set")
                return False
    return True


def check_fixed_case(module_id, program, name, delay):
    with tempfile.TemporaryDirectory() as directory:
        workdir = Path(directory)
        source = prepare(workdir, program, name)
        result = compile_rom(
            workdir, source, ["--force-overflow-module", module_id]
        )
        if result.returncode == 0:
            print(f"[FAIL] {module_id}: forced overflow was accepted")
            return False
        if "fixed and cannot be relocated" not in result.stdout:
            print(f"[FAIL] {module_id}: missing fixed-module diagnostic")
            print(result.stdout[-2000:])
            return False

    with tempfile.TemporaryDirectory() as directory:
        workdir = Path(directory)
        source = prepare(workdir, program, name)
        result = compile_rom(workdir, source, ["-8"])
        if result.returncode != 0:
            print(f"[FAIL] {module_id}: resident MegaROM build failed")
            print(result.stdout[-2000:])
            return False
        rom = find_rom(workdir)
        if rom is None:
            print(f"[FAIL] {module_id}: resident ROM not created")
            return False
        ram = run_emulator(rom, delay)
        if not ram.exists() or ram.read_bytes()[0x2001] != 0x5A:
            print(f"[FAIL] {module_id}: resident marker not set")
            return False
    return True


def check_resident_plain(module_id, program, name, delay):
    with tempfile.TemporaryDirectory() as directory:
        workdir = Path(directory)
        source = prepare(workdir, program, name)
        result = compile_rom(workdir, source, [])
        if result.returncode != 0:
            print(f"[FAIL] {module_id}: resident Plain ROM build failed")
            print(result.stdout[-2000:])
            return False
        rom = find_rom(workdir)
        if rom is None:
            print(f"[FAIL] {module_id}: resident Plain ROM not created")
            return False
        ram = run_emulator(rom, delay)
        if not ram.exists() or ram.read_bytes()[0x2001] != 0x5A:
            print(f"[FAIL] {module_id}: resident Plain ROM marker not set")
            return False
    return True


def main():
    if not APP.exists():
        print(f"[SKIP] release binary not found: {APP}")
        return 0
    if shutil.which("openmsx") is None and "OPENMSX_FLATPAK_APP" not in os.environ:
        print("[SKIP] openMSX is not installed")
        return 0

    failed = False
    for module_id, program, name, delay in SAFE_CASES:
        if check_safe_case(module_id, program, name, delay):
            print(f"[PASS] {module_id}: overflow runtime verified")
        else:
            failed = True

    for module_id, program, name, delay in FIXED_CASES:
        if check_fixed_case(module_id, program, name, delay):
            print(f"[PASS] {module_id}: fixed resident runtime verified")
        else:
            failed = True

    if check_resident_plain(
        "31-cmd-rsctoram", RSCTORAM_PROGRAM, "rsctoram.bas", 10
    ):
        print("[PASS] 31-cmd-rsctoram: resident Plain ROM verified")
    else:
        failed = True

    with tempfile.TemporaryDirectory() as directory:
        workdir = Path(directory)
        source = prepare(workdir, DATA_PROGRAM, "data.bas")
        result = compile_rom(workdir, source, ["-8"])
        if result.returncode != 0:
            print("[FAIL] DATA/RESTORE/READ: compilation failed")
            print(result.stdout[-2000:])
            failed = True
        else:
            rom = find_rom(workdir)
            ram = run_emulator(rom, 10) if rom else None
            if ram is None or not ram.exists() or ram.read_bytes()[0x2001] != 0x5A:
                print("[FAIL] DATA/RESTORE/READ: marker not set")
                failed = True
            else:
                print("[PASS] DATA/RESTORE/READ: resident MegaROM verified")

    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
