/***
 * @file options.cpp
 * @brief Build options class implementation
 * @author Amaury Carvalho (2025)
 */

#ifndef OPTIONS_CPP
#define OPTIONS_CPP

#include "options.h"

BuildOptions::BuildOptions(string filename) {
  BuildOptions();
  setInputFilename(filename);
}

BuildOptions::BuildOptions() {
  /// default file names
  inputFilename = outputFilename = symbolFilename = "";
  appFileName = "None";

  /// default options flags
  help = debug = quiet = error = version = doc = history = false;

  /// default compile mode
  compileMode = CompileMode::Plain;
  megaROM = symbols = lineNumber = false;

  /// default pcode mode
  turbo = noStripRemLines = false;

  /// default paths
  inputPath = outputPath = "";

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
  parser.addOption("-s", "--symbol", "Generate symbols for OpenMSX debugger",
                   false, false, [&](const std::string&) { symbols = true; });
  parser.addOption("-l", "--lin",
                   "Write the MSX BASIC line numbers in the binary code", false,
                   false, [&](const std::string&) { symbols = true; });

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

void BuildOptions::setInputFilename(string filename) {
  inputFilename = filename;
  if (!inputFilename.empty()) {
    if (inputPath.empty()) {
      inputPath = getFilePath(inputFilename);
    }
    if (outputPath.empty()) {
      outputPath = inputPath;
    }
    outputFilename =
        pathJoin(outputPath, getFileNameWithoutExtension(inputFilename));

    if (compileMode == CompileMode::ASCII8 ||
        compileMode == CompileMode::KonamiSCC) {
      outputFilename += "[" + getCompileModeShortName() + "]";
      megaROM = true;
    }

    symbolFilename = outputFilename + ".symbol";
    omdsFilename = outputFilename + ".omds";
    outputFilename += ".rom";
  }
}

bool BuildOptions::parse(int argc, char* argv[]) {
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

string BuildOptions::getCompileModeShortName() {
  switch (compileMode) {
    case BuildOptions::CompileMode::Plain:
      return "Plain";
    case BuildOptions::CompileMode::ASCII8:
      return "ASCII8";
    case BuildOptions::CompileMode::KonamiSCC:
      return "KonamiSCC";
    case BuildOptions::CompileMode::Pcoded:
      return "Pcoded";
  }
  return "?";
}

string BuildOptions::getCompileModeLongName() {
  switch (compileMode) {
    case BuildOptions::CompileMode::Plain:
      return "Plain ROM";
    case BuildOptions::CompileMode::ASCII8:
      return "ASCII8 MegaROM";
    case BuildOptions::CompileMode::KonamiSCC:
      return "Konami SCC MegaROM";
    case BuildOptions::CompileMode::Pcoded:
      return "Pcoded Plain ROM (deprecated)";
  }
  return "?";
}

#endif  // OPTIONS_CPP