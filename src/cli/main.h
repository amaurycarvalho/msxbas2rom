/***
 * @file main.h
 * @brief MSXBAS2ROM compiler CLI main header
 * @author Amaury Carvalho
 * @copyright (GNU GPL3) 2019~
 * @see
 *   MSX BASIC to ROM compiler project:
 *     https://github.com/amaurycarvalho/msxbas2rom/
 *   Contact email:
 *     amauryspires@gmail.com
 * @note
 *   Unit testing:
 *     make test-unit
 * @example
 *   msxbas2rom -h
 *   msxbas2rom --ver
 *   msxbas2rom --doc
 *   msxbas2rom program.bas
 *   msxbas2rom -x program.bas
 *   msxbas2rom -x --scc program.bas
 */

#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include "appinfo.h"
#include "cliparser.h"
#include "compiler.h"
#include "fswrapper.h"
#include "lexer.h"
#include "options.h"
#include "rom.h"

/***
 * @brief msxbas2rom [options] <filename.bas>
 * @example
 *   msxbas2rom program.bas
 *   msxbas2rom -x program.bas
 *   msxbas2rom -x --scc program.bas
 */
int main(int argc, char *argv[]);

//! @brief Print header
void printHeader();

//! @brief Print help
void printHelp();

//! @brief Print version
void printVersion();

//! @brief Print history
void printHistory();

//! @brief Print reference guide
void printDocs();

//! @brief Print footer
void printFooter();

#endif  // MAIN_H_INCLUDED
