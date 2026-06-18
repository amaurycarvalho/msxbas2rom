## 1. Update Architecture Spec

- [x] 1.1 Update "Follow semantic versioning" requirement in `openspec/specs/architecture/spec.md` to reference Keep a Changelog format, ISO dates, and info_history sync rule
- [x] 1.2 Add scenario for CHANGELOG.md following Keep a Changelog format
- [x] 1.3 Add scenario for info_history synced with current release

## 2. Update CLI Spec

- [x] 2.1 Update CLI spec in `openspec/specs/cli/spec.md` to document that `--history` shows only current release + last 2 summary + release link
- [x] 2.2 Add scenario for history output change

## 3. Reformat CHANGELOG.md to Keep a Changelog

- [x] 3.1 Restructure CHANGELOG.md with Keep a Changelog header and format reference links
- [x] 3.2 Convert each release entry to `## [version] - YYYY-MM-DD` format with ISO dates
- [x] 3.3 Categorize all historical entries under appropriate sections (Added, Changed, Fixed, etc.)
- [x] 3.4 Convert `#.#.#.#` placeholder entries to `## [Unreleased]` section
- [x] 3.5 Add version diff links at the bottom of the file (e.g., `[version]: https://github.com/.../releases/tag/vversion`)

## 4. Update info_history in appinfo.h

- [x] 4.1 Replace the full-history `info_history` string with current release entry + last 2 releases summary + release link
- [x] 4.2 Update the `@warning` comment in `appinfo.h` to reference the new sync rule (info_history mirrors CHANGELOG.md current release)

## 5. Verify Consistency

- [x] 5.1 Verify that `app_version`, CHANGELOG.md, debian/changelog, and RPM spec all share the same current version
- [x] 5.2 Build the project and confirm `msxbas2rom --history` outputs the shortened history correctly
- [x] 5.3 Run unit and integration tests (`make test`)
