#!/usr/bin/env python3
"""Run the dispatcher ABI prototype in openMSX and check its results."""

import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
ASM = Path(__file__).resolve().parent / "prototype.asm"
SKILL = (
    ROOT
    / ".opencode"
    / "skills"
    / "msx-memory-debug-skill"
    / "scripts"
    / "debug-msx-memory.sh"
)


def check(results, offset, expected, label):
    if results[offset] != expected:
        print(
            f"[FAIL] {label}: expected 0x{expected:02X}, "
            f"got 0x{results[offset]:02X}"
        )
        return False
    return True


def main():
    if shutil.which("pasmo") is None:
        print("[SKIP] pasmo is not installed")
        return 0
    if shutil.which("openmsx") is None and "OPENMSX_FLATPAK_APP" not in os.environ:
        print("[SKIP] openMSX is not installed")
        return 0

    with tempfile.TemporaryDirectory() as directory:
        workdir = Path(directory)
        rom = workdir / "dispatcher.rom"
        subprocess.run(
            ["pasmo", str(ASM), str(rom)],
            check=True,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )

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

        ram = (workdir / "dispatcher.ram-c000-ffff.bin").read_bytes()

    expected = [
        (0x0000, 0x5A, "magic"),
        (0x0001, 0x01, "resident stub"),
        (0x0002, 0x01, "paged stub"),
        (0x0003, 0x01, "nested call"),
        (0x0004, 0x01, "nested paged call"),
        (0x0005, 0x00, "segment restored"),
        (0x0006, 0x01, "conditional call"),
        (0x0008, 0x0A, "segment switches"),
        (0x000A, 0x01, "timer target"),
        (0x0010, 0xAA, "paged HL result low"),
        (0x0011, 0xAA, "paged HL result high"),
        (0x0012, 0xBB, "paged DE result low"),
        (0x0013, 0xBB, "paged DE result high"),
        (0x0014, 0xCC, "paged BC result low"),
        (0x0015, 0xCC, "paged BC result high"),
        (0x0080, 0x12, "target A argument"),
        (0x0082, 0xBC, "target BC low"),
        (0x0083, 0x9A, "target BC high"),
        (0x0084, 0x78, "target DE low"),
        (0x0085, 0x56, "target DE high"),
        (0x0086, 0x34, "target HL low"),
        (0x0087, 0x12, "target HL high"),
        (0x0088, 0x21, "target IX low"),
        (0x0089, 0x43, "target IX high"),
        (0x002A, 0x5A, "target A result"),
    ]

    failed = False
    if ram[0x0009] == 0:
        print("[FAIL] timer interrupt did not run")
        failed = True
    for offset, value, label in expected:
        if not check(ram, offset, value, label):
            failed = True

    if failed:
        return 1

    print("[PASS] dispatcher ABI prototype verified in openMSX")
    return 0


if __name__ == "__main__":
    sys.exit(main())
