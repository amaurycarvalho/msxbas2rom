#!/usr/bin/env python3
"""Compile representative programs with each safe group forced to overflow."""

import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent.parent
APP = ROOT / "bin" / "Release" / "msxbas2rom"

MATH_PROGRAM = """\
10 A%=1+2*3
20 B%=A% AND 1
30 C$=LEFT$("ABC",1)
40 D!=SIN(0)
50 PRINT USING "##"; A%
60 PRINT TAB(5); "X"
70 END
"""

FILE_PROGRAM = """\
10 MAXFILES=2
20 END
"""

INLINE = {
    "34_file_handling": FILE_PROGRAM,
    "90-math": MATH_PROGRAM,
    "90-strings": MATH_PROGRAM,
    "21_logic_pack": MATH_PROGRAM,
    "90-print": MATH_PROGRAM,
}

CASES = (
    ("31-cmd-tile-sprite", "GRAPH/test94.bas"),
    ("34_file_handling", None),
    ("90-math", None),
    ("90-strings", None),
    ("21_logic_pack", None),
    ("90-print", None),
)

FIXED_MODULES = (
    "32_usr",
    "31-cmd-mtf",
    "70_pletter",
    "31-cmd-cold-graphics",
    "31-cmd-game-graphics",
    "90-graphics",
)


def main():
    if not APP.exists():
        print(f"[SKIP] release binary not found: {APP}")
        return 0

    base = ROOT / "tests" / "integration"
    with tempfile.TemporaryDirectory() as directory:
        workdir = Path(directory)
        for module_id, relative in CASES:
            if relative is None:
                source = workdir / f"{module_id}.bas"
                source.write_text(INLINE[module_id], encoding="utf-8")
            else:
                source = base / relative
                if not source.exists():
                    print(f"[SKIP] missing representative program: {relative}")
                    continue

            result = subprocess.run(
                [str(APP), "-d", "-a", "-o", str(workdir),
                 "--force-overflow-module", module_id, str(source)],
                check=False,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
            )
            if result.returncode != 0:
                print(f"[FAIL] {module_id}: compilation failed")
                print(result.stdout[-2000:])
                return 1
            needle = f"module '{module_id}' does not fit any kernel hole"
            if needle not in result.stdout:
                print(f"[FAIL] {module_id}: module was not forced to overflow")
                return 1

        fixed_source = workdir / "fixed.bas"
        fixed_source.write_text(MATH_PROGRAM, encoding="utf-8")
        for module_id in FIXED_MODULES:
            result = subprocess.run(
                [str(APP), "-d", "-o", str(workdir),
                 "--force-overflow-module", module_id, str(fixed_source)],
                check=False,
                stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT,
                text=True,
            )
            if result.returncode == 0:
                print(f"[FAIL] {module_id}: fixed overflow was accepted")
                return 1
            if "fixed and cannot be relocated" not in result.stdout:
                print(f"[FAIL] {module_id}: missing fixed-overflow diagnostic")
                return 1

    print("[PASS] safe groups overflow; fixed resource modules reject overflow")
    return 0


if __name__ == "__main__":
    sys.exit(main())
