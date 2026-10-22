/***
 * @file test_kernel_image.cpp
 * @brief Kernel-image boundary characterization tests
 */

// NOLINTBEGIN

#include <cstdio>
#include <fstream>
#include <memory>
#include <vector>

#include "build_options.h"
#include "build_options_resolver.h"
#include "compiler.h"
#include "compiler_context.h"
#include "compiler_hooks.h"
#include "doctest/doctest.h"
#include "in_memory_kernel_image.h"
#include "in_memory_resource_content_source.h"
#include "in_memory_rom_writer.h"
#include "in_memory_source_resolver.h"
#include "in_memory_symbol_output_sink.h"
#include "test_support.h"
#include "kernel_image.h"
#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "rom.h"
#include "z80.h"

static std::vector<unsigned char> makeKernelBytes() {
  std::vector<unsigned char> bytes(0x8000, 0);

  auto setDispatch = [&](int index, unsigned int address) {
    bytes[index * 2] = address & 0xFF;
    bytes[(index * 2) + 1] = (address >> 8) & 0xFF;
  };

  setDispatch(DISP_MR_CALL, 0x4567);
  setDispatch(DISP_MR_JUMP, 0x5678);
  setDispatch(DISP_MR_GET_DATA, 0x6789);
  setDispatch(DISP_KONAMI_PATCH_SGM_8000, 0x4100);
  setDispatch(DISP_KONAMI_PATCH_BUGFIX_6800, 0x4200);

  const int konamiPatches[] = {
      DISP_KONAMI_PATCH_SGM_A000,       DISP_KONAMI_PATCH_OMSX_0,
      DISP_KONAMI_PATCH_OMSX_1,         DISP_KONAMI_PATCH_OMSX_2,
      DISP_KONAMI_PATCH_OMSX_3,         DISP_KONAMI_PATCH_OMSX_4,
      DISP_KONAMI_PATCH_BUGFIX_8000,    DISP_KONAMI_PATCH_BUGFIX_A000,
      DISP_KONAMI_PATCH_VERIFY_READ,    DISP_KONAMI_PATCH_VERIFY_WR0,
      DISP_KONAMI_PATCH_VERIFY_WR2,     DISP_KONAMI_PATCH_VERIFY_RESTORE};
  unsigned int address = 0x4400;
  for (int index : konamiPatches) {
    setDispatch(index, address);
    address += 0x10;
  }

  bytes[0x4100] = 0xC3;  // jp
  bytes[0x4101] = 0x21;
  bytes[0x4102] = 0x43;

  return bytes;
}

TEST_SUITE("Kernel image boundary") {
  TEST_CASE("In-memory image resolves dispatch entries and calls") {
    auto bytes = makeKernelBytes();
    InMemoryKernelImage image(bytes, {0x00, 0x11, 0x22});

    CHECK(image.size() == 0x8000);
    CHECK(image.startSize() == 3);
    CHECK(image.dispatchAddress(DISP_MR_CALL) == 0x4567);
    CHECK(image.dispatchAddress(DISP_MR_JUMP) == 0x5678);
    CHECK(image.dispatchAddress(DISP_MR_GET_DATA) == 0x6789);

    CHECK(image.resolveCall(DISP_MR_CALL * 2) == 0x4567);
    CHECK(image.resolveCall(0x4100) == 0x4321);
    CHECK(image.resolveCall(0x4101) == 0x4101);
    CHECK(image.resolveCall(0x9000) == 0x9000);
  }

  TEST_CASE("Code optimizer resolves kernel calls through the injected image") {
    auto bytes = makeKernelBytes();
    auto image = std::make_shared<InMemoryKernelImage>(bytes);

    auto context = std::make_shared<CompilerContext>(
        image, std::make_shared<InMemoryResourceContentSource>(),
        std::make_shared<InMemorySymbolOutputSink>());
    context->getKernelImage() = image;
    context->setHelpers(context);

    CHECK(context->getCodeOptimizer()->getKernelCallAddr(DISP_MR_CALL * 2) ==
          0x4567);
    CHECK(context->getCodeOptimizer()->getKernelCallAddr(0x4100) == 0x4321);
  }

  TEST_CASE("Mapper patches use the supplied kernel image offsets") {
    auto bytes = makeKernelBytes();
    auto image = std::make_shared<InMemoryKernelImage>(
        bytes, std::vector<unsigned char>{0x00, 0x11});
    auto resolver = std::make_shared<InMemorySourceResolver>();
    resolver->add("tmp/kernel_image.bas", "10 END\n");

    auto opts = std::make_shared<BuildOptions>();
    build_options::setInputFilename(opts, "tmp/kernel_image.bas");
    opts->compileMode = BuildOptions::CompileMode::Konami4;
    opts->megaROM = true;

    auto lexer = std::make_shared<Lexer>(resolver);
    REQUIRE(lexer->load(opts) == true);
    REQUIRE(lexer->evaluate() == true);

    auto parser = std::make_shared<Parser>();
    REQUIRE(parser->evaluate(lexer) == true);

    auto cpu = std::make_shared<Z80OpcodeWriter>();
    auto compiler = std::make_shared<Compiler>(
        cpu, image, std::make_shared<InMemoryResourceContentSource>(),
        std::make_shared<InMemorySymbolOutputSink>());
    REQUIRE(compiler->build(parser) == true);

    auto writer = std::make_shared<InMemoryRomWriter>();
    Rom rom(writer);
    INFO(rom.getLogger()->trace().toString());
    REQUIRE(rom.build(compiler) == true);

    auto romBytes = artifactBytes(*rom.getArtifact());
    REQUIRE(romBytes.size() > 0x0202);
    CHECK(romBytes[0x0102] == 0x80);
    CHECK(romBytes[0x0202] == 0x70);
  }
}

// NOLINTEND
