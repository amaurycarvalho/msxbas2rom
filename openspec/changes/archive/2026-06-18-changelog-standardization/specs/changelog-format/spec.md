## ADDED Requirements

### Requirement: CHANGELOG.md follows Keep a Changelog format
The project SHALL maintain `CHANGELOG.md` following the [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) format, with the following conventions:

- Each release SHALL be formatted as `## [version] - YYYY-MM-DD`
- Changes not yet released SHALL be grouped under a `## [Unreleased]` header
- Changes within each release SHALL be categorized using these section headers (in order):
  - `### Added` for new features
  - `### Changed` for changes in existing functionality
  - `### Deprecated` for soon-to-be removed features
  - `### Removed` for removed features
  - `### Fixed` for bug fixes
  - `### Security` for vulnerability fixes
- Dates SHALL use ISO 8601 format (`YYYY-MM-DD`)
- The file SHALL start with a title and a link to the Keep a Changelog specification and Semantic Versioning specification

#### Scenario: New release entry follows format
- **WHEN** a new release entry is added to CHANGELOG.md
- **THEN** it SHALL use the header `## [version] - YYYY-MM-DD`
- **AND** its changes SHALL be grouped under the appropriate category section

#### Scenario: Unreleased changes are grouped
- **WHEN** changes are made that have not been released yet
- **THEN** they SHALL be added under `## [Unreleased]`
- **AND** they SHALL use the same category sections as released entries

### Requirement: info_history mirrors current release from CHANGELOG.md
The `info_history` string in `src/cli/appinfo.h` SHALL contain:
1. The complete entry for the current release (version, date, and all categorized changes)
2. A brief summary (1-2 lines) of each of the last 2 releases
3. A link to the current release on GitHub in the format `https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v[version]`

#### Scenario: info_history is updated for each release
- **WHEN** a new release is created
- **THEN** `info_history` in `src/cli/appinfo.h` SHALL be updated to reflect the current release entry from CHANGELOG.md
- **AND** include a summary of the previous 2 releases
- **AND** end with a link to the current release on GitHub

#### Scenario: info_history content differs from full changelog
- **WHEN** a user runs `msxbas2rom --history`
- **THEN** the output SHALL show the current release entry, last 2 releases summary, and release link
- **AND** NOT show the full changelog history

### Requirement: Version is synced across all release files
When a new version is released, the version number SHALL be updated in all of the following files:
- `src/cli/appinfo.h` — the `app_version` constant
- `debian/changelog` — Debian packaging changelog entry
- `rpmbuild/SPECS/msxbas2rom.spec` — RPM spec Version field and %changelog entry

#### Scenario: Version update propagates to all files
- **WHEN** a new release version is set in `src/cli/appinfo.h`
- **THEN** `CHANGELOG.md`, `debian/changelog`, and `rpmbuild/SPECS/msxbas2rom.spec` SHALL have a matching version entry

#### Scenario: Release checklist verifies sync
- **WHEN** preparing a new release
- **THEN** the version SHALL be verified as consistent across `appinfo.h`, `CHANGELOG.md`, `debian/changelog`, and `rpmbuild/SPECS/msxbas2rom.spec`

### Requirement: Version placeholder converted to Unreleased
Entries in CHANGELOG.md with version `#.#.#.#` (placeholder for internal changes without a release tag) SHALL be stored under the `## [Unreleased]` section using the same categorization format.

#### Scenario: Unreleased entry preserves historical context
- **WHEN** reformatting CHANGELOG.md to Keep a Changelog format
- **THEN** any `#.#.#.#` entries SHALL be moved under `## [Unreleased]` with their original content and dates preserved in parentheses
