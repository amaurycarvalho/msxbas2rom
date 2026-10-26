# Design

## Context

See `proposal.md` - Why. The runner already has `compare_probes` for RAM and
`compare_vram` for text/regions. VRAM is captured as a byte array.

## Goals / Non-Goals

**Goals:**

- Add a manifest-level exact-byte VRAM assertion.
- Keep the existing text/region behavior unchanged.
- Make probe results readable and deterministic.

**Non-Goals:**

- Replacing region hashes or baseline generation.
- Deriving VRAM addresses from symbols; probes use explicit offsets.

## Decisions

### D1: Mirror the RAM probe shape

Use `vram.probes` with `address` plus `value` or `bytes`, matching
`ram_probes`. Bytes are parsed with the same hex/decimal token rules. This keeps
the manifest familiar and easy to validate.

### D2: Evaluate probes before the volatile short-circuit

`compare_vram` currently returns early for volatile programs. Move probe
evaluation before that return so a volatile program can still assert an exact
VRAM byte. The snapshot and region checks remain skipped for volatile programs.

### D3: Probe addresses are absolute VRAM offsets

The probe address is an offset into the captured VRAM buffer, not a CPU address.
The runner validates `address < len(vram)` and `address + length <= len(vram)`
and fails fast on an invalid configuration.

### D4: Keep the existing text/region checks

Probes are additive. If `vram.text` or `vram.regions` are configured, they
continue to run as before. Probe results are added to the checks list and decide
pass/fail together with the other configured checks.

## Risks / Trade-offs

- **[Risk] Addresses are machine/layout-specific** -> Tests that use probes must
  pin the machine and document the VRAM layout; the OR-color probe uses the
  screen-4 sprite color table at `0x1C00` on `Panasonic_FS-A1WX`.
- **[Risk] A probe can be mistaken for a region check** -> Name check results
  `vram_probe_0x...` and include expected/actual bytes.

## Migration Plan

- Additive manifest field. Existing manifests continue to validate and run.
- Roll back by removing the probe code and schema field.
