/***
 * @file test_rom.cpp
 * @brief MSXBAS2ROM ROM builder unit testing
 */

// NOLINTBEGIN

#include <string>
#include <vector>

#include "build_options.h"
#include "build_options_resolver.h"
#include "compiler.h"
#include "compiler_hooks.h"
#include "doctest/doctest.h"
#include "file_rom_writer.h"
#include "fswrapper.h"
#include "logger.h"
#include "rom.h"
#include "rom_artifact.h"
#include "test_support.h"

extern unsigned char bin_header_bin[];

static std::shared_ptr<Compiler> compileProgram(
    CompilationHarness& harness, const std::string& filename,
    const std::string& content, std::shared_ptr<BuildOptions> opts = nullptr) {
  return harness.compileSource(filename, content, opts);
}

static std::shared_ptr<BuildOptions> makeOptions(
    const std::string& filename,
    BuildOptions::CompileMode mode = BuildOptions::CompileMode::Plain,
    bool megaROM = false) {
  auto opts = std::make_shared<BuildOptions>();
  build_options::setInputFilename(opts, filename);
  opts->compileMode = mode;
  opts->megaROM = megaROM;
  return opts;
}

static int countMappedAddresses(const std::vector<unsigned char>& bytes,
                                unsigned char opcode, unsigned char high) {
  int count = 0;
  for (std::size_t i = 0; i + 2 < 0x4000; i++) {
    if (bytes[i] == opcode && bytes[i + 1] == 0 && bytes[i + 2] == high) {
      count++;
    }
  }
  return count;
}

