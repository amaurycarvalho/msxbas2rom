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
 *     ./test/unit/test
 *   Integration testing:
 *     ./test/integration/test.sh
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
#ifndef MacOS
#include <malloc.h>
#endif
#include <math.h>

#include "main.h"

using namespace std;

int main(int argc, char *argv[]) {
  BuildOptions opts;
  Lexer lexer;
  Tokenizer tokenizer;
  Rom rom;
  Parser parser;
  Compiler compiler;
  CompilerPT3 compilerPT3;

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

  if (fileExists(opts.outputFilename)) {
    remove(opts.outputFilename.c_str());
  }

  if (!opts.quiet)
    printf("Converting %s to %s ...\n", opts.inputFilename.c_str(),
           opts.outputFilename.c_str());

  /// LEXICAL ANALYSIS
  //! @note lexing the input file, tokenizing it

  if (!opts.quiet) printf("(1) Doing lexical analysis...\n");

  if (!lexer.load(&opts)) {
    printf("ERROR: Cannot load input file for lexical analysis\n");
    printf("%s\n", lexer.errorMessage.c_str());
    return 1;
  }

  if (!lexer.evaluate()) {
    lexer.error();
    printf("ERROR: Lexical error at line %i\n", lexer.lineNo);
    printf("%s\n", lexer.errorMessage.c_str());
    return 1;
  }

  if (opts.debug) {
    printf("Displaying lexical analysis:\n");
    lexer.print();
  }

  if (opts.compileMode != BuildOptions::CompileMode::Pcoded) {
    /// SYNTACTIC ANALYSIS
    //! @note parsing the lexing tokens, building the syntax tree

    if (!opts.quiet) printf("(2) Doing syntactic analysis...\n");

    if (!parser.evaluate(&lexer)) {
      if (parser.lineNo == 0)
        printf("ERROR: Cannot build the symbol tree\n");
      else {
        parser.error();
        printf("ERROR: Syntax error at line %i\n", parser.lineNo);
      }
      return 1;
    }

    if (opts.debug) {
      printf("Displaying syntactic analysis:\n");
      parser.print();
    }

    /// SEMANTIC ANALYSIS
    //! @note create assembly output

    if (!opts.quiet) printf("(3) Doing semantic analysis (compiling)...\n");

    if (parser.has_akm && parser.has_pt3) {
      printf("ERROR: Cannot compile PT3 and Arkos players in the same ROM\n");
      return 1;
    }

    if (parser.has_pt3) {
      if (!compilerPT3.build(&parser)) {
        printf("Error: %s\n", compilerPT3.error_message.c_str());
        if (compilerPT3.current_tag) compilerPT3.current_tag->print();
        return 1;
      }

      if (!opts.quiet) {
        if (opts.megaROM) {
          printf("    Compiling for MegaROM format (Konami with SCC mapper)\n");
        }
        printf("    Compiled code size = %i byte(s)\n", compilerPT3.code_size);
        printf("    Memory allocated to variables = %i byte(s)\n",
               compilerPT3.ram_size);
      }

    } else {
      if (!compiler.build(&parser)) {
        printf("Error: %s\n", compiler.error_message.c_str());
        if (compiler.current_tag) compiler.current_tag->print();
        return 1;
      }

      if (!opts.quiet) {
        if (opts.megaROM) {
          if (opts.compileMode == BuildOptions::CompileMode::KonamiSCC) {
            printf(
                "    Compiling for MegaROM format (Konami with SCC mapper)\n");
          } else
            printf("    Compiling for MegaROM format (ASCII8 mapper)\n");
        }
        printf("    Compiled code size = %i byte(s)\n", compiler.code_size);
        printf("    Memory allocated to variables = %i byte(s)\n",
               compiler.ram_size);
      }
    }

    /// ROM OUTPUT

    if (!opts.quiet) printf("(4) Building ROM...\n");

    if (parser.has_pt3) {
      if (!rom.build(&compilerPT3)) {
        rom.error();
        printf("ERROR: ROM building error\n");
        return 1;
      }

      if (opts.symbols) {
        compilerPT3.resourceManager.saveSymbolFile(&opts, rom.code_start,
                                                   compilerPT3.ram_page);
      }

    } else {
      if (!rom.build(&compiler)) {
        rom.error();
        printf("ERROR: ROM building error\n");
        return 1;
      }

      if (opts.symbols) {
        compiler.resourceManager.saveSymbolFile(&opts, rom.code_start,
                                                compiler.ram_page);
      }
    }

    /// finish process

    if (!opts.quiet) {
      if (parser.has_pt3) {
        if (compilerPT3.resourceManager.fileList.size()) {
          int i, t = compilerPT3.resourceManager.fileList.size(), len1 = 0,
                 len2 = 0;
          for (i = 0; i < t; i++) {
            len1 += compilerPT3.resourceManager.fileList[i]->length;
            len2 += compilerPT3.resourceManager.fileList[i]->packed_length;
          }
          printf("%i file(s) attached into ROM = %i byte(s) [%i unpacked]\n", t,
                 len2, len1);
        }
        if (compilerPT3.resourceManager.resourceList.size()) {
          printf("%i resource(s) found\n",
                 (int)compilerPT3.resourceManager.resourceList.size());
        }
        if (compilerPT3.font) {
          printf("Built-In fonts included as resources\n");
        }

        if (opts.megaROM) {
          if (opts.compileMode == BuildOptions::CompileMode::KonamiSCC) {
            printf(
                "Extended memory scheme mode activated (Konami with SCC "
                "mapper).\n");
          } else
            printf("Extended memory scheme mode activated (ASCII8 mapper).\n");
          printf("MegaROM size = %ikb\n", compilerPT3.segm_total * 8);
          printf("Resources+Compiled code occupied %.1f%% of MegaROM space\n",
                 rom.stdMemoryPerc);
        } else {
          printf("Standard memory scheme mode activated.\n");
          printf("Resources+Compiled code occupied %.1f%% of avaliable space\n",
                 rom.stdMemoryPerc);
        }

        printf("RAM memory will be occupied in %.1f%% of avaliable space\n",
               compilerPT3.ramMemoryPerc);

        if (opts.symbols) {
          printf("Symbols file created for use on OpenMSX debugger\n");
        }

        printf("Conversion finished with success.\n");

        printf("\nIncluded into this ROM:\n");
        printf("- XBASIC support routines, copyright by J.Suzuki 1989.\n");
        printf("- Pletter v0.5c1 by XL2S Entertainment 2008.\n");
        if (compilerPT3.pt3) {
          printf(
              "- PT3 player for MSX by Alfonso D. C. aka Dioniso "
              "<dioniso072@yahoo.es>\n");
        }

        printFooter();

      } else {
        if (compiler.resourceManager.fileList.size()) {
          int i, t = compiler.resourceManager.fileList.size(), len1 = 0,
                 len2 = 0;
          for (i = 0; i < t; i++) {
            len1 += compiler.resourceManager.fileList[i]->length;
            len2 += compiler.resourceManager.fileList[i]->packed_length;
          }
          printf("%i file(s) attached into ROM = %i byte(s) [%i unpacked]\n", t,
                 len2, len1);
        }
        if (compiler.resourceManager.resourceList.size()) {
          printf("%i resource(s) found\n",
                 (int)compiler.resourceManager.resourceList.size());
        }
        if (compiler.font) {
          printf("Built-In fonts included as resources\n");
        }

        if (opts.megaROM) {
          if (opts.compileMode == BuildOptions::CompileMode::KonamiSCC) {
            printf(
                "Extended memory scheme mode activated (Konami with SCC "
                "mapper).\n");
          } else
            printf("Extended memory scheme mode activated (ASCII8 mapper).\n");
          printf("MegaROM size = %ikb (%.1f%% free)\n", rom.rom_size / 1024,
                 100.0 - rom.stdMemoryPerc - rom.rscMemoryPerc);
          if (compiler.resourceManager.resourceList.size()) {
            printf("Resources occupied %.1f%% of MegaROM space\n",
                   rom.rscMemoryPerc);
          }
          printf("Compiled code occupied %.1f%% of MegaROM space\n",
                 rom.stdMemoryPerc);
        } else {
          printf("Standard memory scheme mode activated.\n");
          if (compiler.resourceManager.resourceList.size()) {
            printf("Resources occupied %.1f%% of avaliable space\n",
                   rom.rscMemoryPerc);
          }
          printf("Compiled code occupied %.1f%% of avaliable space\n",
                 rom.stdMemoryPerc);
        }

        printf("RAM memory will be occupied in %.1f%% of avaliable space\n",
               compiler.ramMemoryPerc);

        if (opts.symbols) {
          printf("Symbols file created for use on OpenMSX debugger\n");
        }

        printf("Conversion finished with success.\n");

        printf("\nIncluded into this ROM:\n");
        printf("- XBASIC support routines, copyright by J.Suzuki 1989.\n");
        printf("- Pletter v0.5c1 by XL2S Entertainment 2008.\n");
        if (compiler.akm) {
          printf(
              "- Arkos Tracker 2 minimalist player v2.0.1 by Julien Névo, "
              "2021\n");
        }
        if (compiler.has_tiny_sprite) {
          printf(
              "- Tiny Sprite resource file support (Rafael Jannone, 2022)\n");
        }

        printFooter();
      }
    }

  } else {
    /// TOKEN ANALYSIS
    //! @note default basic interpreter tokenizer

    if (!opts.quiet) printf("(2) Doing token analysis...\n");

    tokenizer.debug = opts.debug;
    tokenizer.force_turbo = opts.turbo;

    if (!tokenizer.evaluate(&lexer)) {
      tokenizer.error();
      printf("ERROR: Syntax error at line %i\n", tokenizer.lineNo);
      return 1;
    }

    if (tokenizer.turbo_mode) opts.turbo = true;

    if (!opts.quiet) {
      if (opts.turbo) printf("Turbo mode detected and activated\n");
      if (tokenizer.cmd) printf("CMD instruction detected and activated\n");
      if (tokenizer.pt3) printf("PT3 support detected and activated\n");
    }

    if (opts.debug) {
      printf("Displaying token analysis:\n");
      tokenizer.print();
    }

    /// ROM OUTPUT

    if (!opts.quiet) printf("(3) Building ROM...\n");

    if (!opts.quiet && opts.noStripRemLines) {
      printf("Including remark (REM) lines into ROM file\n");
    }

    if (!rom.build(&tokenizer)) {
      rom.error();
      printf("ERROR: ROM building error\n");
      return 1;
    }

    /// finish process

    if (!opts.quiet) {
      if (tokenizer.resourceList.size()) {
        printf("%i resource(s) found\n", (int)tokenizer.resourceList.size());
      }
      if (tokenizer.font) {
        printf("Built-In fonts included as resources\n");
      }

      if (opts.megaROM) {
        printf("Extended memory scheme mode activated.\n");
        printf("Basic code occupied %.1f%% of STD avaliable space\n",
               rom.stdMemoryPerc);
      } else {
        printf("Standard memory scheme mode activated.\n");
        if (tokenizer.resourceList.size())
          printf("Basic code+resources occupied %.1f%% of avaliable space\n",
                 rom.stdMemoryPerc);
        else
          printf("Basic code occupied %.1f%% of avaliable space\n",
                 rom.stdMemoryPerc);
      }

      printf("Conversion finished with success.\n");

      if (tokenizer.turbo_mode || opts.turbo || tokenizer.cmd ||
          tokenizer.pt3) {
        printf("\nIncluded into this ROM:\n");
        if (tokenizer.turbo_mode || opts.turbo) {
          printf("- XBASIC, copyright by J.Suzuki 1989.\n");
        }
        if (tokenizer.cmd) {
          printf("- Pletter v0.5c1 by XL2S Entertainment 2008.\n");
          printf(
              "- VRAM Depacker v1.1 by Metalion 2008 (metalion@orange.fr).\n");
        }
        if (tokenizer.pt3) {
          printf(
              "- PT3 player for MSX by Alfonso D. C. aka Dioniso "
              "<dioniso072@yahoo.es>\n");
        }
      }

      printFooter();
    }
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