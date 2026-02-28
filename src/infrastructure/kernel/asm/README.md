# Kernel ASM Infrastructure

## Purpose

`src/infrastructure/kernel/asm/` contains the Z80 assembly source and build flow used to generate embedded kernel headers for the C++ application.

## Structure

- `src/`: assembly sources (`header.asm`, `start.asm`, and symbols helper)
- `bin/`: generated binary artifacts (`*.bin`)
- `assets/`: reference assets, external engine data, and helper scripts
- `Makefile`: assembly-to-header build pipeline

## Build Flow

The local makefile compiles ASM and converts binaries into C headers consumed by the kernel layer:

- `src/header.asm` -> `bin/header.bin` -> `../header.h`
- `src/start.asm` -> `bin/start.bin` -> `../start.h`

From repository root:

```bash
make -C src/infrastructure/kernel/asm all
```

## Tooling

Expected tools:

- `pasmo` (Z80 assembler)
- `xxd` (binary to C header converter)
- `pletter` (declared in makefile; compression support)

## Maintenance Policy

- Keep generated artifacts in `bin/` reproducible from sources in `src/`.
- Treat files under `assets/` as supporting inputs/references for kernel-related workflows.
- Prefer updating source ASM and regenerating headers instead of manually editing generated header content.
