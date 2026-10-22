/***
 * @file build_options_resolver.cpp
 * @brief Application policy that derives output names from build options
 * @author Amaury Carvalho (2026)
 */

#include "build_options_resolver.h"

#include "build_options.h"
#include "build_path_policy.h"

namespace build_options {

void setInputFilename(BuildOptions& opts, const std::string& filename) {
  opts.inputFilename = filename;
  if (opts.inputFilename.empty()) return;

  if (opts.inputPath.empty()) {
    opts.inputPath = build_path::getFilePath(opts.inputFilename);
  }
  if (opts.outputPath.empty()) {
    opts.outputPath = opts.inputPath;
  }
  opts.baseFilename = build_path::pathJoin(
      opts.outputPath,
      build_path::getFileNameWithoutExtension(opts.inputFilename));

  if (opts.compileMode == BuildOptions::CompileMode::ASCII8 ||
      opts.compileMode == BuildOptions::CompileMode::ASCII16 ||
      opts.compileMode == BuildOptions::CompileMode::ASCII16X ||
      opts.compileMode == BuildOptions::CompileMode::Konami4 ||
      opts.compileMode == BuildOptions::CompileMode::KonamiSCC) {
    opts.baseFilename += "[" + opts.getCompileModeShortName() + "]";
    opts.megaROM = true;
  }

  opts.outputFilename = opts.baseFilename + ".rom";
}

void setInputFilename(std::shared_ptr<BuildOptions> opts,
                      const std::string& filename) {
  if (!opts) return;
  setInputFilename(*opts, filename);
}

}  // namespace build_options
