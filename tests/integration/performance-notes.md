# Graphics Dispatch Performance Notes

Program: `tests/integration/GRAPH/test94.bas` (ASCII8 MegaROM).

| Mode | Tile/sprite graphics | Paged calls | Code size | ROM size |
| --- | --- | ---: | ---: | ---: |
| Resident baseline | resident | 0 | 981 | 128K |
| Forced overflow | overflow | 3 | 981 | 128K |

Each paged call uses a 6-byte `call dispatcher; db segment; dw address`
stub instead of a 3-byte `jp` resident stub. The dispatcher saves six
register pairs, saves and restores `SGMADR`, and switches the mapper
segment through `MR_CHANGE_SGM` on every call. The current dispatcher has
no segment cache, so it always switches and restores even when the target
segment is already active. The resident baseline has no dispatcher path;
a resident call is a single `jp` (about 10 T-states). The paged path is
therefore a deliberate performance trade-off for programs that overflow
the relocatable tile/sprite graphics group. Game graphics itself remains
pinned because pinned core callers reference it directly; moving it would
require central-table routing of those callers.
