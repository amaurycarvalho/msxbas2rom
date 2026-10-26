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
scripts, RAM probes, VRAM text/region checks, and volatile ranges. Baselines are
stored under `regression/baselines/` and are versioned reference data. Run
artifacts and the JSON/text summary report are written under
`regression/artifacts/runs/<run-id>/` (and linked from
`regression/artifacts/runs/latest`); `regression/artifacts/` is gitignored.
Review baselines before treating them as a gate. Programs whose runtime state is
timing-dependent are marked `"volatile": true` and are checked through their
configured probes instead of a full snapshot.

Example RAM probe and volatile configuration:

```json
{
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

