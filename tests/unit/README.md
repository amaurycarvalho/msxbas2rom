# Unit Tests

## Purpose

`tests/unit/` contains module-focused automated tests for core components.

## Structure

- `src/`: unit test sources (`test_*.cpp`)
- `Makefile`: build and run support for unit binaries
- `bin/`, `obj/`, `tmp/`: generated artifacts

## How To Run

From repository root:

```bash
make test-unit
```

Or directly:

```bash
make -C tests/unit all
cd tests/unit && ./bin/test_lexer
```

## Maintenance Policy

- Add new module-level automated tests here.
- Keep tests deterministic and isolated.
- Do not store integration scenarios or large fixture datasets in this folder.
