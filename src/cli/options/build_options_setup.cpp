/***
 * @file build_options_reader.cpp
 * @brief Build options reader facade class implementation
 * @author Amaury Carvalho (2026)
 */

#ifndef BUILD_OPTIONS_READER_CPP
#define BUILD_OPTIONS_READER_CPP

#include "build_options_setup.h"

BuildOptionsSetup::BuildOptionsSetup() : BuildOptions() {
  setupParser();
}

BuildOptionsSetup::BuildOptionsSetup(string filename) : BuildOptions(filename) {
  setupParser();
}

void BuildOptionsSetup::setupParser() {
  /// general options setup
  parser.addOption("-h", "--help", "Show help message", false, false,
                   [&](const std::string&) { help = true; });
  parser.addOption("-?", "", "Show help message", false, false,
                   [&](const std::string&) { help = true; });
  parser.addOption("-q", "--quiet", "Quiet mode (no verbose)", false, false,
                   [&](const std::string&) { quiet = true; });
  parser.addOption("-d", "--debug", "Debug mode (show details)", false, false,
                   [&](const std::string&) { debug = true; });
  parser.addOption("-D", "--doc", "Display documentation", false, false,
                   [&](const std::string&) { doc = true; });
  parser.addOption("-v", "--version", "Display version history", false, false,
                   [&](const std::string&) { version = true; });
  parser.addOption("-H", "--history", "Display version history", false, false,
                   [&](const std::string&) { history = true; });

  /// path options setup
  parser.addOption("-i", "--inputPath", "Input path (default=source file path)",
                   true, false,
                   [&](const std::string& val) { inputPath = val; });
  parser.addOption("-o", "--outputPath",
                   "Output path (default=source file path)", true, false,
                   [&](const std::string& val) { outputPath = val; });

  /// compile options setup
  parser.addOption(
      "-c", "--compile", "Compile mode (default=Plain ROM)", false, false,
      [&](const std::string&) { compileMode = CompileMode::Plain; });
  parser.addOption(
      "-0", "--plain", "Plain ROM type (a.k.a. Linear)", false, false,
      [&](const std::string&) { compileMode = CompileMode::Plain; });
  parser.addOption(
      "-x", "--megarom", "MegaROM type (default=ASCII8)", false, false,
      [&](const std::string&) { compileMode = CompileMode::ASCII8; });
  parser.addOption(
      "-8", "--ascii8", "ASCII8 MegaROM type", false, false,
      [&](const std::string&) { compileMode = CompileMode::ASCII8; });
  parser.addOption(
      "-k", "--scc", "KonamiSCC MegaROM type", false, false,
      [&](const std::string&) { compileMode = CompileMode::KonamiSCC; });
  parser.addOption("-a", "--auto",
                   "Auto ROM mode (fallback from Plain ROM to ASCII8 MegaROM)",
                   false, false, [&](const std::string&) { autoROM = true; });
  parser.addOption("", "--symbol", "Generate symbols in .symbol format", false,
                   false,
                   [&](const std::string&) { symbols = SymbolsMode::Symbol; });
  parser.addOption("-s", "--noi", "Generate symbols in .noi format", false,
                   false,
                   [&](const std::string&) { symbols = SymbolsMode::NoICE; });
  parser.addOption("", "--noice", "Generate symbols in .noi format", false,
                   false,
                   [&](const std::string&) { symbols = SymbolsMode::NoICE; });
  parser.addOption("", "--omds", "Generate symbols in .omds format", false,
                   false,
                   [&](const std::string&) { symbols = SymbolsMode::Omds; });
  parser.addOption("", "--cdb", "Generate symbols in .cdb format", false, false,
                   [&](const std::string&) { symbols = SymbolsMode::Cdb; });
  parser.addOption("-l", "--lin",
                   "Write the MSX BASIC line numbers in the binary code", false,
                   false, [&](const std::string&) { lineNumber = true; });

  /// pcoded options setup (deprecated)
  parser.addOption(
      "-p", "--pcode", "Tokenized p-code mode", false, true,
      [&](const std::string&) { compileMode = CompileMode::Pcoded; });
  parser.addOption("-t", "--turbo",
                   "P-coded turbo mode (or use CALL TURBO instructions)", false,
                   true, [&](const std::string&) { turbo = true; });
  parser.addOption("-N", "--nsr", "P-coded no strip remark lines", false, true,
                   [&](const std::string&) { noStripRemLines = true; });
}

bool BuildOptionsSetup::parse(int argc, char* argv[]) {
  try {
    appFileName = argv[0];
    parser.parse(argc, argv);
    setInputFilename(parser.getFilename());

  } catch (const std::exception& ex) {
    errorMessage = ex.what();
    error = true;
    return false;
  }

  return true;
}

#endif  // BUILD_OPTIONS_READER_CPP
