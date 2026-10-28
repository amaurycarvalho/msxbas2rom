# Integration Tests

## Purpose

`tests/integration/` contains end-to-end and feature-level scenarios validating module interaction through real inputs.

## Structure

- Feature folders (for example `PRINT/`, `IF/`, `VAR/`) with `.bas` and related assets
- `test.sh`: integration execution entry point

## How To Run

From repository root:

```bash
make test-integration
```

Or directly:

```bash
cd tests/integration && ./test.sh
```

## Maintenance Policy

- Add cross-module behavior scenarios here.
- Keep scenario names and folder grouping aligned with language features.
- Prefer unit tests for isolated logic and integration tests for composed behavior.

## Runtime Regression (local only)

`regression/` compiles selected programs in ASCII8 mode, runs them in openMSX,
and validates RAM and VRAM instead of relying on screenshots. It is independent
from `make run`/`make test-integration` and is intentionally not wired into CI.

Requirements: openMSX installed locally, the release compiler built
(`make release`), and an X display for joystick input.

List scenarios and programs without compiling or launching openMSX:

```bash
make -C tests/integration regression-list
```

Generate or refresh baselines for all programs (default: three captures each):

```bash
make -C tests/integration regression-baseline
```

Run the regression suite:

```bash
make -C tests/integration regression
```

Select one scenario or one program:

```bash
make -C tests/integration regression SCENARIO=INPUT
make -C tests/integration regression SCENARIO=INPUT PROGRAM=test8
make -C tests/integration regression PROGRAM=test8
```

Per-program configuration lives in `regression/manifest.json` (schema in
`regression/manifest.schema.json`): machine choice, fallback time, input
scripts, RAM probes, VRAM text/region checks, VRAM byte probes, and volatile
ranges. Baselines are stored under `regression/baselines/` and are versioned
reference data. Run
artifacts and the JSON/text summary report are written under
`regression/artifacts/runs/<run-id>/` (and linked from
`regression/artifacts/runs/latest`); `regression/artifacts/` is gitignored.
Review baselines before treating them as a gate. Programs whose runtime state is
timing-dependent are marked `"volatile": true` and are checked through their
configured probes instead of a full snapshot.

Machine selection uses the least-capable class that satisfies the program:

- `msx1` uses `msx1_machine` (`Sharp_HB-8000_1.2` by default).
- `msx2` uses `msx2_machine` (`Panasonic_FS-A1WX` by default).
- `msx2plus` uses `msx2plus_machine` (`Panasonic_FS-A1WX` by default).
- `turboR` uses `turbo_r_machine` (`Panasonic_FS-A1GT` by default).

Selection precedence is an explicit `machine`, then an explicit `requires`
class, then inference. Inference chooses `msx2plus` when the highest `SCREEN`
literal is 4 or greater (with or without a space, e.g. `SCREEN 4` or `SCREEN8`)
or when `CMD TURBO`, `CALL TURBO`, or `TURBO()` appears; otherwise it chooses
`msx1`. Turbo R is never inferred, so turbo-capable programs that need it must
declare `"requires": "turboR"`. Use `make -C tests/integration regression-list`
to audit each program's class and selected machine. A baseline generated on a
different machine is rejected with a request to regenerate it.

Turbo R coverage distinguishes feature tests from performance-only usage. Feature
tests declare `"requires": "turboR"` (`MISC/test1`, `MISC/test64`,
`MISC/test65`, `MISC/turbo_gate`) and run on `Panasonic_FS-A1GT`. Programs that
use `CMD TURBO` only to speed up drawing or math (`GRAPH/tree`,
`GRAPH/mexhat`, `GRAPH/test78`, `IO/test48`, `MATH/cos`) stay on
`Panasonic_FS-A1WX` (MSX2+). Assert stable markers and labels such as
`MSX = 3` and the turbo timing labels, while leaving elapsed-time values
volatile.

