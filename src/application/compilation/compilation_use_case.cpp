/***
 * @file compilation_use_case.cpp
 * @brief Application compilation workflow implementation
 * @author Amaury Carvalho (2026)
 */

#include "compilation_use_case.h"

#include <cstdio>
#include <set>
#include <string>

#include "build_options.h"
#include "build_options_resolver.h"
#include "compiler.h"
#include "kernel_image.h"
#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "resource_content_source.h"
#include "resource_manager.h"
#include "rom.h"
#include "rom_artifact.h"
#include "rom_writer.h"
#include "source_resolver.h"
#include "symbol_manager.h"
#include "symbol_output_sink.h"
#include "z80.h"

using namespace std;

static void appendLogger(shared_ptr<Logger> target, shared_ptr<Logger> source) {
  if (target && source) target->add(source);
}

CompilationResult CompilationUseCase::execute(
    const CompilationRequest& request) {
  CompilationResult result;
  shared_ptr<BuildOptions> opts = request.options;
  shared_ptr<SourceResolver> sourceResolver = request.sourceResolver;
  shared_ptr<KernelImage> kernelImage = request.kernelImage;
  shared_ptr<ResourceContentSource> resourceSource =
      request.resourceContentSource;
  shared_ptr<SymbolOutputSink> symbolSink = request.symbolOutputSink;
  shared_ptr<RomWriter> romWriter = request.romWriter;

  set<Logger::LogLevel> logLevels = {Logger::LogLevel::ERR};
  shared_ptr<Lexer> lexer;
  shared_ptr<Rom> rom;
  shared_ptr<Parser> parser;
  shared_ptr<Compiler> compiler;
  shared_ptr<ICpuOpcodeWriter> cpu;
  shared_ptr<Logger> logger;
  bool retriedWithAscii8 = false;

  result.diagnostics = make_shared<Logger>();

  if (!opts) {
    result.output = "ERROR: Build options are missing!\n";
    return result;
  }

  if (!sourceResolver || !kernelImage || !resourceSource || !symbolSink ||
      !romWriter) {
    result.output = "ERROR: Compilation services are missing!\n";
    return result;
  }

  if (opts->debug) logLevels.insert(Logger::LogLevel::DEBUG);
  if (!opts->quiet) {
    logLevels.insert(Logger::LogLevel::INFO);
    logLevels.insert(Logger::LogLevel::WARNING);
    result.output += "Converting " + opts->inputFilename + " to " +
                     opts->outputFilename + " ...\n";
  }

  while (true) {
    lexer = make_shared<Lexer>(sourceResolver);
    parser = make_shared<Parser>();
    cpu = make_shared<Z80OpcodeWriter>();
    compiler =
        make_shared<Compiler>(cpu, kernelImage, resourceSource, symbolSink);
    rom = make_shared<Rom>(romWriter);

    /// LEXICAL ANALYSIS
    logger = lexer->getLogger();
    logger->info("(1) Doing lexical analysis...");

    if (!lexer->load(opts)) {
      logger->error("Cannot load input file for lexical analysis");
      appendLogger(result.diagnostics, logger);
      result.output += logger->trace().toString();
      return result;
    }

    if (!lexer->evaluate()) {
      if (!logger->containErrors()) logger->error("Unknown error");
      appendLogger(result.diagnostics, logger);
      result.output += logger->trace().toString();
      return result;
    }

    if (!retriedWithAscii8) {
      result.output += logger->filter(logLevels).toString();
    }
    appendLogger(result.diagnostics, logger);

    /// SYNTACTIC ANALYSIS
    logger = parser->getLogger();
    logger->info("(2) Doing syntactic analysis...");

    if (!parser->evaluate(lexer)) {
      if (!parser->getLineNumber()) {
        logger->error("Is the file empty?");
      } else if (!logger->containErrors()) {
        logger->error("Unknown error");
      }
      appendLogger(result.diagnostics, logger);
      result.output += logger->trace().toString();
      return result;
    }

    if (!retriedWithAscii8) {
      result.output += logger->filter(logLevels).toString();
    }
    appendLogger(result.diagnostics, logger);

    if (parser->getHasPt3()) {
      result.output +=
          "ERROR: PT3 support is now DEPRECATED\n"
          "If you really need it, try to use v0.3.3.1 release.\n"
          "You can get it at:\n"
          "https://github.com/amaurycarvalho/msxbas2rom/releases/tag/v0.3.3.1";
      return result;
    }

    /// SEMANTIC ANALYSIS
    logger = compiler->getLogger();
    logger->info("(3) Doing semantic analysis (compiling)...");

    if (!compiler->build(parser)) {
      if (!logger->containErrors()) logger->error("Unknown error");
      appendLogger(result.diagnostics, logger);
      result.output += logger->trace().toString();
      return result;
    }

    if (opts->megaROM) {
      if (opts->compileMode == BuildOptions::CompileMode::KonamiSCC) {
        logger->info(
            "    Compiling for MegaROM format (Konami with SCC mapper)");
      } else if (opts->compileMode == BuildOptions::CompileMode::Konami4) {
        logger->info("    Compiling for MegaROM format (Konami4 without SCC)");
      } else if (opts->compileMode == BuildOptions::CompileMode::ASCII16) {
        logger->info("    Compiling for MegaROM format (ASCII16 mapper)");
      } else if (opts->compileMode == BuildOptions::CompileMode::ASCII16X) {
        logger->info("    Compiling for MegaROM format (ASCII16-X mapper)");
      } else {
        logger->info("    Compiling for MegaROM format (ASCII8 mapper)");
      }
    }
    logger->info("    Compiled code size = " +
                 to_string(compiler->getCodeSize()) + " byte(s)");
    logger->info("    Memory allocated to variables = " +
                 to_string(compiler->getRamSize()) + " byte(s)");

    result.output += logger->filter(logLevels).toString();
    appendLogger(result.diagnostics, logger);

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
        result.output += logger->filter(logLevels).toString();

        opts->compileMode = BuildOptions::CompileMode::ASCII8;
        opts->megaROM = true;
        build_options::setInputFilename(opts, opts->inputFilename);
        retriedWithAscii8 = true;
        result.retriedWithAscii8 = true;
        continue;
      }

      if (!logger->containErrors()) logger->error("ROM building error");
      appendLogger(result.diagnostics, logger);
      result.output += logger->trace().toString();
      return result;
    }

    if (opts->symbols != BuildOptions::SymbolsMode::None) {
      compiler->getSymbolManager()->saveSymbol(opts);
    }

    break;
  }

  result.output += rom->getLogger()->filter(logLevels).toString();

  /// FINAL SUMMARY WITH COMPILATION STATISTICS
  if (!opts->quiet) {
    if (opts->megaROM) {
      if (opts->compileMode == BuildOptions::CompileMode::KonamiSCC) {
        result.output +=
            "    MegaROM mode activated (Konami with SCC mapper).\n";
      } else if (opts->compileMode == BuildOptions::CompileMode::Konami4) {
        result.output +=
            "    MegaROM mode activated (Konami4 without SCC mapper).\n";
      } else if (opts->compileMode == BuildOptions::CompileMode::ASCII16) {
        result.output += "    MegaROM mode activated (ASCII16 mapper).\n";
      } else if (opts->compileMode == BuildOptions::CompileMode::ASCII16X) {
        result.output += "    MegaROM mode activated (ASCII16-X mapper).\n";
      } else {
        result.output += "    MegaROM mode activated (ASCII8 mapper).\n";
      }
    } else {
      result.output += "    Plain ROM mode activated.\n";
    }

    char buffer[512];
    snprintf(buffer, sizeof(buffer), "    ROM size = %.0fK (%.1f%% free)\n",
             rom->romSize / 1024.0,
             100.0 - rom->codeShare - rom->resourcesShare - rom->kernelShare);
    result.output += buffer;

    auto resourceManager = compiler->getResourceManager();
    if (resourceManager->resources.size()) {
      snprintf(buffer, sizeof(buffer),
               "    Resources occupied %.1f%% of avaliable space\n",
               rom->resourcesShare);
      result.output += buffer;
      snprintf(buffer, sizeof(buffer),
               "      %i resource(s) found (%.1fK size",
               (int)resourceManager->resources.size(),
               resourceManager->resourcesPackedSize / 1024.0);
      result.output += buffer;
      if (resourceManager->resourcesPackedSize <
          resourceManager->resourcesUnpackedSize) {
        snprintf(buffer, sizeof(buffer), ", %.1f%% packed rate",
                 resourceManager->packedRate);
        result.output += buffer;
      }
      result.output += ")\n";
      if (opts->debug) result.output += resourceManager->toString();
    }

    snprintf(buffer, sizeof(buffer),
             "    Kernel code occupied %.1f%% of avaliable space\n",
             rom->kernelShare);
    result.output += buffer;
    if (compiler->getFont()) {
      result.output += "        Built-In fonts included in the kernel\n";
    }
    snprintf(buffer, sizeof(buffer),
             "    Compiled code occupied %.1f%% of avaliable space\n",
             rom->codeShare);
    result.output += buffer;

    snprintf(buffer, sizeof(buffer),
             "    RAM usage will be %.1f%% of avaliable capacity\n",
             compiler->getRamMemoryPerc());
    result.output += buffer;

    if (opts->symbols != BuildOptions::SymbolsMode::None) {
      result.output += "    Symbols file created for debugger support\n";
    }

    result.output += "Compilation finished with success.\n";
    result.output += "\nIncluded into this ROM:\n";
    result.output += "- XBASIC support routines, copyright by J.Suzuki 1989.\n";
    result.output += "- Pletter v0.5c1 by XL2S Entertainment 2008.\n";
    if (compiler->getAkm()) {
      result.output +=
          "- Arkos Tracker 2 minimalist player v2.0.1 by Julien Névo, "
          "2021\n";
    }
    if (compiler->getHasTinySprite()) {
      result.output +=
          "- Tiny Sprite resource file support (Rafael Jannone, 2022)\n";
    }
  }

  result.success = true;
  result.exitCode = 0;
  result.artifact = rom->getArtifact();
  if (result.diagnostics) {
    result.diagnosticsText = result.diagnostics->trace().toString();
  }

  return result;
}
