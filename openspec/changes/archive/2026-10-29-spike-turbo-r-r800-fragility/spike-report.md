# Turbo R / R800 Fragility Spike Report

## Environment

- Emulator: openMSX 19.1 (Debian build)
- Machine: `Panasonic_FS-A1GT` (turbo R class), selected through the local regression manifest
- Runner: `tests/integration/regression/runner.py` with `make -C tests/integration regression SCENARIO=MISC PROGRAM=<name>`
- Real hardware: **not available in this environment**. Every runtime row below is openMSX evidence; hardware confirmation is called out separately.
- Capture trigger: the runner breakpoint at `END_STMT+2`, except where noted. RAM range is `0xC000-0xFFFF`, VRAM is the full 128 KiB.

## Breakage Matrix

| Candidate | Path exercised | Turbo off | Turbo on | Classification | Evidence | Suspected mechanism | Priority |
|---|---|---|---|---|---|---|---|
| ARKOS PSG writes | `CMD PLYLOAD`/`PLYSONG`/`PLYPLAY`/`PLYMUTE`; direct PSG writes through `psg_probe.asm` | Player completed; PSG read-back `AA,05,0F` | Player completed; PSG read-back `AA,05,0F` | `not reproduced` | `tests/integration/regression/artifacts/runs/20261029-105732` (player), `.../20261029-105840` (PSG probe) | Player writes PSG select/data without a software wait. openMSX does not model PSG sub-instruction timing, so a pass is not evidence of hardware safety. | Hardware validation; optional delay |
| VDP resource copy | `CMD WRTVRAM` / `CMD VRAMTORAM` round-trip of a 128-byte pattern at VRAM `0x2000` | `VDPCOPY OFF=OK` | `VDPCOPY ON=OK` | `safe` | `.../20261029-105845` | The S1990 inserts the VDP wait for `LDIRVM`/`LDIRMV`, so the block copy is not timing-sensitive in this path. | None |
| Partial MTF window | `CMD MTF` partial window path (`SUB_LDIRMV`, `INI`) | Not exercised by a dedicated spike program | Not exercised by a dedicated spike program | `not reproduced` | No dedicated run; adjacent VDP block copy passed in `.../20261029-105845` | Partial MTF uses `INI` against the VDP, which should also be covered by the S1990 wait; unproven in this spike. | Add a partial-MTF probe to the optional change |
| VDP graphics/scroll | `SCREEN 5`, `PSET`, `LINE`, `CIRCLE`, `SET SCROLL`, sampled with `POINT` | `GFX OFF=OK` | `GFX ON=OK` | `safe` | `.../20261029-105727` | Graphics commands sequence VDP I/O through protected VDP accesses; sampled pixels and scroll setup matched. | None |
| Turbo mode selection | `CMD TURBO 2`, then `TURBO()` and an integer RAM canary | `TURBO2 START=0` | `TURBO2 AFTER=2`, canary `OK`, then `CMD TURBO 0` recovered | `broken` | `.../20261029-105852`, `.../20261029-105854`, `.../20261029-105856` | `CMD TURBO` forwards the argument to `CHGCPU` without clamping; value 2 selects R800 DRAM mode. The canary survived here, but the mode is outside the supported contract. | Priority: clamp to 0/1 |
| Mouse PSG read | `PAD(0)`, `PAD(12)`, `PAD(13)`, `PAD(14)` | `PAD OFF= 0,-1,1,1` | `PAD ON= 0,-1,1,1` | `not reproduced` | `.../20261029-105737` | Values match across modes, but no mouse movement/button input was injected, so the PSG read path was not stressed. | Hardware/mouse validation; optional |
| Integer math | Multiply, divide, modulo, `TURBO()` | `MUL=3852 DIV=321 MOD=0`, `TURBO=0` | `MUL=3852 DIV=321 MOD=0`, `TURBO=1` | `safe` | `.../20261029-105745` | Integer results stayed identical; the R800 `MULUW`-related path did not change observable output in this workload. | None |
| `SLL` use | Static review of the remaining observable use | Not applicable | Not applicable | `safe` | Static review only | R800 executes `SLL` as `SLA`; the reviewed result is not consumed today, so observable behavior is unchanged. | Optional cleanup |

`broken` means the specified behavior failed and the failure reproduced. `not reproduced` means the path was either not exercised deeply enough or cannot be trusted from emulator evidence alone. `safe` means the path produced the same observable result and the mechanism is understood.

## Reproduced Breakage

`CMD TURBO 2` is the only confirmed breakage. Three independent runs produced:

```
TURBO2 START= 0
TURBO2 AFTER= 2
TURBO2 CANARY=OK
TURBO2 DONE
```

The three RAM and VRAM captures were byte-identical:

| Run | RAM SHA-256 (first 16) | VRAM SHA-256 (first 16) |
|---|---|---|
| `20261029-105852` | `888a94b66669b864` | `c6a25af552955fc9` |
| `20261029-105854` | `888a94b66669b864` | `c6a25af552955fc9` |
| `20261029-105856` | `888a94b66669b864` | `c6a25af552955fc9` |

The canary and the later `CMD TURBO 0` show that openMSX recovered, but the runtime contract still forbids reporting mode 2.

## openMSX Fidelity Limits

- **PSG sub-instruction timing:** openMSX schedules PSG writes at instruction granularity. A tight direct `OUT` loop can pass even when a real turbo R would drop a select/data pair. The PSG and ARKOS rows therefore require hardware confirmation, and the ARKOS row needs an audio capture rather than only completion markers.
- **Partial MTF:** no dedicated partial-window program ran in this spike. The VDP resource-copy result is only indirect evidence for the `INI`-based MTF helper.
- **Mouse PSG reads:** the emulator run had no mouse movement or button events. Equal `PAD()` values show no mode-dependent difference in the idle state, not that the read path is timing-safe.
- **Hardware rows:** ARKOS PSG writes, the PSG timing probe, partial MTF, and mouse PSG reads require a real MSX turbo R (machine model and BIOS version recorded with the evidence) before they can move from `not reproduced` to `safe` or `broken`.
- **No real hardware available:** this spike could not satisfy the hardware repeat. The openMSX evidence is complete for the paths above; the hardware follow-up remains open in the optional change.

## Priority Recommendation

1. **Priority (`harden-turbo-r-r800-priority`)**: clamp `CMD TURBO` on turbo R to 0/1 and verify `TURBO()` never reports 2, with the RAM canary intact. This is the only confirmed breakage.
2. **Optional (`harden-turbo-r-r800-optional`)**: hardware PSG timing validation and an R800-aware delay, ARKOS audio validation, a dedicated partial-MTF probe, mouse input coverage, and the `SLL` cleanup.

## Diagnostic Harness Notes

- The temporary programs were `turbo_spike_vdp_copy.bas`, `turbo_spike_vdp_graphics.bas`, `turbo_spike_player.bas`, `turbo_spike_pad.bas`, `turbo_spike_math.bas`, `turbo_spike_turbo2.bas`, and `turbo_spike_psg_timing.bas` under `tests/integration/MISC/`.
- Resources were `turbo_spike_pattern.bin` (Pletter-compressed because `CMD WRTVRAM` always unpacks its resource) and `psg_probe.asm`/`psg_probe.bin`.
- The `CMD TURBO 2` diagnostic was promoted to the permanent `tests/integration/MISC/turbo_mode_clamp.bas` in `harden-turbo-r-r800-priority`. The other temporary programs and manifest entries were removed after the evidence was recorded; the run artifacts above remain the reproducible evidence.
- The `CMD TURBO 2` test runs in its own program so a hang or corruption cannot mask the other results.
