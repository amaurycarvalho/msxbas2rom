/***
 * @file appinfo.h
 * @brief Application information header
 * @author Amaury Carvalho (2025)
 */

#ifndef APPINFO_H
#define APPINFO_H

/***
 * @brief Compiler current version
 * @warning Before each release, sync CHANGELOG.md (Keep a Changelog format),
 * then update info_history below to mirror the current release entry,
 * add a summary of the last 2 releases, and the release URL.
 * Also update release version in debian/changelog and
 * rpmbuild/SPECS/msxbas2rom.spec.
 * @remark Run the code below to create a new release on github:
 * git tag -a v0.0.0.0 -m "Release 0.0.0.0" && git push origin --tags
 * git checkout -b release/v0.0.0.0
 * git push origin release/v0.0.0.0
 * git switch master
 */
#define app_version "1.0.0.0"

//! @brief Header information
const char* info_header = R"(MSXBAS2ROM - MSX-BASIC TO ROM COMPILER
Created by Amaury Carvalho (2020-2026)
Version:)";

//! @brief Footer information
const char* info_footer = R"(
Help us to maintain this project, learn how:
https://github.com/amaurycarvalho/msxbas2rom/wiki/Contributing )";

//! @brief Help information (-h parameter)
const char* info_help = R"(
Usage: msxbas2rom [options] <filename.bas>

General options:
    -h or --help or -? = help
    -q or --quiet = quiet (no verbose)
    -d or --debug = debug mode (show details)
    -D or --doc = display quick reference guide
    -H or --history = display app history
    -v or --version = display app version

Compile options (optional):
    -c  = plain ROM compile mode (default)
    -a or --auto = auto mode (fallback to ASCII8 when plain ROM overflows)
    -x or -8 or --ascii8 = ASCII8 MegaROM compile mode
    -k or --scc = Konami SCC MegaROM compile mode

Path options (optional)
    -i  = input path (default=source file path)
    -o  = output path (default=source file path)

Special options:
    -s generate symbols for Z80 debugging (default format: .noi)
    --noi or --noice = generate symbols in .noi format (openMSX)
    --cdb = generate symbols in .cdb format (sdcc)
    --symbol = generate symbols in .symbol format (pasmo)
    --omds = generate symbols in .omds format (openMSX deprecated)
    --lin = write the MSX-BASIC line numbers in the binary code
    --vscode = initialize a VSCode MSX-BASIC project in the current path

Output: <filename.rom>

See more information at:
https://github.com/amaurycarvalho/msxbas2rom/wiki/Usage
)";

/***
 * @var
 * @brief History information (--ver parameter)
 * @warning info_history MUST mirror the current release entry from
 * CHANGELOG.md, include a summary of the last 2 releases, and the release URL.
 * CHANGELOG.md is the source of truth — keep it in sync before each release.
 * See openspec/specs/changelog-format/spec.md for the full rule.
 */
const char* info_history = R"(
Version history

## [1.0.0.0] - 2026-05-04

### Added
- File handling support implementation (US-007).
- Spec migration to OpenSpec format.
- Changelog standardized to Keep a Changelog format.

## [0.3.4.2] - 2026-04-14

Sprite glitch bug fix (issue #19) and Z80 kernel SRP refactoring.

## [0.3.4.1] - 2026-04-09

ELF export bug fix.

See full changelog:
https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.0.0.0
)";

/***
 * @brief Documentation information (--doc parameter)
 */
const char* info_documentation = R"(
See a complete reference guide on project wiki page:
https://github.com/amaurycarvalho/msxbas2rom/wiki/Documentation

SUPPORTERS SPECIAL THANKS

  - Luciano Cadari (iplay.com.br);
  - Mario Cavalcanti (clubemsx.com.br);
  - Pedro de Medeiros;
  - Claudio Pinheiro;
  - Diogo Patrao;
  - Paulo Goncalves;
  - Gilberto Taborda;
  - Ronaldo Prado;
  - Julio Berrincha.

Know more at:
https://github.com/amaurycarvalho/msxbas2rom/wiki
)";

#endif  // APPINFO_H
