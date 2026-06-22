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

### Requirement: Changelog files are updated by openspec-changelog skill
All changelog-related files (`CHANGELOG.md`, `CHANGELOG-ARCHIVE.md`, `debian/changelog`, `rpmbuild/SPECS/msxbas2rom.spec`, `info_history` in `src/cli/appinfo.h`) are updated by the `openspec-changelog` skill. See `openspec/specs/governance/spec.md` for the authoritative rule. This spec defines only the format requirements.

#### Scenario: info_history is updated by skill
- **WHEN** a release is created
- **THEN** the `openspec-changelog` skill SHALL update `info_history` in `src/cli/appinfo.h`
- **AND** `info_history` SHALL contain the current release summary and release URL only (NOT the full changelog)

#### Scenario: Version is synced across all release files
- **WHEN** a new release version is set in `src/cli/appinfo.h`
- **THEN** `CHANGELOG.md`, `debian/changelog`, and `rpmbuild/SPECS/msxbas2rom.spec` SHALL have a matching version entry

### Requirement: Version placeholder converted to Unreleased
Entries in CHANGELOG.md with version `#.#.#.#` (placeholder for internal changes without a release tag) SHALL be stored under the `## [Unreleased]` section using the same categorization format.

#### Scenario: Unreleased entry preserves historical context
- **WHEN** reformatting CHANGELOG.md to Keep a Changelog format
- **THEN** any `#.#.#.#` entries SHALL be moved under `## [Unreleased]` with their original content and dates preserved in parentheses
