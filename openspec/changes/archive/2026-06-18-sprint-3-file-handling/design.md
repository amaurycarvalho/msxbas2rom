## Context

File I/O support required BDOS-level integration with MSX-DOS, new kernel assembly routines, and compiler strategies for each statement/function.

## Goals / Non-Goals

**Goals:**
- Implement sequential file I/O (INPUT, OUTPUT, APPEND)
- Implement file state functions (EOF, LOC, LOF, FPOS, ERR, DSKF)
- Support MAXFILES memory allocation

## Decisions

### Decision 1: BDOS-based runtime
All file operations delegate to BDOS calls via kernel assembly routines.

### Decision 2: Memory profile switching
Non-disk mode uses HIMEM=0xF380; disk mode uses HIMEM=0xF1C9 with DSKDIS.

## Risks / Trade-offs

- File I/O reduces available RAM for user programs.
