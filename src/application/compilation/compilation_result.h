/***
 * @file compilation_result.h
 * @brief Application compilation result
 * @author Amaury Carvalho (2026)
 */

#ifndef COMPILATION_RESULT_H_INCLUDED
#define COMPILATION_RESULT_H_INCLUDED

#include <memory>
#include <string>

class Logger;
class RomArtifact;

class CompilationResult {
 public:
  bool success;
  int exitCode;
  bool retriedWithAscii8;
  std::string output;
  std::string diagnosticsText;
  std::shared_ptr<Logger> diagnostics;
  std::shared_ptr<RomArtifact> artifact;

  CompilationResult()
      : success(false),
        exitCode(1),
        retriedWithAscii8(false) {}
};

#endif  // COMPILATION_RESULT_H_INCLUDED
