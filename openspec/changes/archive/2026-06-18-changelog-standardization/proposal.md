## Why

The project needs a standardized, human-readable changelog format following [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and a clear sync rule between `CHANGELOG.md` and the `info_history` string in `appinfo.h`. Currently the changelog is a flat list without categorization, dates use non-ISO format, and `info_history` duplicates the entire history as a C string literal — inflating the binary and creating maintenance burden.

## What Changes

- Convert `CHANGELOG.md` to Keep a Changelog format with categorized sections (Added, Changed, Fixed, Removed, Security)
- Migrate date format from `(YYYY/MM/DD)` to ISO `YYYY-MM-DD`
- Convert placeholder `#.#.#.#` entries into `## [Unreleased]` sections
- Update `info_history` (in `appinfo.h`) to contain only: current release details + summary of the last 2 releases + link to the current release on GitHub
- Update the architecture spec to document the new changelog format and sync conventions
- Update `appinfo.h` comments to reflect the new sync rule

## Capabilities

### New Capabilities
- `changelog-format`: Defines the Keep a Changelog format standard, the info_history sync rule (current release + last 2 summary + release URL), the ISO date format requirement, and the Unreleased section convention.

### Modified Capabilities
- `architecture`: The "Follow semantic versioning" requirement (L91-96) needs to be updated to document the new changelog format and the info_history sync rule.
- `cli`: The `--history` flag behavior changes because `info_history` will no longer show the full changelog — only current release + last 2 summary + release link.

## Impact

- `CHANGELOG.md` — complete reformat
- `src/cli/appinfo.h` — `info_history` string shortened, comments updated
- `openspec/specs/architecture/spec.md` — new requirements for changelog format and sync
- `openspec/specs/cli/spec.md` — delta for `--history` behavior change
- Release process — new step to keep info_history in sync with CHANGELOG.md
