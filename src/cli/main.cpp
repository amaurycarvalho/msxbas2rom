/***
 * @file main.cpp
 * @brief MSXBAS2ROM compiler CLI main implementation
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
 *   Integration testing:
 *     make test-integration
 * @example
 *   msxbas2rom -h
 *   msxbas2rom --ver
 *   msxbas2rom --doc
 *   msxbas2rom program.bas
 *   msxbas2rom -x program.bas
 *   msxbas2rom -x --scc program.bas
 */

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <memory>
#ifndef MacOS
#include <malloc.h>
#endif
#include <math.h>

#include "main.h"

using namespace std;

int main(int argc, char* argv[]) {
  BuildOptionsSetup opts;
  unique_ptr<Lexer> lexer;
  unique_ptr<Rom> rom;
  unique_ptr<Parser> parser;
  unique_ptr<Compiler> compiler;
  bool retriedWithAscii8 = false;

  /// parsing parameters

  if (!opts.parse(argc, argv)) {
    printHeader();
    printf("ERROR: Invalid parameter(s)\n%s\n", opts.errorMessage.c_str());
    return 1;
  };

  if (!opts.quiet) {
    bool footer = false;

    /// version
    if (opts.version) {
      printVersion();
      return 0;
    }

    /// header information
    printHeader();

    /// quick reference
    if (opts.doc) {
      printDocs();
      footer = true;

      /// history
    } else if (opts.history) {
      printHistory();
      footer = true;

      /// help hint
    } else if (argc == 1 || opts.help) {
      printHelp();
      footer = true;
    }

    if (footer) {
      printFooter();
      return 0;
    }
  } else {
    if (argc == 1 || opts.help || opts.doc || opts.version || opts.history) {
      printf("ERROR: Invalid parameter for quiet mode\n");
      return 1;
    }
  }

  if (opts.inputFilename.empty()) {
    printf("ERROR: Input file name parameter is missing!\n");
    return 1;
  }

  if (!fileExists(opts.inputFilename)) {
    printf("ERROR: Input file not found!\n");
    return 1;
  }

  while (true) {
    lexer.reset(new Lexer());
    parser.reset(new Parser());
    compiler.reset(new Compiler());
    rom.reset(new Rom());

    if (fileExists(opts.outputFilename)) {
      remove(opts.outputFilename.c_str());
    }

    if (!opts.quiet)
      printf("Converting %s to %s ...\n", opts.inputFilename.c_str(),
             opts.outputFilename.c_str());

    /// LEXICAL ANALYSIS
    //! @note lexing the input file, tokenizing it

    if (!opts.quiet) printf("(1) Doing lexical analysis...\n");

    if (!lexer->load(&opts)) {
      printf("ERROR: Cannot load input file for lexical analysis\n");
      printf("%s\n", lexer->errorMessage.c_str());
      return 1;
    }

    if (!lexer->evaluate()) {
      printf("%s", lexer->errorToString().c_str());
      printf("ERROR: Lexical error at line %i\n", lexer->lineNo);
      printf("%s\n", lexer->errorMessage.c_str());
      return 1;
    }

    if (opts.debug) {
      printf("Displaying lexical analysis:\n");
      printf("%s", lexer->toString().c_str());
    }

    if (opts.compileMode == BuildOptions::CompileMode::Pcoded) {
      printf(
          "ERROR: P-code mode is now DEPRECATED\n"
          "If you really need it, try to use v0.3.3.1 release.\n"
          "You can get it at:\n"
          "https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.1");
      return 1;
    }

    /// SYNTACTIC ANALYSIS
    //! @note parsing the lexing tokens, building the syntax tree

    if (!opts.quiet) printf("(2) Doing syntactic analysis...\n");

    if (!parser->evaluate(lexer.get())) {
      if (parser->lineNo == 0)
        printf("ERROR: Cannot build the symbol tree\n");
      else {
        printf("%s", parser->errorToString().c_str());
        printf("ERROR: Syntax error at line %i\n", parser->lineNo);
      }
      return 1;
    }

    if (opts.debug) {
      printf("Displaying syntactic analysis:\n");
      printf("%s", parser->toString().c_str());
    }

    /// SEMANTIC ANALYSIS
    //! @note create assembly output

    if (!opts.quiet) printf("(3) Doing semantic analysis (compiling)...\n");

    if (parser->has_pt3) {
      printf(
          "ERROR: PT3 support is now DEPRECATED\n"
          "If you really need it, try to use v0.3.3.1 release.\n"
          "You can get it at:\n"
          "https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.1");
      return 1;
    }

    if (!compiler->build(parser.get())) {
      printf("Error: %s\n", compiler->error_message.c_str());
      if (compiler->current_tag)
        printf("%s", compiler->current_tag->toString().c_str());
      return 1;
    }

    if (!opts.quiet) {
      if (opts.megaROM) {
        if (opts.compileMode == BuildOptions::CompileMode::KonamiSCC) {
          printf("    Compiling for MegaROM format (Konami with SCC mapper)\n");
        } else
          printf("    Compiling for MegaROM format (ASCII8 mapper)\n");
      }
      printf("    Compiled code size = %i byte(s)\n", compiler->code_size);
      printf("    Memory allocated to variables = %i byte(s)\n",
             compiler->ram_size);
    }

    /// ROM OUTPUT

    if (!opts.quiet) printf("(4) Building ROM...\n");

    if (!rom->build(compiler.get())) {
      bool shouldRetryWithAscii8 =
          opts.autoROM && !retriedWithAscii8 &&
          opts.compileMode == BuildOptions::CompileMode::Plain &&
          rom->getErrorMessage().find("plain ROM limit") != string::npos;

      if (shouldRetryWithAscii8) {
        if (!opts.quiet) {
          printf(
              "    Auto mode: plain ROM overflow detected, retrying as "
              "ASCII8 MegaROM...\n");
        }
        opts.compileMode = BuildOptions::CompileMode::ASCII8;
        opts.megaROM = true;
        opts.setInputFilename(opts.inputFilename);
        retriedWithAscii8 = true;
        continue;
      }

      rom->error();
      printf("ERROR: ROM building error\n");
      return 1;
    }

    if (opts.symbols != BuildOptions::SymbolsMode::None) {
      compiler->symbolManager.saveSymbol(&opts);
    }

    break;
  }

  /// finish process

  if (!opts.quiet) {
    if (opts.megaROM) {
      if (opts.compileMode == BuildOptions::CompileMode::KonamiSCC) {
        printf(
            "    MegaROM mode activated (Konami with SCC "
            "mapper).\n");
      } else
        printf("    MegaROM mode activated (ASCII8 mapper).\n");
    } else {
      printf("    Plain ROM mode activated.\n");
    }
    printf("    ROM size = %.0fK (%.1f%% free)\n", rom->romSize / 1024.0,
           100.0 - rom->codeShare - rom->resourcesShare - rom->kernelShare);

    if (compiler->resourceManager.resources.size()) {
      printf("    Resources occupied %.1f%% of avaliable space\n",
             rom->resourcesShare);
      printf("      %i resource(s) found (%.1fK size",
             (int)compiler->resourceManager.resources.size(),
             compiler->resourceManager.resourcesPackedSize / 1024.0);
      if (compiler->resourceManager.resourcesPackedSize <
          compiler->resourceManager.resourcesUnpackedSize) {
        printf(", %.1f%% packed rate", compiler->resourceManager.packedRate);
      }
      printf(")\n");
      if (opts.debug) printf("%s", compiler->resourceManager.toString().c_str());
    }

    printf("    Kernel code occupied %.1f%% of avaliable space\n",
           rom->kernelShare);
    if (compiler->font) {
      printf("        Built-In fonts included in the kernel\n");
    }
    printf("    Compiled code occupied %.1f%% of avaliable space\n",
           rom->codeShare);

    printf("    RAM usage will be %.1f%% of avaliable capacity\n",
           compiler->ramMemoryPerc);

    printf("    Symbols file created for debugger support\n");

    printf("Compilation finished with success.\n");

    printf("\nIncluded into this ROM:\n");
    printf("- XBASIC support routines, copyright by J.Suzuki 1989.\n");
    printf("- Pletter v0.5c1 by XL2S Entertainment 2008.\n");
    if (compiler->akm) {
      printf(
          "- Arkos Tracker 2 minimalist player v2.0.1 by Julien Névo, "
          "2021\n");
    }
    if (compiler->has_tiny_sprite) {
      printf("- Tiny Sprite resource file support (Rafael Jannone, 2022)\n");
    }

    printFooter();
  }

  return 0;
}

void printHeader() {
  printf("%s %s\n", info_header, app_version);
}

void printVersion() {
  printf("%s\n", app_version);
}

void printHelp() {
  printf("%s", info_help);
  // parser.printHelp(appFileName);
}

void printHistory() {
  printf("%s", info_history);
}

void printDocs() {
  printf("%s", info_documentation);
}

void printFooter() {
  printf("%s\n", info_footer);
}
