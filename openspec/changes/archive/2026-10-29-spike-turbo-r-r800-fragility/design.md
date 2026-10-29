# Design

## Context

See `proposal.md` - Why. Static review produced a ranked list of candidate R800/turbo R failure points. This spike turns that list into evidence: each path runs the same workload with turbo off and turbo on, on the turbo R class machine, and records whether behavior changes.

## Goals / Non-Goals

**Goals:**

- Produce a reproducible breakage matrix for the candidate paths.
- Capture evidence (RAM/VRAM artifacts, text, screenshots, logs, audio observations) for each path.
- Identify the likely mechanism for each failure.
- End with a concrete split of `harden-turbo-r-r800-compatibility` into priority and optional changes.

**Non-Goals:**

- Fixing kernel behavior in this change.
- Exhaustive turbo R performance benchmarking.
- Replacing the regression suite or wiring diagnostic programs into CI.
- Covering R800 DRAM mode as a supported feature.

## Decisions

### D1: One isolated scenario per candidate

Create a small BASIC program per path. Each program checks `MSX() = 3`, runs the workload with `CMD TURBO 0`, repeats with `CMD TURBO 1`, and prints stable markers. Timing values are not asserted.

### D2: Use the regression runner for capture

Run the programs through the local regression workflow on `Panasonic_FS-A1GT`, using volatile programs and text expectations. Use the `msx-memory-debug` skill for extra RAM/VRAM captures when the runner output is not enough. Real hardware runs are recorded separately when available.

### D3: Isolate risky paths

`CMD TURBO 2` runs last and in its own program so a hang or memory corruption cannot hide the other results. The ARKOS player test runs with the existing ARKTRK resources and does not assert audio, only completion/status. The PSG probe runs as a small assembly resource loaded with `CMD RSCTORAM`/`DEF USR`; it reads registers back through BIOS `RDPSG` to isolate write timing from read timing.

### D4: Record a breakage matrix

The report contains one row per candidate with: path, turbo-off result, turbo-on result, broken yes/no/not-reproduced, evidence path, suspected mechanism, and proposed priority. A path that passes in openMSX but is still suspect on hardware is marked `not reproduced` rather than `safe`.

### D5: Split the harden change at the end

The final task creates `harden-turbo-r-r800-priority` with only confirmed breakage and `harden-turbo-r-r800-optional` with the remaining hardening. It also updates the original `harden-turbo-r-r800-compatibility` to point at the two new changes and marks it superseded.

## Risks / Trade-offs

- **Emulator fidelity** -> openMSX may not reproduce PSG write timing; record that limitation and require hardware validation for the PSG row.
- **Timing flakiness** -> Use stable markers and fixed loops; run each program multiple times when a failure appears.
- **Resource/path mistakes** -> Reuse the existing ARKTRK and MTF assets; verify relative `FILE` paths in the compiler.
- **Temporary test churn** -> Keep the diagnostic programs clearly named and either promote or remove them when the harden changes are created.
- **`CMD TURBO 2` corruption** -> Run it in isolation and stop at the first anomaly.

## Migration Plan

1. Run the diagnostic programs on openMSX `Panasonic_FS-A1GT`.
2. Repeat on real hardware where available; record the machine and BIOS.
3. Write the breakage matrix and evidence index under the change directory.
4. Create the priority and optional harden changes from the matrix.
5. Update the original harden change and mark it superseded.
6. Leave the diagnostic harness in place until the priority change decides which probes become permanent regression tests.

## Open Questions

- Is a real MSX turbo R available for the spike, or is openMSX the only environment?
- Can openMSX record audio well enough to judge ARKOS PSG glitches, or should the report rely on register probes?
- Should the temporary diagnostic programs live in `MISC/` during the spike or in a dedicated `TURBO/` scenario?
