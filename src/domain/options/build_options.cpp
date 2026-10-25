/***
 * @file build_options.cpp
 * @brief Build options class implementation
 * @author Amaury Carvalho (2025)
 */

#ifndef BUILD_OPTIONS_CPP
#define BUILD_OPTIONS_CPP

#include "build_options.h"

BuildOptions::BuildOptions(string filename) : BuildOptions() {
  inputFilename = filename;
}

BuildOptions::BuildOptions() {
  /// default file names
  inputFilename = outputFilename = baseFilename = "";
  appFilename = "None";

  /// default options flags
  help = debug = quiet = error = version = doc = history = autoROM =
      pinGameGraphics = vscode = false;

  /// default compile mode
  compileMode = CompileMode::Plain;
  symbols = SymbolsMode::None;
  megaROM = lineNumber = false;

  /// default pcode mode
  turbo = noStripRemLines = false;

  /// default paths
  inputPath = outputPath = "";
}

string BuildOptions::getCompileModeShortName() {
  switch (compileMode) {
    case BuildOptions::CompileMode::Plain:
      return "Plain";
    case BuildOptions::CompileMode::ASCII8:
      return "ASCII8";
    case BuildOptions::CompileMode::ASCII16:
      return "ASCII16";
    case BuildOptions::CompileMode::ASCII16X:
      return "ASCII16X";
    case BuildOptions::CompileMode::Konami4:
      return "Konami";
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
    case BuildOptions::CompileMode::ASCII16:
      return "ASCII16 MegaROM";
    case BuildOptions::CompileMode::ASCII16X:
      return "ASCII16X MegaROM";
    case BuildOptions::CompileMode::Konami4:
      return "Konami MegaROM";
    case BuildOptions::CompileMode::KonamiSCC:
      return "Konami SCC MegaROM";
    case BuildOptions::CompileMode::Pcoded:
      return "Pcoded Plain ROM (deprecated)";
  }
  return "?";
}

#endif  // BUILD_OPTIONS_CPP
