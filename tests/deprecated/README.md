# Deprecated Test Assets

## Purpose

`tests/deprecated/` stores legacy artifacts and historical/manual scenarios kept for reference and occasional manual regression checks.

## Scope

- This folder is archival.
- This folder is manual-only.
- This folder is not part of CI/TDD acceptance flow.

## Maintenance Policy

- Do not add new automated tests in this folder.
- Prefer adding or moving relevant coverage to `tests/unit/` or `tests/integration/`.
- If a legacy scenario is still valuable, migrate it to active test suites and keep only historical material here.
