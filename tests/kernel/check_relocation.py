#!/usr/bin/env python3
"""Dual-base relocation prototype for one small assembly module."""

import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

TEMPLATE = """\
org {base:#06x}
start:
  call helper
  ld hl, target
  ld de, 1234h
  ret
helper:
  ld hl, start
  ret
target:
  nop
  jr start
"""

BASES = (0x4000, 0x4100, 0x4200)


def assemble(workdir, base):
    source = workdir / f"prototype_{base:04x}.asm"
    binary = workdir / f"prototype_{base:04x}.bin"
    source.write_text(TEMPLATE.format(base=base), encoding="utf-8")
    subprocess.run(
        ["pasmo", str(source), str(binary)],
        check=True,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    return binary.read_bytes()


def diff_relocations(base_a, base_b, delta):
    if len(base_a) != len(base_b):
        raise ValueError("dual-base assemblies have different sizes")

    offsets = []
    for offset in range(len(base_a) - 1):
        value_a = base_a[offset] | (base_a[offset + 1] << 8)
        value_b = base_b[offset] | (base_b[offset + 1] << 8)
        if value_a == value_b:
            continue
        if ((value_b - value_a) & 0xFFFF) == (delta & 0xFFFF):
            offsets.append(offset)
    return offsets


def apply_relocations(data, assembly_base, final_base, offsets):
    delta = final_base - assembly_base
    result = bytearray(data)
    for offset in offsets:
        value = result[offset] | (result[offset + 1] << 8)
        adjusted = value + delta
        if adjusted < 0 or adjusted > 0xFFFF:
            raise ValueError("relocation outside the 16-bit address space")
        result[offset] = adjusted & 0xFF
        result[offset + 1] = (adjusted >> 8) & 0xFF
    return bytes(result)


def main():
    if shutil.which("pasmo") is None:
        print("[SKIP] pasmo is not installed")
        return 0

    with tempfile.TemporaryDirectory() as directory:
        workdir = Path(directory)
        at_4000 = assemble(workdir, 0x4000)
        at_4100 = assemble(workdir, 0x4100)
        at_4200 = assemble(workdir, 0x4200)

        offsets = diff_relocations(at_4000, at_4100, 0x100)
        relocated = apply_relocations(at_4000, 0x4000, 0x4200, offsets)

        if not offsets:
            print("[FAIL] dual-base diff found no relocations")
            return 1
        if relocated != at_4200:
            print("[FAIL] relocated module differs from assembly at 0x4200")
            return 1

        print(
            f"[PASS] dual-base relocation verified "
            f"({len(offsets)} relocation records)"
        )
        return 0


if __name__ == "__main__":
    sys.exit(main())
