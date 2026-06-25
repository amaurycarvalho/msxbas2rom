/***
 * @file appinfo.h
 * @brief Application information header
 * @author Amaury Carvalho (2025)
 */

#ifndef APPINFO_H
#define APPINFO_H

/***
 * @brief Compiler current version
 * @warning Changelog documentation is automated by the openspec-changelog
 * skill. Run /opsx-changelog after archiving changes to update all files. Do
 * NOT manually edit CHANGELOG.md, debian/changelog, rpm spec, or info_history.
 * @remark Use /release-version skill to update app_version.
 * When it's ready for publishing, use manually /release-push skill to create a
 * new release on github or, alternatively, run the code below on terminal:
 * git tag -a v0.0.0.0 -m "Release 0.0.0.0" && git push origin --tags
 * git checkout -b release/v0.0.0.0 && git push origin release/v0.0.0.0
 * git switch master
 */
#define app_version "1.2.0.0"

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
    -h or --help = help
    -q or --quiet = quiet (no verbose)
    -d or --debug = debug mode (show details)
    -D or --doc = display quick reference guide
    -H or --history = display app history
    -v or --version = display app version

Compile options (optional):
    -c  = plain ROM compile mode (default)
    -a or --auto = auto mode (fallback to ASCII8 when plain ROM overflows)
    -x or -8 or --ascii8 = ASCII8 MegaROM compile mode
    -6 or --ascii16 = ASCII16 MegaROM compile mode
    -7 or --ascii16x = ASCII16-X MegaROM compile mode
    -4 or --konami = Konami MegaROM compile mode
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
 * @warning info_history is updated by the openspec-changelog skill.
 * Run /opsx-changelog to update. Do NOT edit manually.
 * See openspec/specs/governance/spec.md for the full rule.
 */
const char* info_history = R"(
Version history

## [1.2.0.0] - 2026-06-25

Release 1.2.0.0: MTF window-copy support with extended parameter handling

See full changelog:
https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v1.2.0.0
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
