# Design

## Context

See `proposal.md` - Why. The compiler registers the END routine at `END_STMT`
and an implicit-end jump at `END_PGM`. `END_STMT` starts with a two-byte skip,
then calls `DISP_XBASIC_END`, which restores text mode before returning to BASIC.
The runner currently installs both `END_STMT + 2` and `END_PGM` breakpoints and
captures on the first hit.

`GRAPH/line1` showed an inconsistency: the captured screenshot was graphical,
but the captured VDP register block reported a text mode and the baseline used
the source fallback (SCREEN 5). The artifact and baseline must be revalidated
after the capture point is deterministic.

## Goals / Non-Goals

**Goals:**

- Capture before the end routine switches the VDP back to text mode.
- Make the selected screen mode and VRAM region policy consistent for both
  explicit and implicit ends.
- Surface source/VDP mode conflicts instead of hiding them.

**Non-Goals:**

- Changing the program's END semantics.
- Fixing unrelated test programs that draw nothing or have source-level issues.
- Replacing the V9958 decoder; see `fix-regression-machine-selection`.

## Decisions

### D1: Prefer `END_STMT + 2` for every terminal capture

`END_STMT + 2` points at the `CALL DISP_XBASIC_END` instruction, before the text
restore. Both explicit END and implicit end eventually reach it. Use it as the
primary breakpoint for both cases.

### D2: Keep `END_PGM` only as a fallback

`END_PGM` is a jump to the real END routine. Install it only when `END_STMT` is
absent from the symbol file, or when a specific program needs it. Avoid
installing both by default because the first hit wins and may produce an
inconsistent capture.

### D3: Detect source/VDP mode conflicts

Compare the decoded VDP mode with the program's last literal SCREEN. If they
conflict, record the conflict in the run result and use the decoded mode for the
VRAM layout only when it is a supported graphical mode. Do not silently fall
back to the source mode for all END captures.

### D4: Source fallback remains for SCREEN 9-12

The V9958 register pairs observed on `Panasonic_FS-A1WX` do not distinguish
SCREEN 8 from SCREEN 9-12. For those modes only, use the program's last literal
SCREEN as the decoder fallback.

## Risks / Trade-offs

- **[Risk] `END_STMT + 2` is not reached for some implicit ends** -> Keep
  `END_PGM` as a fallback and add a regression test for a falling-off-the-end
  program.
- **[Risk] openMSX breakpoint semantics differ from the symbol address** ->
  Validate with `GRAPH/line1` and `GRAPH/sprite1`, comparing screenshot, VDP
  registers, and VRAM region selection.
- **[Risk] Programs with dynamic SCREEN changes have no reliable source mode** ->
  Prefer the VDP registers and report a conflict; such programs can use
  `requires`/`machine` and explicit VRAM regions.
- **[Risk] Baseline churn** -> `refresh-regression-baselines` regenerates the
  affected baselines after reviewer confirmation.

## Migration Plan

- Change the breakpoint selection and screen-mode policy additively.
- Re-run the affected graphical regressions and refresh baselines.
- Roll back by restoring the previous breakpoint ordering.

## Validation note

- Generated `GRAPH/line1` confirms `END_STMT=0x8063`, `END_STMT+2=0x8065`
  (`CALL DISP_XBASIC_END`), and `END_PGM=0x828B` jumping to `0x8065`.
- A post-change `GRAPH/line1` run selects the fallback capture, reports
  `R0=0x06`, `R1=0x60` (SCREEN 5), and compares the 128 KiB bitmap region. The
  hash still differs from the pre-change baseline, and the screenshot is black;
  baseline refresh handles the hash, while the black screenshot remains a
  source-level capture issue to review.