TEST_SUITE("Rom") {
  TEST_CASE("Sets startup file I/O flag to non-disk mode by default") {
    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_nondisk_flag.bas");
    auto compiler = compileProgram(harness, "tmp/rom_nondisk_flag.bas",
                                   "10 PRINT \"HI\"\n20 END\n", opts);
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    auto bytes = artifactBytes(*artifact);
    REQUIRE(bytes.size() > 0x4000 + 10);
    CHECK(bytes[0x4000 + 10] == 0x00);
  }

  TEST_CASE("Sets startup file I/O flag to disk mode when file support is used") {
    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_disk_flag.bas");
    auto compiler = compileProgram(
        harness, "tmp/rom_disk_flag.bas",
        "10 OPEN \"A:TEST.TXT\" FOR INPUT AS #1\n20 CLOSE #1\n30 END\n", opts);
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    auto bytes = artifactBytes(*artifact);
    REQUIRE(bytes.size() > 0x4000 + 10);
    CHECK(bytes[0x4000 + 10] == 0x01);
  }

  TEST_CASE("Fails when compiler is not compiled") {
    CompilationHarness harness;
    auto compiler = harness.makeCompiler();

    CHECK(compiler->isCompiled() == false);
    CHECK(harness.buildRom(compiler) == nullptr);
  }

  TEST_CASE("Builds ROM from compiled program") {
    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_valid.bas");
    auto compiler = compileProgram(harness, "tmp/rom_valid.bas",
                                   "10 PRINT \"HI\"\n20 END\n", opts);
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    CHECK(artifact->romSize > 0);
    CHECK(artifactBytes(*artifact).size() > 0);
  }

  TEST_CASE("Builds Konami4 ROM with patched kernel addresses") {
    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_konami4.bas",
                            BuildOptions::CompileMode::Konami4, true);
    auto compiler = compileProgram(harness, "tmp/rom_konami4.bas",
                                   "10 PRINT \"HI\"\n20 END\n", opts);
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    auto bytes = artifactBytes(*artifact);

    int addrCount8000 = countMappedAddresses(bytes, 0x32, 0x80) +
                        countMappedAddresses(bytes, 0x3A, 0x80);
    int addrCountA000 = countMappedAddresses(bytes, 0x32, 0xA0) +
                        countMappedAddresses(bytes, 0x3A, 0xA0);
    CHECK(addrCount8000 > 0);
    CHECK(addrCountA000 > 0);
    CHECK((addrCount8000 + addrCountA000) >= 14);
  }

  TEST_CASE("Konami4 patches exact bytes at kernel-derived offsets") {
    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_konami4_exact.bas",
                            BuildOptions::CompileMode::Konami4, true);
    auto compiler = compileProgram(harness, "tmp/rom_konami4_exact.bas",
                                   "10 PRINT \"HI\"\n20 END\n", opts);
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    auto bytes = artifactBytes(*artifact);

    auto patchByte = [&](int dispIndex) -> unsigned char {
      int tableAddr = def_wrapper_routines_map_table + dispIndex * 2;
      int kernelAddr =
          bin_header_bin[tableAddr] | (bin_header_bin[tableAddr + 1] << 8);
      int offset = kernelAddr - 0x4000;
      return bytes[offset + 2];
    };

    CHECK(patchByte(DISP_KONAMI_PATCH_SGM_8000) == 0x80);
    CHECK(patchByte(DISP_KONAMI_PATCH_SGM_A000) == 0xA0);
    CHECK(patchByte(DISP_KONAMI_PATCH_BUGFIX_6800) == 0x70);
    CHECK(patchByte(DISP_KONAMI_PATCH_BUGFIX_8000) == 0x80);
    CHECK(patchByte(DISP_KONAMI_PATCH_VERIFY_RESTORE) == 0x80);
  }

  TEST_CASE("Builds ASCII16 ROM with patched kernel") {
    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_ascii16.bas",
                            BuildOptions::CompileMode::ASCII16, true);
    auto compiler = compileProgram(harness, "tmp/rom_ascii16.bas",
                                   "10 PRINT \"HI\"\n20 END\n", opts);
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    CHECK(artifact->romSize > 0);
  }

  TEST_CASE("ASCII16 patches exact srl-a sequence at kernel-derived offset") {
    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_ascii16_exact.bas",
                            BuildOptions::CompileMode::ASCII16, true);
    auto compiler = compileProgram(harness, "tmp/rom_ascii16_exact.bas",
                                   "10 PRINT \"HI\"\n20 END\n", opts);
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    auto bytes = artifactBytes(*artifact);

    auto readAt = [&](int dispIndex, unsigned char* buf, int n) {
      int tableAddr = def_wrapper_routines_map_table + dispIndex * 2;
      int kernelAddr =
          bin_header_bin[tableAddr] | (bin_header_bin[tableAddr + 1] << 8);
      int offset = kernelAddr - 0x4000;
      for (int i = 0; i < n; i++) buf[i] = bytes[offset + i];
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
  }

  TEST_CASE("Builds ASCII16X ROM with patched kernel and signature") {
    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_ascii16x.bas",
                            BuildOptions::CompileMode::ASCII16X, true);
    auto compiler = compileProgram(harness, "tmp/rom_ascii16x.bas",
                                   "10 PRINT \"HI\"\n20 END\n", opts);
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    auto bytes = artifactBytes(*artifact);
    REQUIRE(bytes.size() > 0x18);
    CHECK(std::string(reinterpret_cast<char*>(&bytes[0x10]), 8) ==
          std::string("ASCII16X"));
  }

  TEST_CASE("Builds KonamiSCC ROM with patched kernel addresses") {
    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_konamiscc.bas",
                            BuildOptions::CompileMode::KonamiSCC, true);
    auto compiler = compileProgram(harness, "tmp/rom_konamiscc.bas",
                                   "10 PRINT \"HI\"\n20 END\n", opts);
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    auto bytes = artifactBytes(*artifact);

    int addrCount8000 = countMappedAddresses(bytes, 0x32, 0x80) +
                        countMappedAddresses(bytes, 0x3A, 0x80);
    int addrCountA000 = countMappedAddresses(bytes, 0x32, 0xA0) +
                        countMappedAddresses(bytes, 0x3A, 0xA0);
    CHECK(addrCount8000 > 0);
    CHECK(addrCountA000 > 0);
  }

  TEST_CASE("ASCII8 ROM does NOT have ASCII16X signature") {
    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_ascii8_nosig.bas",
                            BuildOptions::CompileMode::ASCII8, true);
    auto compiler = compileProgram(harness, "tmp/rom_ascii8_nosig.bas",
                                   "10 PRINT \"HI\"\n20 END\n", opts);
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    auto bytes = artifactBytes(*artifact);
    REQUIRE(bytes.size() >= 0x18);
    CHECK(std::string(reinterpret_cast<char*>(&bytes[0x10]), 8) !=
          std::string("ASCII16X"));
  }

  TEST_CASE("Assembles the ROM artifact without creating an output file") {
    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_artifact.bas");
    auto compiler = compileProgram(harness, "tmp/rom_artifact.bas",
                                   "10 PRINT \"ART\"\n20 END\n", opts);
    REQUIRE(compiler != nullptr);

    Rom rom(harness.romWriter);
    REQUIRE(rom.build(compiler) == true);

    REQUIRE(harness.romWriter->hasArtifact() == true);
    CHECK(harness.romWriter->filename() == opts->outputFilename);
    CHECK(harness.romWriter->artifact().pages.size() >= 2);
    CHECK(harness.romWriter->artifact().romSize > 0x4000);
    CHECK(fileExists(opts->outputFilename) == false);

    REQUIRE(rom.getArtifact() != nullptr);
    CHECK(rom.getArtifact()->romSize == rom.romSize);
    CHECK(rom.getArtifact()->codeSize == rom.codeSize);
    CHECK(rom.getArtifact()->resourceMapSegment == 0);
  }

  TEST_CASE("Fails when output file cannot be created") {
    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_invalid_output.bas");
    auto compiler = compileProgram(harness, "tmp/rom_invalid_output.bas",
                                   "10 PRINT \"HI\"\n20 END\n", opts);
    REQUIRE(compiler != nullptr);

    opts->outputFilename = "tmp/no_such_dir/out.rom";

    auto fileWriter = std::make_shared<FileRomWriter>();
    Rom rom(fileWriter);
    CHECK(rom.build(compiler) == false);
    CHECK(rom.getLogger()->errors().toString().find(
              "Cannot create output file") != std::string::npos);
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

    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_multipage.bas",
                            BuildOptions::CompileMode::Konami4, true);
    auto compiler = compileProgram(harness, "tmp/rom_multipage.bas", content,
                                   opts);
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    CHECK(artifact->romSize > 0x8000);
    CHECK(artifactBytes(*artifact).size() > 0x8000);
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

    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_overlimit.bas");
    auto compiler =
        compileProgram(harness, "tmp/rom_overlimit.bas", content, opts);
    REQUIRE(compiler != nullptr);

    Rom rom(harness.romWriter);
    CHECK(rom.build(compiler) == false);
    CHECK(rom.getLogger()->errors().toString().find(
              "Code exceeded 16k plain ROM limit") != std::string::npos);
  }

  TEST_CASE("Reports share percentages that sum to the ROM size") {
    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_shares.bas");
    auto compiler = compileProgram(harness, "tmp/rom_shares.bas",
                                   "10 PRINT \"HI\"\n20 END\n", opts);
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    CHECK(artifact->romSize > 0);
    CHECK(artifact->codeSize > 0);
    CHECK(artifact->kernelShare > 0);
    CHECK(artifact->codeShare > 0);
    CHECK(artifact->resourcesShare >= 0);
    CHECK(artifact->resourcesShare < 1.0);
    CHECK(artifact->kernelShare + artifact->codeShare +
              artifact->resourcesShare ==
          doctest::Approx(100.0).epsilon(0.5));
  }

  TEST_CASE("Builds a plain ROM that embeds a DATA resource") {
    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_data_resource.bas");
    auto compiler = compileProgram(harness, "tmp/rom_data_resource.bas",
                                   "10 DATA 1,2,3,4,5\n20 READ A\n30 END\n",
                                   opts);
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    CHECK(artifact->resourcesSize > 0);
    CHECK(artifact->resourcesShare > 0);
    CHECK(artifact->kernelShare + artifact->codeShare +
              artifact->resourcesShare ==
          doctest::Approx(100.0).epsilon(1.0));
  }

  TEST_CASE("Builds a Konami4 ROM that embeds a DATA resource") {
    std::string content = "10 DATA ";
    for (int i = 0; i < 80; i++) {
      content += std::to_string(i);
      if (i < 79) content += ",";
    }
    content += "\n20 READ A\n30 END\n";

    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_data_mega.bas",
                            BuildOptions::CompileMode::Konami4, true);
    auto compiler =
        compileProgram(harness, "tmp/rom_data_mega.bas", content, opts);
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    CHECK(artifact->resourcesSize > 0);
    CHECK(artifact->resourcesShare > 0);
    CHECK(artifact->romSize >= 0x8000);
    CHECK(artifact->kernelShare + artifact->codeShare +
              artifact->resourcesShare ==
          doctest::Approx(100.0).epsilon(1.0));
  }

  TEST_CASE("MegaROM resource map start address and segment are written") {
    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_resmap.bas",
                            BuildOptions::CompileMode::Konami4, true);
    auto compiler = compileProgram(harness, "tmp/rom_resmap.bas",
                                   "10 PRINT \"HI\"\n20 END\n", opts);
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    auto bytes = artifactBytes(*artifact);
    REQUIRE(bytes.size() > 0x4000 + 13);
    CHECK(bytes[0x4000 + 11] == 0x10);
    CHECK(bytes[0x4000 + 12] == 0x80);
    CHECK(bytes[0x4000 + 13] == 4);
  }

  TEST_CASE("Reports an exact resources share for a DATA resource") {
    std::string content = "10 DATA ";
    for (int i = 0; i < 80; i++) {
      content += std::to_string(i);
      if (i < 79) content += ",";
    }
    content += "\n20 READ A\n30 END\n";

    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_data_share.bas");
    auto compiler =
        compileProgram(harness, "tmp/rom_data_share.bas", content, opts);
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    CHECK(artifact->resourcesSize > 0);
    CHECK(artifact->romSize > 0);
    double expected =
        (static_cast<double>(artifact->resourcesSize) / artifact->romSize) *
        100.0;
    CHECK(artifact->resourcesShare ==
          doctest::Approx(expected).epsilon(0.01));
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

    CompilationHarness harness;
    auto opts = makeOptions("tmp/rom_mega_mult.bas",
                            BuildOptions::CompileMode::Konami4, true);
    auto compiler =
        compileProgram(harness, "tmp/rom_mega_mult.bas", content, opts);
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    auto bytes = artifactBytes(*artifact);
    CHECK(bytes.size() > 0);
    CHECK(bytes.size() % (8 * 0x4000) == 0);
  }
}

// NOLINTEND
