/***
 * @file main.cpp
 * @brief MSXBAS2ROM compiler CLI main implementation
 * @author Amaury Carvalho
 * @copyright (GNU GPL3) 2019~
 * @see
 *   MSX-BASIC to ROM compiler project:
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

#include <memory>
#ifndef MacOS
#include <malloc.h>
#endif
#include <math.h>

#include <set>

#include "logger.h"
#include "main.h"
#include "z80.h"

using namespace std;

int main(int argc, char* argv[]) {
  shared_ptr<BuildOptionsSetup> opts = make_shared<BuildOptionsSetup>();
  set<Logger::LogLevel> logLevels = {Logger::LogLevel::ERROR};
  shared_ptr<Lexer> lexer;
  shared_ptr<Rom> rom;
  shared_ptr<Parser> parser;
  shared_ptr<Compiler> compiler;
  shared_ptr<ICpuOpcodeWriter> cpu;
  shared_ptr<Logger> logger;
  bool retriedWithAscii8 = false;

  /// parsing parameters

  if (!opts->parse(argc, argv)) {
    printHeader();
    printf("ERROR: Invalid parameter(s)\n%s\n", opts->errorMessage.c_str());
    return 1;
  };

  if (opts->debug) {
    logLevels.insert(Logger::LogLevel::DEBUG);
  }

  if (!opts->quiet) {
    bool footer = false;

    logLevels.insert(Logger::LogLevel::INFO);
    logLevels.insert(Logger::LogLevel::WARNING);

    /// version
    if (opts->version) {
      printVersion();
      return 0;
    }

    /// header information
    printHeader();

    /// quick reference
    if (opts->doc) {
      printDocs();
      footer = true;

      /// history
    } else if (opts->history) {
      printHistory();
      footer = true;

      /// help hint
    } else if (argc == 1 || opts->help) {
      printHelp();
      footer = true;
    }

    if (footer) {
      printFooter();
      return 0;
    }
  } else {
    if (argc == 1 || opts->help || opts->doc || opts->version ||
        opts->history) {
      printf("ERROR: Invalid parameter for quiet mode\n");
      return 1;
    }
  }

  if (opts->vscode) {
    VSCodeHelper vscodeHelper(opts->appFilename);

    if (!vscodeHelper.initialize()) {
      printf("ERROR: VSCode MSX-BASIC project already initialized.\n");
      return 1;
    }

    printf("\nVSCode MSX-BASIC project initialized successfully.\n");
    printf(
        "Now, use on the VSCode:\n"
        "- SHIFT+CTRL+B to build your project;\n"
        "- SHIFT+CTRL+T to test your project on the emulator;\n"
        "- F5 for debugging your code on the emulator (breakpoints will\n"
        "  be created automatically for each line).\n");
    printf(
        "Please, check the correct emulator path settings on "
        ".vscode/tasks.json.\n\n");

    if (argc == 2) return 0;
  }

  if (opts->inputFilename.empty()) {
    printf("ERROR: Input file name parameter is missing!\n");
    return 1;
  }

  if (!fileExists(opts->inputFilename)) {
    printf("ERROR: Input file not found!\n");
    return 1;
  }

  if (opts->compileMode == BuildOptions::CompileMode::Pcoded) {
    printf(
        "ERROR: P-code mode is now DEPRECATED\n"
        "If you really need it, try to use v0.3.3.1 release.\n"
        "You can get it at:\n"
        "https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.1");
    return 1;
  }

  while (true) {
    lexer = make_shared<Lexer>();
    parser = make_shared<Parser>();
    cpu = make_shared<Z80OpcodeWriter>();
    compiler = make_shared<Compiler>(cpu);
    rom = make_shared<Rom>();

    if (fileExists(opts->outputFilename)) {
      remove(opts->outputFilename.c_str());
    }

    if (!opts->quiet)
      printf("Converting %s to %s ...\n", opts->inputFilename.c_str(),
             opts->outputFilename.c_str());

    /// LEXICAL ANALYSIS
    //! @note lexing the input file, tokenizing it

    logger = lexer->getLogger();

    logger->info("(1) Doing lexical analysis...");

    if (!lexer->load(opts)) {
      logger->error("Cannot load input file for lexical analysis");
      print(logger->trace().toString());
      return 1;
    }

    if (!lexer->evaluate()) {
      if (!logger->containErrors()) {
        logger->error("Unknown error");
      }
      print(logger->trace().toString());
      return 1;
    }

    if (!retriedWithAscii8) print(logger->filter(logLevels).toString());

    /// SYNTACTIC ANALYSIS
    //! @note parsing the lexing tokens, building the syntax tree

    logger = parser->getLogger();

    logger->info("(2) Doing syntactic analysis...");

    if (!parser->evaluate(lexer)) {
      if (!parser->getLineNumber()) {
        logger->error("Is the file empty?");
      } else if (!logger->containErrors()) {
        logger->error("Unknown error");
      }
      print(logger->trace().toString());
      return 1;
    }

    if (!retriedWithAscii8) print(logger->filter(logLevels).toString());

    if (parser->getHasPt3()) {
      printf(
          "ERROR: PT3 support is now DEPRECATED\n"
          "If you really need it, try to use v0.3.3.1 release.\n"
          "You can get it at:\n"
          "https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.1");
      return 1;
    }

    /// SEMANTIC ANALYSIS
    //! @note create assembly output

    logger = compiler->getLogger();

    logger->info("(3) Doing semantic analysis (compiling)...");

    if (!compiler->build(parser)) {
      if (!logger->containErrors()) {
        logger->error("Unknown error");
      }
      print(logger->trace().toString());
      return 1;
    }

    if (opts->megaROM) {
      if (opts->compileMode == BuildOptions::CompileMode::KonamiSCC) {
        logger->info(
            "    Compiling for MegaROM format (Konami with SCC mapper)");
      } else
        logger->info("    Compiling for MegaROM format (ASCII8 mapper)");
    }
    logger->info("    Compiled code size = " +
                 to_string(compiler->getCodeSize()) + " byte(s)");
    logger->info("    Memory allocated to variables = " +
                 to_string(compiler->getRamSize()) + " byte(s)");

    print(logger->filter(logLevels).toString());

    /// ROM OUTPUT

    logger = rom->getLogger();

    logger->info("(4) Building ROM...");

    if (!rom->build(compiler)) {
      bool shouldRetryWithAscii8 =
          opts->autoROM && !retriedWithAscii8 &&
          opts->compileMode == BuildOptions::CompileMode::Plain &&
          rom->getLogger()->errors().toString().find("plain ROM limit") !=
              string::npos;

      if (shouldRetryWithAscii8) {
        logger->clear();
        logger->info(
            "WARNING: Plain ROM overflow detected.\n"
            "Auto mode activated to retry as an ASCII8 MegaROM...\n");
        print(logger->filter(logLevels).toString());

        opts->compileMode = BuildOptions::CompileMode::ASCII8;
        opts->megaROM = true;
        opts->setInputFilename(opts->inputFilename);
        retriedWithAscii8 = true;
        continue;
      }

      if (!logger->containErrors()) {
        logger->error("ROM building error");
      }
      print(logger->trace().toString());
      return 1;
    }

    if (opts->symbols != BuildOptions::SymbolsMode::None) {
      auto symbolManager = compiler->getSymbolManager();
      symbolManager->saveSymbol(opts);
    }

    break;
  }

  print(logger->filter(logLevels).toString());

  /// FINAL SUMMARY WITH COMPILATION STATISTICS

  if (!opts->quiet) {
    if (opts->megaROM) {
      if (opts->compileMode == BuildOptions::CompileMode::KonamiSCC) {
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

    auto resourceManager = compiler->getResourceManager();
    if (resourceManager->resources.size()) {
      printf("    Resources occupied %.1f%% of avaliable space\n",
             rom->resourcesShare);
      printf("      %i resource(s) found (%.1fK size",
             (int)resourceManager->resources.size(),
             resourceManager->resourcesPackedSize / 1024.0);
      if (resourceManager->resourcesPackedSize <
          resourceManager->resourcesUnpackedSize) {
        printf(", %.1f%% packed rate", resourceManager->packedRate);
      }
      printf(")\n");
      if (opts->debug) printf("%s", resourceManager->toString().c_str());
    }

    printf("    Kernel code occupied %.1f%% of avaliable space\n",
           rom->kernelShare);
    if (compiler->getFont()) {
      printf("        Built-In fonts included in the kernel\n");
    }
    printf("    Compiled code occupied %.1f%% of avaliable space\n",
           rom->codeShare);

    printf("    RAM usage will be %.1f%% of avaliable capacity\n",
           compiler->getRamMemoryPerc());

    printf("    Symbols file created for debugger support\n");

    printf("Compilation finished with success.\n");

    printf("\nIncluded into this ROM:\n");
    printf("- XBASIC support routines, copyright by J.Suzuki 1989.\n");
    printf("- Pletter v0.5c1 by XL2S Entertainment 2008.\n");
    if (compiler->getAkm()) {
      printf(
          "- Arkos Tracker 2 minimalist player v2.0.1 by Julien Névo, "
          "2021\n");
    }
    if (compiler->getHasTinySprite()) {
      printf("- Tiny Sprite resource file support (Rafael Jannone, 2022)\n");
    }

    printFooter();
  }

  return 0;
}

void print(string msg) {
  printf("%s", msg.c_str());
}

void printHeader() {
  printf("%s %s\n", info_header, app_version);
}

void printVersion() {
  print(app_version);
  printf("\n");
}

void printHelp() {
  print(info_help);
}

void printHistory() {
  print(info_history);
}

void printDocs() {
  print(info_documentation);
}

void printFooter() {
  print(info_footer);
  printf("\n");
}