```json
{
  "version": 1,
  "defaults": {},
  "scenarios": {
    "MISC": {
      "programs": {
        "test65": {
          "requires": "turboR",
          "fallback_seconds": 30,
          "vram": {
            "text": {
              "name_table": "0x1800",
              "width": 40,
              "expected": [
                "TURBO OFF ELAPSED TIME=",
                "TURBO ON ELAPSED TIME="
              ]
            }
          }
        }
      }
    },
    "GRAPH": {
      "programs": {
        "tree": {"volatile": true}
      }
    }
  }
}
```

Fallback captures wait for the greater of the program's `fallback_seconds` and
the selected machine class's boot minimum. Defaults are 10 emulated seconds for
`msx1` and 15 for `msx2`, `msx2plus`, and `turboR`. Override the minimums under
`defaults.machine_boot_seconds`; this is the supported way to adjust the
machine-specific wait. The run report and baseline record the effective value
and machine class.

Terminal captures use the pre-cleanup END entry: `END_STMT + 2`, the
`CALL DISP_XBASIC_END` instruction before the end routine restores text mode.
Both explicit `END` and implicit end reach this instruction. `END_PGM` is used
only as a fallback when `END_STMT` is unavailable. Programs that do not
terminate are captured after the effective fallback. If the captured VDP mode
conflicts with the source's last literal `SCREEN`, the report records a note and
uses the decoded graphical mode unless it is a text mode, in which case the
source graphical mode selects the VRAM layout.

```json
{
  "version": 1,
  "defaults": {
    "msx1_machine": "Sharp_HB-8000_1.2",
    "msx2plus_machine": "Panasonic_FS-A1WX",
    "turbo_r_machine": "Panasonic_FS-A1GT",
    "machine_boot_seconds": {
      "msx1": 10,
      "msx2": 15,
      "msx2plus": 15,
      "turboR": 15
    }
  },
  "scenarios": {
    "GRAPH": {
      "programs": {
        "sprite3": {"machine": "Panasonic_FS-A1WX"},
        "turbo_demo": {"requires": "turboR"}
      }
    }
  }
}
```

Example RAM probe and volatile configuration:

```json
{
  "version": 1,
  "defaults": {},
  "scenarios": {
    "PRINT": {
      "programs": {
        "test1": {
          "ram_probes": [{"address": "0xC000", "value": "0x00"}],
          "volatile_ranges": [["0xF3E0", "0xF3FF"]]
        }
      }
    }
  }
}
```

VRAM byte probes assert exact values at absolute offsets into the captured VRAM
buffer. Each probe provides an `address` plus either a `value` or a `bytes`
string. Probes are evaluated even for `"volatile": true` programs, whose
snapshot and region checks are skipped. An out-of-range probe stops the run as a
configuration error.

```json
{
  "version": 1,
  "defaults": {},
  "scenarios": {
    "PUT SPRITE": {
      "programs": {
        "or_color": {
          "machine": "Panasonic_FS-A1WX",
          "volatile": true,
          "vram": {
            "probes": [
              {"address": "0x1C10", "value": "0x6C"},
              {"address": "0x1C11", "bytes": "74 74"}
            ]
          }
        }
      }
    }
  }
}
```

Text regression programs that do not issue `SCREEN` or `WIDTH` start in the
compiler's normalized `SCREEN 0` 40-column layout. Configure their text capture
with `"name_table": "0x0000"` and `"width": 40`; do not rely on the machine boot
default. Programs that set their own `SCREEN` must configure the name table and
width for that mode instead. `PRINT/test5` and `STRING/test1` are the reference
layouts, including byte probes for tabs, spaces, and concatenated results:

```bash
make -C tests/integration regression-baseline SCENARIO=PRINT PROGRAM=test5
make -C tests/integration regression-baseline SCENARIO=STRING PROGRAM=test1
make -C tests/integration regression SCENARIO=PRINT PROGRAM=test5
make -C tests/integration regression SCENARIO=STRING PROGRAM=test1
```

