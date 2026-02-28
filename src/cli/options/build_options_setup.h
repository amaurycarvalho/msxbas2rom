/***
 * @file build_options_reader.h
 * @brief Build options reader facade class header
 * @author Amaury Carvalho (2026)
 */

#ifndef BUILD_OPTIONS_READER_H
#define BUILD_OPTIONS_READER_H

#include "build_options.h"
#include "cliparser.h"

class BuildOptionsSetup : public BuildOptions {
 private:
  CommandLineParser parser;
  void setupParser();

 public:
  BuildOptionsSetup();
  BuildOptionsSetup(string filename);

  //! @brief Parse CLI
  bool parse(int argc, char* argv[]);
};

#endif  // BUILD_OPTIONS_READER_H
