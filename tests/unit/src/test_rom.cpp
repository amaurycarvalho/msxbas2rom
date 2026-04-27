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
#include "logger.h"
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

static bool compileWithOpts(const std::string& filename,
                            shared_ptr<Compiler> compiler,
                            shared_ptr<BuildOptions> opts) {
  shared_ptr<Lexer> lexer = make_shared<Lexer>();
  shared_ptr<Parser> parser = make_shared<Parser>();

  opts->setInputFilename(filename);

  if (!lexer->load(opts)) return false;
  if (!lexer->evaluate()) return false;
  if (!parser->evaluate(lexer)) return false;
  return compiler->build(parser);
}

TEST_SUITE("Rom") {
  TEST_CASE("Sets startup file I/O flag to non-disk mode by default") {
    const std::string filename =
        createTempBas("rom_nondisk_flag.bas", "10 PRINT \"HI\"\n20 END\n");

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    REQUIRE(rom->build(compiler) == true);

    std::ifstream out(opts->outputFilename, std::ios::binary);
    REQUIRE(out.good());
    out.seekg(0x4000 + 10, std::ios::beg);
    unsigned char startupMode = 0xFF;
    out.read(reinterpret_cast<char*>(&startupMode), 1);
    CHECK(startupMode == 0x00);
    out.close();

    std::remove(filename.c_str());
    std::remove(opts->outputFilename.c_str());
  }

  TEST_CASE("Sets startup file I/O flag to disk mode when file support is used") {
    const std::string filename = createTempBas(
        "rom_disk_flag.bas",
        "10 OPEN \"A:TEST.TXT\" FOR INPUT AS #1\n20 CLOSE #1\n30 END\n");

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    REQUIRE(rom->build(compiler) == true);

    std::ifstream out(opts->outputFilename, std::ios::binary);
    REQUIRE(out.good());
    out.seekg(0x4000 + 10, std::ios::beg);
    unsigned char startupMode = 0x00;
    out.read(reinterpret_cast<char*>(&startupMode), 1);
    CHECK(startupMode == 0x01);
    out.close();

    std::remove(filename.c_str());
    std::remove(opts->outputFilename.c_str());
  }

  TEST_CASE("Fails when compiler is not compiled") {
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);
    shared_ptr<Rom> rom = make_shared<Rom>();

    CHECK(compiler->isCompiled() == false);
    CHECK(rom->build(compiler) == false);
  }

  TEST_CASE("Builds ROM from compiled program") {
    const std::string filename =
        createTempBas("rom_valid.bas", "10 PRINT \"HI\"\n20 END\n");

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    CHECK(rom->build(compiler) == true);

    std::ifstream out(opts->outputFilename, std::ios::binary);
    CHECK(out.good());
    out.seekg(0, std::ios::end);
    CHECK(out.tellg() > 0);
    out.close();

    std::remove(filename.c_str());
    std::remove(opts->outputFilename.c_str());
  }

  TEST_CASE("Fails when output file cannot be created") {
    const std::string filename =
        createTempBas("rom_invalid_output.bas", "10 PRINT \"HI\"\n20 END\n");

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    opts->outputFilename = "tmp/no_such_dir/out.rom";

    Rom rom;
    CHECK(rom.build(compiler) == false);
    CHECK(rom.getLogger()->errors().toString().find(
              "Cannot create output file") != std::string::npos);

    std::remove(filename.c_str());
  }
}

// NOLINTEND
