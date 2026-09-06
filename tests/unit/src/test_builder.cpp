/***
 * @file test_builder.cpp
 * @brief MSXBAS2ROM builder (ROM + resources) unit testing
 */

// NOLINTBEGIN

#include <cstdio>
#include <fstream>
#include <string>

#include "build_options.h"
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

static bool compileProgram(const std::string& filename,
                           shared_ptr<Compiler> compiler) {
  shared_ptr<Lexer> lexer = make_shared<Lexer>();
  shared_ptr<Parser> parser = make_shared<Parser>();

  if (!lexer->load(filename)) return false;
  if (!lexer->evaluate()) return false;
  if (!parser->evaluate(lexer)) return false;
  return compiler->build(parser);
}

TEST_SUITE("Builder") {
  TEST_CASE("Builds ROM from compiled code") {
    const std::string filename =
        createTempBas("builder_valid.bas", "10 PRINT \"ROM\"\n20 END\n");

    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();

    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
    REQUIRE(compileProgram(filename, compiler) == true);

    Rom rom;
    CHECK(rom.build(compiler) == true);
    CHECK(rom.romSize > 0);
    CHECK(fileExists(compiler->getOpts()->outputFilename) == true);

    std::remove(filename.c_str());
    std::remove(compiler->getOpts()->outputFilename.c_str());
  }

  TEST_CASE("Fails ROM build with non-compiled input") {
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
    Rom rom;

    CHECK(rom.build(compiler) == false);
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

  TEST_CASE("Builds resource map with multiple resources") {
    const std::string f1 = createTempBin("builder_multi_a.bin", 16);
    const std::string f2 = createTempBin("builder_multi_b.bin", 32);

    ResourceManager rm;
    REQUIRE(rm.addFile(f1, "tmp") == true);
    REQUIRE(rm.addFile(f2, "tmp") == true);
    CHECK(rm.buildMap(0, 0x8000) == true);
    CHECK(rm.pages.size() >= 1);
    CHECK(rm.resourcesPackedSize > 0);
    CHECK(rm.resourcesUnpackedSize > 0);
    CHECK(rm.toString().size() > 0);

    std::remove(f1.c_str());
    std::remove(f2.c_str());
  }

  TEST_CASE("Builds resource map with a multi-block text resource") {
    const std::string filename = "tmp/builder_txt.txt";
    std::ofstream ofs(filename);
    ofs << "line one\nline two\nline three\n";
    ofs.close();

    ResourceManager rm;
    REQUIRE(rm.addFile(filename, "tmp") == true);
    CHECK(rm.buildMap(0, 0x8000) == true);
    CHECK(rm.pages.size() >= 1);

    std::remove(filename.c_str());
  }

  TEST_CASE("Builds resource map spanning multiple segments") {
    const std::string filename = createTempBin("builder_segment.bin", 0x3FF0);

    ResourceManager rm;
    REQUIRE(rm.addFile(filename, "tmp") == true);
    CHECK(rm.buildMap(0, 0x8000) == true);
    CHECK(rm.pages.size() >= 2);

    std::remove(filename.c_str());
  }
}

// NOLINTEND
