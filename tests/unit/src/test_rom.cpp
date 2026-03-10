/***
 * @file test_rom.cpp
 * @brief MSXBAS2ROM ROM builder unit testing
 */

// NOLINTBEGIN

#include <cstdio>
#include <fstream>
#include <string>

#include "build_options.h"
#include "compiler.h"
#include "doctest/doctest.h"
#include "lexer.h"
#include "parser.h"
#include "rom.h"
#include "z80.h"

static std::string createTempBas(const std::string& filename,
                                 const std::string& content) {
  const std::string path = "tmp/" + filename;
  std::ofstream ofs(path);
  ofs << content;
  ofs.close();
  return path;
}

static bool compileWithOpts(const std::string& filename, Compiler& compiler,
                            BuildOptions& opts) {
  Lexer lexer;
  Parser parser;

  opts.setInputFilename(filename);

  if (!lexer.load(&opts)) return false;
  if (!lexer.evaluate()) return false;
  if (!parser.evaluate(&lexer)) return false;
  return compiler.build(&parser);
}

TEST_SUITE("Rom") {
  TEST_CASE("Fails when compiler is not compiled") {
    Z80OpcodeWriter cpuOpcodeWriter;
    Compiler compiler(&cpuOpcodeWriter);
    Rom rom;

    CHECK(compiler.isCompiled() == false);
    CHECK(rom.build(&compiler) == false);
  }

  TEST_CASE("Builds ROM from compiled program") {
    const std::string filename =
        createTempBas("rom_valid.bas", "10 PRINT \"HI\"\n20 END\n");

    BuildOptions opts;
    Z80OpcodeWriter cpuOpcodeWriter;
    Compiler compiler(&cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    Rom rom;
    CHECK(rom.build(&compiler) == true);

    std::ifstream out(opts.outputFilename, std::ios::binary);
    CHECK(out.good());
    out.seekg(0, std::ios::end);
    CHECK(out.tellg() > 0);
    out.close();

    std::remove(filename.c_str());
    std::remove(opts.outputFilename.c_str());
  }

  TEST_CASE("Fails when output file cannot be created") {
    const std::string filename =
        createTempBas("rom_invalid_output.bas", "10 PRINT \"HI\"\n20 END\n");

    BuildOptions opts;
    Z80OpcodeWriter cpuOpcodeWriter;
    Compiler compiler(&cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    opts.outputFilename = "tmp/no_such_dir/out.rom";

    Rom rom;
    CHECK(rom.build(&compiler) == false);
    CHECK(rom.getErrorMessage().find("Cannot create output file") !=
          std::string::npos);

    std::remove(filename.c_str());
  }
}

// NOLINTEND
