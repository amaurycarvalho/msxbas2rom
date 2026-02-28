/***
 * @file build_options.cpp
 * @brief Build options class implementation
 * @author Amaury Carvalho (2025)
 */

#ifndef BUILD_OPTIONS_CPP
#define BUILD_OPTIONS_CPP

#include "build_options.h"

BuildOptions::BuildOptions(string filename) : BuildOptions() {
  setInputFilename(filename);
}

BuildOptions::BuildOptions() {
  /// default file names
  inputFilename = outputFilename = symbolFilename = "";
  appFileName = "None";

  /// default options flags
  help = debug = quiet = error = version = doc = history = autoROM = false;

  /// default compile mode
  compileMode = CompileMode::Plain;
  symbols = SymbolsMode::None;
  megaROM = lineNumber = false;

  /// default pcode mode
  turbo = noStripRemLines = false;

  /// default paths
  inputPath = outputPath = "";

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
    noiceFilename = outputFilename + ".noi";
    cdbFilename = outputFilename + ".cdb";
    outputFilename += ".rom";
  }
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

#endif  // BUILD_OPTIONS_CPP
