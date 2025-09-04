/***
 * @file options.h
 * @brief Build options class header
 * @author Amaury Carvalho (2025)
 */

#ifndef OPTIONS_H
#define OPTIONS_H

#include "cliparser.h"
#include "fswrapper.h"

using namespace std;

///-----------------------------------------------------------
/// @name BUILD OPTIONS CLASS
///-----------------------------------------------------------

/***
 * @brief Build options class
 */
class BuildOptions {
 private:
  CommandLineParser parser;

 public:
  //! @brief Compile modes: Plain ROM, ASCII8 MegaROM and KonamiSCC MegaROM
  //! @deprecated Pcoded
  enum class CompileMode { Plain, ASCII8, KonamiSCC, Pcoded } compileMode;

  //! @brief File names (input, output and symbol)
  string inputFilename, outputFilename, symbolFilename, omdsFilename,
      appFileName;

  //! @brief Options flags
  bool help, debug, quiet, error, version, history, doc;

  //! @brief Options flags
  //! @note for compiled mode only
  bool symbols, lineNumber, megaROM;

  //! @brief Options flags
  //! @deprecated for pcoded mode only
  bool turbo, noStripRemLines;

  //! @brief Options paths
  string inputPath, outputPath;

  //! @brief Options error message
  string errorMessage;

  //! @brief Parse CLI
  bool parse(int argc, char *argv[]);

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

#endif  // OPTIONS_H