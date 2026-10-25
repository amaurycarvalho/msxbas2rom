/***
 * @file runner.cpp
 * @brief MSXBAS2ROM CLI implementation and production composition root
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

#include <stdio.h>

#include <memory>
#include <set>

#include "build_options_resolver.h"
#include "compilation_request.h"
#include "compilation_use_case.h"
#include "file_resource_content_source.h"
#include "file_rom_writer.h"
#include "file_source_resolver.h"
#include "file_symbol_output_sink.h"
#include "generated_kernel_image.h"
#include "generated_kernel_module_library.h"
#include "main.h"
#include "runner.h"

using namespace std;

int runCli(int argc, char* argv[]) {
  auto sourceResolver = make_shared<FileSourceResolver>();
  auto kernelImage = make_shared<GeneratedKernelImage>();
  auto moduleLibrary = make_shared<GeneratedKernelModuleLibrary>();
  auto resourceContentSource = make_shared<FileResourceContentSource>();
  auto symbolOutputSink = make_shared<FileSymbolOutputSink>();
  auto romWriter = make_shared<FileRomWriter>();

  shared_ptr<BuildOptionsSetup> opts = make_shared<BuildOptionsSetup>();

  /// parsing parameters

  if (!opts->parse(argc, argv)) {
    printHeader();
    printf("ERROR: Invalid parameter(s)\n%s\n", opts->errorMessage.c_str());
    return 1;
  };

  if (!opts->quiet) {
    bool footer = false;

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

  CompilationRequest request(opts, sourceResolver, kernelImage,
                             resourceContentSource, symbolOutputSink,
                             romWriter, moduleLibrary);

  CompilationUseCase useCase;
  CompilationResult result = useCase.execute(request);

  print(result.output);

  if (result.success && !opts->quiet) printFooter();

  return result.exitCode;
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
