#!/usr/bin/env python3
"""Static game-graphics dispatch regression and performance notes."""

import re
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
APP = ROOT / "bin" / "Release" / "msxbas2rom"
SOURCE = ROOT / "tests" / "integration" / "GRAPH" / "test94.bas"
REPORT = ROOT / "tests" / "integration" / "performance-notes.md"


def run(extra):
    with tempfile.TemporaryDirectory() as directory:
        return subprocess.run(
            [str(APP), "-d", "-8", "-o", directory] + extra + [str(SOURCE)],
            check=False,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )


def measure(output, label):
    code = re.search(r"Compiled code size = (\d+)", output)
    rom = re.search(r"ROM size = (\d+)K", output)
    return {
        "label": label,
        "resident": "placement 31-cmd-tile-sprite resident" in output,
        "overflow": "placement 31-cmd-tile-sprite overflow" in output,
        "paged": output.count("Paged dispatch"),
        "code": int(code.group(1)) if code else 0,
        "rom": int(rom.group(1)) if rom else 0,
    }


def main():
    if not APP.exists():
        print(f"[SKIP] release binary not found: {APP}")
        return 0

    baseline = run([])
    forced = run(["--force-overflow-module", "31-cmd-tile-sprite"])
    if baseline.returncode != 0 or forced.returncode != 0:
        print("[FAIL] graphics dispatch perf build failed")
        print((baseline.stdout + forced.stdout)[-3000:])
        return 1

    resident = measure(baseline.stdout, "resident")
    overflow = measure(forced.stdout, "forced-overflow")

    if not resident["resident"] or resident["paged"] != 0:
        print("[FAIL] resident graphics baseline is not paged-dispatch free")
        return 1
    if not overflow["overflow"] or overflow["paged"] == 0:
        print("[FAIL] forced graphics did not produce paged calls")
        return 1

    REPORT.write_text(
        "# Graphics Dispatch Performance Notes\n\n"
        "Program: `tests/integration/GRAPH/test94.bas` (ASCII8 MegaROM).\n\n"
        "| Mode | Tile/sprite graphics | Paged calls | Code size | ROM size |\n"
        "| --- | --- | ---: | ---: | ---: |\n"
        f"| Resident baseline | resident | {resident['paged']} | "
        f"{resident['code']} | {resident['rom']}K |\n"
        f"| Forced overflow | overflow | {overflow['paged']} | "
        f"{overflow['code']} | {overflow['rom']}K |\n\n"
        "Each paged call uses a 6-byte `call dispatcher; db segment; dw address`\n"
        "stub instead of a 3-byte `jp` resident stub. The dispatcher saves six\n"
        "register pairs, saves and restores `SGMADR`, and switches the mapper\n"
        "segment through `MR_CHANGE_SGM` on every call. The current dispatcher has\n"
        "no segment cache, so it always switches and restores even when the target\n"
        "segment is already active. The resident baseline has no dispatcher path;\n"
        "a resident call is a single `jp` (about 10 T-states). The paged path is\n"
        "therefore a deliberate performance trade-off for programs that overflow\n"
        "the relocatable tile/sprite graphics group. Game graphics itself remains\n"
        "pinned because pinned core callers reference it directly; moving it would\n"
        "require central-table routing of those callers.\n",
        encoding="utf-8",
    )
    print(
        f"[PASS] graphics perf: resident paged={resident['paged']}, "
        f"forced paged={overflow['paged']}"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
