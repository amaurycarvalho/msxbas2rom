# Tasks

## 1. Kernel fix

- [x] 1.1 Change the MSX2 `AND 0FH` to `AND 7FH` in `90-graphics.asm` and leave
  the MSX1 branch unchanged; verify the kernel builds and the instruction size is
  unchanged.
- [x] 1.2 Rebuild the release compiler/kernel and compile `sprite3.bas`; verify
  the generated ROM writes `0x6C` to `0x1C10..0x1C1F` on `Panasonic_FS-A1WX`.

## 2. Regression integration

- [x] 2.1 Add `sprite3` to `manifest.json` with `machine: Panasonic_FS-A1WX`,
  a 15-second fallback, and a `vram.probes` entry for `0x1C10` expecting 16
  `0x6C` bytes; verify the manifest validates.
- [x] 2.2 Run `regression SCENARIO=GRAPH PROGRAM=sprite3` against the old and
  fixed kernels; verify the old kernel fails the probe and the fixed kernel
  passes.

## 3. Baseline and documentation

- [x] 3.1 Generate `baselines/GRAPH/sprite3.json` after the fix and review the
  sprite color table and attributes; verify the baseline records
  `Panasonic_FS-A1WX`.
- [x] 3.2 Add a comment to `sprite3.bas` documenting the expected `0x6C` bytes
  and the layout-specific probe address; verify the comment does not change the
  program output.
- [x] 3.3 Run `openspec validate fix-put-sprite-or-color --strict`; verify it
  passes.
