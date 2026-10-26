# Tasks

## 1. Regenerate affected baselines

- [x] 1.1 Regenerate MSX2+ machine baselines: `IO/test5`, `MTF/mtf2`,
  `GRAPH/test96`, `GRAPH/base2`, `GRAPH/tree`; verify each records
  `Panasonic_FS-A1WX`.
- [x] 1.2 Regenerate turbo R feature baselines: `MISC/test1`, `MISC/test64`,
  `MISC/test65`; verify each records `Panasonic_FS-A1GT` and the expected
  volatile/probe configuration.
- [x] 1.3 Regenerate the screen/region-affected baselines: `GRAPH/cadbit`,
  `GRAPH/sprite1`, `GRAPH/sprite2`, `GRAPH/test38`, `GRAPH/test80`, `TS/ts7`,
  and `GRAPH/line1`; verify the recorded screen matches the decoder and capture
  point.
- [x] 1.4 Generate the new `GRAPH/sprite3` baseline after the OR-color fix;
  verify sprite 1's color-table probe and baseline both reflect `0x6C`.
- [x] 1.5 Set `MEGAROM/test51` `fallback_seconds` to 45, regenerate its
  baseline, and verify the capture occurs at `END_STMT+2` around 33.6 emulated
  seconds after the full print loop.

## 2. Review and verify

- [x] 2.1 Review every regenerated baseline's `machine`, `screen`, `regions`,
  `volatile`, and capture time; document any intentional difference.
- [x] 2.2 Run `make -C tests/integration regression SCENARIO=GRAPH` and the
  affected MISC/IO/MTF/TS selections; verify no machine-mismatch failures remain.
- [x] 2.3 Run the full `make -C tests/integration regression`; verify it passes
  or that any failure is documented as an unrelated known issue.
- [x] 2.4 Run `openspec validate refresh-regression-baselines --strict`; verify
  it passes.
- [x] 2.5 Rerun the full `make -C tests/integration regression` after the
  `MEGAROM/test51` fix; verify 164/164 pass.
