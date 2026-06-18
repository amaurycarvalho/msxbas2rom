## MODIFIED Requirements

### Requirement: Follow semantic versioning
The system SHALL use semantic versioning (MAJOR.MINOR.PATCH.BUILD) with the version constant in `src/cli/appinfo.h` replicated in CHANGELOG.md, debian/changelog, and rpmbuild/SPECS/msxbas2rom.spec.

`CHANGELOG.md` SHALL follow the [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) format with ISO 8601 dates (`YYYY-MM-DD`) and categorized sections (Added, Changed, Deprecated, Removed, Fixed, Security).

The `info_history` string in `src/cli/appinfo.h` SHALL contain only the current release entry, a summary of the last 2 releases, and a link to the current release on GitHub — NOT the full changelog.

#### Scenario: Version is consistent across all files
- **WHEN** `app_version` in `src/cli/appinfo.h` is updated
- **THEN** CHANGELOG.md, debian/changelog, and rpmbuild/SPECS/msxbas2rom.spec SHALL reflect the same version

#### Scenario: CHANGELOG.md follows Keep a Changelog format
- **WHEN** CHANGELOG.md is inspected
- **THEN** it SHALL use the Keep a Changelog format with ISO 8601 dates

#### Scenario: info_history is synced with current release
- **WHEN** a release is created
- **THEN** `info_history` SHALL contain the current release entry from CHANGELOG.md, the last 2 releases summary, and the release URL
