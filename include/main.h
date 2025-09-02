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
 *     ./test/unit/test
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
#include "compiler_pt3.h"
#include "fswrapper.h"
#include "lex.h"
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

/***
 * @brief Write symbols file to use with OpenMSX
 * @param compiler compiler object
 * @param code_start code start position on RAM memory
 * @note https://github.com/openMSX/debugger/blob/master/src/SymbolTable.cpp
 */
bool saveSymbolFile(BuildOptions *opts, Compiler *compiler, int code_start);

/***
 * @brief Write symbols file to use with OpenMSX
 * @deprecated Deprecated
 * @param compiler compiler object for PT3 support (deprecated)
 * @param code_start code start position on RAM memory
 */
bool saveSymbolFile(BuildOptions *opts, CompilerPT3 *compiler, int code_start);

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
