# Integration Tests

## Purpose

`tests/integration/` contains end-to-end and feature-level scenarios validating module interaction through real inputs.

## Structure

- Feature folders (for example `PRINT/`, `IF/`, `VAR/`) with `.bas` and related assets
- `test.sh`: integration execution entry point

## How To Run

From repository root:

```bash
make test-integration
```

Or directly:

```bash
cd tests/integration && ./test.sh
```

## Maintenance Policy

- Add cross-module behavior scenarios here.
- Keep scenario names and folder grouping aligned with language features.
- Prefer unit tests for isolated logic and integration tests for composed behavior.
