/***
 * @file build_options.h
 * @brief Build options class header
 * @author Amaury Carvalho (2025)
 */

#ifndef BUILD_OPTIONS_H
#define BUILD_OPTIONS_H

#include <string>

using namespace std;

///-----------------------------------------------------------
/// @name BUILD OPTIONS CLASS
///-----------------------------------------------------------

/***
 * @brief Build options class
 */
class BuildOptions {
 public:
  //! @brief Compile modes: Plain ROM, ASCII8 MegaROM and KonamiSCC MegaROM
  //! @deprecated Pcoded
  enum class CompileMode { Plain, ASCII8, KonamiSCC, Pcoded } compileMode;

  //! @brief Symbols output format
  enum class SymbolsMode { None, NoICE, Omds, Symbol, Cdb, Elf } symbols;

  //! @brief File names (input, output, app and base names)
  string inputFilename, outputFilename, appFilename, baseFilename;

  //! @brief Options flags
  bool help, debug, quiet, error, version, history, doc, autoROM;

  //! @brief Options flags
  //! @note for compiled mode only
  bool lineNumber, megaROM;

  //! @brief Options flags
  //! @deprecated for pcoded mode only
  bool turbo, noStripRemLines;

  //! @brief Options paths
  string inputPath, outputPath;

  //! @brief Options error message
  string errorMessage;

  //! @brief Set input filename
  void setInputFilename(string filename);

  //! @brief Get compile mode long name
  string getCompileModeLongName();

  //! @brief Get compile mode short name
  string getCompileModeShortName();

  //! @brief Build options constructor
  //! @param filename Input file name
  BuildOptions(string filename);

  //! @brief Build options constructor
  BuildOptions();
};

#endif  // BUILD_OPTIONS_H
