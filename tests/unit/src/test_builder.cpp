/***
 * @file test_builder.cpp
 * @brief MSXBAS2ROM builder (ROM + resources) unit testing
 */

// NOLINTBEGIN

#include <cstdio>
#include <fstream>
#include <string>

#include "compiler.h"
#include "doctest/doctest.h"
#include "fswrapper.h"
#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "resources.h"
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

static std::string createTempBin(const std::string& filename, int size) {
  const std::string path = "tmp/" + filename;
  std::ofstream ofs(path, std::ios::binary);
  std::string data(size, static_cast<char>(0xAA));
  ofs.write(data.data(), data.size());
  ofs.close();
  return path;
}

static bool compileProgram(const std::string& filename, Compiler& compiler) {
  Lexer lexer;
  Parser parser;

  if (!lexer.load(filename)) return false;
  if (!lexer.evaluate()) return false;
  if (!parser.evaluate(&lexer)) return false;
  return compiler.build(&parser);
}

TEST_SUITE("Builder") {
  TEST_CASE("Builds ROM from compiled code") {
    const std::string filename =
        createTempBas("builder_valid.bas", "10 PRINT \"ROM\"\n20 END\n");

    Z80OpcodeWriter cpuOpcodeWriter;
    Compiler compiler(&cpuOpcodeWriter);
    REQUIRE(compileProgram(filename, compiler) == true);

    Rom rom;
    CHECK(rom.build(&compiler) == true);
    CHECK(rom.romSize > 0);
    CHECK(fileExists(compiler.getOpts()->outputFilename) == true);

    std::remove(filename.c_str());
    std::remove(compiler.getOpts()->outputFilename.c_str());
  }

  TEST_CASE("Fails ROM build with non-compiled input") {
    Z80OpcodeWriter cpuOpcodeWriter;
    Compiler compiler(&cpuOpcodeWriter);
    Rom rom;

    CHECK(rom.build(&compiler) == false);
  }

  TEST_CASE("Rejects resource block larger than 16K") {
    const std::string filename =
        createTempBin("builder_large_resource.bin", 0x4001);

    ResourceManager rm;
    REQUIRE(rm.addFile(filename, "tmp") == true);
    CHECK(rm.buildMap(0, 0) == false);
    CHECK(rm.logger->errors().toString().find("Resource file size exceeds") !=
          std::string::npos);

    std::remove(filename.c_str());
  }
}

// NOLINTEND
