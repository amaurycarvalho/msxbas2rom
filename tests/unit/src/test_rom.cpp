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
#include "compiler_hooks.h"
#include "doctest/doctest.h"
#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "rom.h"
#include "z80.h"

extern unsigned char bin_header_bin[];

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

  TEST_CASE("Builds Konami4 ROM with patched kernel addresses") {
    const std::string filename =
        createTempBas("rom_konami4.bas", "10 PRINT \"HI\"\n20 END\n");

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->compileMode = BuildOptions::CompileMode::Konami4;
    opts->megaROM = true;
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    REQUIRE(rom->build(compiler) == true);

    std::ifstream out(opts->outputFilename, std::ios::binary);
    REQUIRE(out.good());

    int addrCount8000 = 0, addrCountA000 = 0;
    for (int i = 0; i < 0x4000 - 3; i++) {
      unsigned char buf[3];
      out.seekg(i, std::ios::beg);
      out.read(reinterpret_cast<char*>(buf), 3);
      if (buf[1] == 0 && buf[0] == 0x32 && buf[2] == 0x80) addrCount8000++;
      if (buf[1] == 0 && buf[0] == 0x3A && buf[2] == 0x80) addrCount8000++;
      if (buf[1] == 0 && buf[0] == 0x32 && buf[2] == 0xA0) addrCountA000++;
      if (buf[1] == 0 && buf[0] == 0x3A && buf[2] == 0xA0) addrCountA000++;
    }
    CHECK(addrCount8000 > 0);
    CHECK(addrCountA000 > 0);
    CHECK((addrCount8000 + addrCountA000) >= 14);
    out.close();

    std::remove(filename.c_str());
    std::remove(opts->outputFilename.c_str());
  }

  TEST_CASE("Konami4 patches exact bytes at kernel-derived offsets") {
    const std::string filename =
        createTempBas("rom_konami4_exact.bas", "10 PRINT \"HI\"\n20 END\n");

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->compileMode = BuildOptions::CompileMode::Konami4;
    opts->megaROM = true;
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    REQUIRE(rom->build(compiler) == true);

    std::ifstream out(opts->outputFilename, std::ios::binary);
    REQUIRE(out.good());

    // fixKonamiMapper writes newByte at page[0][kernelAddr - 0x4000 + 2].
    auto patchByte = [&](int dispIndex) -> unsigned char {
      int tableAddr = def_wrapper_routines_map_table + dispIndex * 2;
      int kernelAddr =
          bin_header_bin[tableAddr] | (bin_header_bin[tableAddr + 1] << 8);
      int offset = kernelAddr - 0x4000;
      unsigned char b = 0;
      out.seekg(offset + 2, std::ios::beg);
      out.read(reinterpret_cast<char*>(&b), 1);
      return b;
    };

    CHECK(patchByte(DISP_KONAMI_PATCH_SGM_8000) == 0x80);
    CHECK(patchByte(DISP_KONAMI_PATCH_SGM_A000) == 0xA0);
    CHECK(patchByte(DISP_KONAMI_PATCH_BUGFIX_6800) == 0x70);
    CHECK(patchByte(DISP_KONAMI_PATCH_BUGFIX_8000) == 0x80);
    CHECK(patchByte(DISP_KONAMI_PATCH_VERIFY_RESTORE) == 0x80);
    out.close();

    std::remove(filename.c_str());
    std::remove(opts->outputFilename.c_str());
  }

  TEST_CASE("Builds ASCII16 ROM with patched kernel") {
    const std::string filename =
        createTempBas("rom_ascii16.bas", "10 PRINT \"HI\"\n20 END\n");

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->compileMode = BuildOptions::CompileMode::ASCII16;
    opts->megaROM = true;
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    REQUIRE(rom->build(compiler) == true);

    std::ifstream out(opts->outputFilename, std::ios::binary);
    REQUIRE(out.good());
    out.seekg(0, std::ios::end);
    CHECK(out.tellg() > 0);
    out.close();

    std::remove(filename.c_str());
    std::remove(opts->outputFilename.c_str());
  }

  TEST_CASE("ASCII16 patches exact srl-a sequence at kernel-derived offset") {
    const std::string filename =
        createTempBas("rom_ascii16_exact.bas", "10 PRINT \"HI\"\n20 END\n");

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->compileMode = BuildOptions::CompileMode::ASCII16;
    opts->megaROM = true;
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    REQUIRE(rom->build(compiler) == true);

    std::ifstream out(opts->outputFilename, std::ios::binary);
    REQUIRE(out.good());

    // fixAscii16Mapper writes seq_mr_change_sgm (9 bytes) at
    // DISP_KONAMI_PATCH_SGM_8000: push af; srl a; ld (0x7000),a; pop af; ret
    auto readAt = [&](int dispIndex, unsigned char* buf, int n) {
      int tableAddr = def_wrapper_routines_map_table + dispIndex * 2;
      int kernelAddr =
          bin_header_bin[tableAddr] | (bin_header_bin[tableAddr + 1] << 8);
      int offset = kernelAddr - 0x4000;
      out.seekg(offset, std::ios::beg);
      out.read(reinterpret_cast<char*>(buf), n);
    };

    unsigned char seq[9] = {};
    readAt(DISP_KONAMI_PATCH_SGM_8000, seq, 9);
    const unsigned char expected[] = {0xF5, 0xCB, 0x3F, 0x32, 0x00,
                                      0x70, 0xF1, 0xC9, 0x00};
    for (int i = 0; i < 9; i++) CHECK(seq[i] == expected[i]);

    unsigned char omsx[3] = {};
    readAt(DISP_KONAMI_PATCH_OMSX_3, omsx, 3);
    CHECK(omsx[0] == 0x32);
    CHECK(omsx[1] == 0xFF);
    CHECK(omsx[2] == 0x77);
    out.close();

    std::remove(filename.c_str());
    std::remove(opts->outputFilename.c_str());
  }

  TEST_CASE("Builds ASCII16X ROM with patched kernel and signature") {
    const std::string filename =
        createTempBas("rom_ascii16x.bas", "10 PRINT \"HI\"\n20 END\n");

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->compileMode = BuildOptions::CompileMode::ASCII16X;
    opts->megaROM = true;
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    REQUIRE(rom->build(compiler) == true);

    std::ifstream out(opts->outputFilename, std::ios::binary);
    REQUIRE(out.good());
    out.seekg(0, std::ios::end);
    CHECK(out.tellg() > 0);

    out.seekg(0x0010);
    char sig[9] = {};
    out.read(sig, 8);
    CHECK(std::string(sig) == std::string("ASCII16X"));
    out.close();

    std::remove(filename.c_str());
    std::remove(opts->outputFilename.c_str());
  }

  TEST_CASE("Builds KonamiSCC ROM with patched kernel addresses") {
    const std::string filename =
        createTempBas("rom_konamiscc.bas", "10 PRINT \"HI\"\n20 END\n");

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->compileMode = BuildOptions::CompileMode::KonamiSCC;
    opts->megaROM = true;
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    REQUIRE(rom->build(compiler) == true);

    std::ifstream out(opts->outputFilename, std::ios::binary);
    REQUIRE(out.good());

    int addrCount8000 = 0, addrCountA000 = 0;
    for (int i = 0; i < 0x4000 - 3; i++) {
      unsigned char buf[3];
      out.seekg(i, std::ios::beg);
      out.read(reinterpret_cast<char*>(buf), 3);
      if (buf[1] == 0 && buf[0] == 0x32 && buf[2] == 0x80) addrCount8000++;
      if (buf[1] == 0 && buf[0] == 0x3A && buf[2] == 0x80) addrCount8000++;
      if (buf[1] == 0 && buf[0] == 0x32 && buf[2] == 0xA0) addrCountA000++;
      if (buf[1] == 0 && buf[0] == 0x3A && buf[2] == 0xA0) addrCountA000++;
    }
    CHECK(addrCount8000 > 0);
    CHECK(addrCountA000 > 0);
    out.close();

    std::remove(filename.c_str());
    std::remove(opts->outputFilename.c_str());
  }

  TEST_CASE("ASCII8 ROM does NOT have ASCII16X signature") {
    const std::string filename =
        createTempBas("rom_ascii8_nosig.bas", "10 PRINT \"HI\"\n20 END\n");

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->compileMode = BuildOptions::CompileMode::ASCII8;
    opts->megaROM = true;
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    REQUIRE(rom->build(compiler) == true);

    std::ifstream out(opts->outputFilename, std::ios::binary);
    REQUIRE(out.good());
    out.seekg(0x0010);
    char buf[8] = {};
    out.read(buf, 8);
    CHECK(std::string(buf) != std::string("ASCII16X"));
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

  TEST_CASE("Builds multi-page ROM in MegaROM mode") {
    std::string content;
    int line = 10;
    for (int i = 0; i < 200; i++) {
      content += std::to_string(line++) + " A=" + std::to_string(i) + "\n";
      content += std::to_string(line++) + " B=A+" + std::to_string(i) + "\n";
      content += std::to_string(line++) + " C=A*B\n";
      content += std::to_string(line++) + " PRINT C\n";
    }
    content += std::to_string(line) + " END\n";
    const std::string filename =
        createTempBas("rom_multipage.bas", content);

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->compileMode = BuildOptions::CompileMode::Konami4;
    opts->megaROM = true;
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    REQUIRE(rom->build(compiler) == true);
    CHECK(rom->romSize > 0x8000);

    std::ifstream out(opts->outputFilename, std::ios::binary);
    CHECK(out.good());
    out.seekg(0, std::ios::end);
    CHECK(out.tellg() > 0x8000);
    out.close();

    std::remove(filename.c_str());
    std::remove(opts->outputFilename.c_str());
  }

  TEST_CASE("Fails when code exceeds plain ROM limit") {
    std::string content;
    int line = 10;
    for (int i = 0; i < 200; i++) {
      content += std::to_string(line++) + " A=" + std::to_string(i) + "\n";
      content += std::to_string(line++) + " B=A+" + std::to_string(i) + "\n";
      content += std::to_string(line++) + " C=A*B\n";
      content += std::to_string(line++) + " PRINT C\n";
    }
    content += std::to_string(line) + " END\n";
    const std::string filename =
        createTempBas("rom_overlimit.bas", content);

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    CHECK(rom->build(compiler) == false);
    CHECK(rom->getLogger()->errors().toString().find(
              "Code exceeded 16k plain ROM limit") != std::string::npos);

    std::remove(filename.c_str());
  }

  TEST_CASE("Reports share percentages that sum to the ROM size") {
    const std::string filename =
        createTempBas("rom_shares.bas", "10 PRINT \"HI\"\n20 END\n");

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    REQUIRE(rom->build(compiler) == true);

    CHECK(rom->romSize > 0);
    CHECK(rom->codeSize > 0);
    CHECK(rom->kernelShare > 0);
    CHECK(rom->codeShare > 0);
    // Without resources the resource share is only the fixed map overhead.
    CHECK(rom->resourcesShare >= 0);
    CHECK(rom->resourcesShare < 1.0);
    CHECK(rom->kernelShare + rom->codeShare + rom->resourcesShare ==
          doctest::Approx(100.0).epsilon(0.5));

    std::remove(filename.c_str());
    std::remove(opts->outputFilename.c_str());
  }

  TEST_CASE("Builds a plain ROM that embeds a DATA resource") {
    const std::string filename = createTempBas(
        "rom_data_resource.bas", "10 DATA 1,2,3,4,5\n20 READ A\n30 END\n");

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    REQUIRE(rom->build(compiler) == true);

    CHECK(rom->resourcesSize > 0);
    CHECK(rom->resourcesShare > 0);
    CHECK(rom->kernelShare + rom->codeShare + rom->resourcesShare ==
          doctest::Approx(100.0).epsilon(1.0));

    std::remove(filename.c_str());
    std::remove(opts->outputFilename.c_str());
  }

  TEST_CASE("Builds a Konami4 ROM that embeds a DATA resource") {
    std::string content = "10 DATA ";
    for (int i = 0; i < 80; i++) {
      content += std::to_string(i);
      if (i < 79) content += ",";
    }
    content += "\n20 READ A\n30 END\n";
    const std::string filename =
        createTempBas("rom_data_mega.bas", content);

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->compileMode = BuildOptions::CompileMode::Konami4;
    opts->megaROM = true;
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    REQUIRE(rom->build(compiler) == true);

    CHECK(rom->resourcesSize > 0);
    CHECK(rom->resourcesShare > 0);
    CHECK(rom->romSize >= 0x8000);
    CHECK(rom->kernelShare + rom->codeShare + rom->resourcesShare ==
          doctest::Approx(100.0).epsilon(1.0));

    std::remove(filename.c_str());
    std::remove(opts->outputFilename.c_str());
  }

  TEST_CASE("MegaROM resource map start address and segment are written") {
    const std::string filename =
        createTempBas("rom_resmap.bas", "10 PRINT \"HI\"\n20 END\n");

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->compileMode = BuildOptions::CompileMode::Konami4;
    opts->megaROM = true;
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    REQUIRE(rom->build(compiler) == true);

    std::ifstream out(opts->outputFilename, std::ios::binary);
    REQUIRE(out.good());
    // pages[1][11..13] = resource map start address (lo/hi) + segment
    unsigned char lo = 0, hi = 0, seg = 0;
    out.seekg(0x4000 + 11, std::ios::beg);
    out.read(reinterpret_cast<char*>(&lo), 1);
    out.read(reinterpret_cast<char*>(&hi), 1);
    out.read(reinterpret_cast<char*>(&seg), 1);
    // resourceAddress = 0x8000 + 0x10 = 0x8010
    CHECK(lo == 0x10);
    CHECK(hi == 0x80);
    // resourceSegment = pages.size() * 2 (kernel + start = 2 pages)
    CHECK(seg == 4);
    out.close();

    std::remove(filename.c_str());
    std::remove(opts->outputFilename.c_str());
  }

  TEST_CASE("Reports an exact resources share for a DATA resource") {
    std::string content = "10 DATA ";
    for (int i = 0; i < 80; i++) {
      content += std::to_string(i);
      if (i < 79) content += ",";
    }
    content += "\n20 READ A\n30 END\n";
    const std::string filename =
        createTempBas("rom_data_share.bas", content);

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    REQUIRE(rom->build(compiler) == true);

    CHECK(rom->resourcesSize > 0);
    CHECK(rom->romSize > 0);
    double expected =
        (static_cast<double>(rom->resourcesSize) / rom->romSize) * 100.0;
    CHECK(rom->resourcesShare == doctest::Approx(expected).epsilon(0.01));

    std::remove(filename.c_str());
    std::remove(opts->outputFilename.c_str());
  }

  TEST_CASE("MegaROM output size is a multiple of 128KB") {
    std::string content;
    int line = 10;
    for (int i = 0; i < 200; i++) {
      content += std::to_string(line++) + " A=" + std::to_string(i) + "\n";
      content += std::to_string(line++) + " B=A+" + std::to_string(i) + "\n";
      content += std::to_string(line++) + " C=A*B\n";
      content += std::to_string(line++) + " PRINT C\n";
    }
    content += std::to_string(line) + " END\n";
    const std::string filename =
        createTempBas("rom_mega_mult.bas", content);

    shared_ptr<BuildOptions> opts = make_shared<BuildOptions>();
    opts->compileMode = BuildOptions::CompileMode::Konami4;
    opts->megaROM = true;
    shared_ptr<Z80OpcodeWriter> cpuOpcodeWriter =
        make_shared<Z80OpcodeWriter>();
    shared_ptr<Compiler> compiler = make_shared<Compiler>(cpuOpcodeWriter);

    REQUIRE(compileWithOpts(filename, compiler, opts) == true);

    shared_ptr<Rom> rom = make_shared<Rom>();
    REQUIRE(rom->build(compiler) == true);

    std::ifstream out(opts->outputFilename, std::ios::binary);
    REQUIRE(out.good());
    out.seekg(0, std::ios::end);
    std::streamoff size = out.tellg();
    out.close();
    CHECK(size > 0);
    CHECK(size % (8 * 0x4000) == 0);

    std::remove(filename.c_str());
    std::remove(opts->outputFilename.c_str());
  }
}

// NOLINTEND
