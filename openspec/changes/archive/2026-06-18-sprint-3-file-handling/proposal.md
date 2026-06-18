## Why

Add file I/O support to MSXBAS2ROM, enabling programs to read from and write to files across MSX devices (disk, cassette, memory). This was a major feature gap preventing real-world MSX-BASIC programs from being compiled to ROM.

## What Changes

- Implement OPEN, CLOSE, PRINT#, INPUT#, LINE INPUT#, MAXFILES commands
- Implement EOF(), LOC(), LOF(), FPOS(), ERR(), DSKF() functions
- Add BDOS-based file I/O runtime in kernel assembly
- Implement disk vs non-disk memory profile switching
- Add INPUT#/LINE INPUT# field and line parsing modes

## Capabilities

### New Capabilities
- `file-handling`: File I/O support across MSX devices

### Modified Capabilities
- *(none)*

## Impact

- `src/application/compiler/statements/strategies/io/` — new file I/O statement strategies
- `src/application/compiler/functions/strategies/io/` — new file I/O function strategies
- `src/infrastructure/kernel/asm/src/34_file_handling.asm` — BDOS-based runtime
- `specs/user-stories/US-007` — new user story for file handling
- Release 1.0.0.0
