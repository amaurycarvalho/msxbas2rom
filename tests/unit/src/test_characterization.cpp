/***
 * @file test_characterization.cpp
 * @brief Behavior-preserving characterization tests for Phase A
 */

// NOLINTBEGIN
// test-inventory: file-adapter

#include <cstdio>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "build_options.h"
#include "build_options_resolver.h"
#include "compiler.h"
#include "doctest/doctest.h"
#include "file_source_resolver.h"
#include "fswrapper.h"
#include "in_memory_rom_writer.h"
#include "in_memory_source_resolver.h"
#include "test_support.h"
#include "test_temp_dir.h"
#include "in_memory_resource_content_source.h"
#include "in_memory_symbol_output_sink.h"
#include "kernel_image.h"
#include "lexer.h"
#include "logger.h"
#include "parser.h"
#include "rom.h"
#include "source_resolver.h"
#include "z80.h"

static bool compileWith(shared_ptr<SourceResolver> resolver,
                        const std::string& logicalName,
                        shared_ptr<BuildOptions> opts,
                        shared_ptr<Parser>& parser,
                        shared_ptr<Compiler>& compiler) {
  auto lexer = std::make_shared<Lexer>(resolver);
  if (!lexer->load(opts)) return false;
  if (!lexer->evaluate()) return false;

  parser = std::make_shared<Parser>();
  if (!parser->evaluate(lexer)) return false;

  auto cpu = std::make_shared<Z80OpcodeWriter>();
  compiler = std::make_shared<Compiler>(
      cpu, makeTestKernelImage(),
      std::make_shared<InMemoryResourceContentSource>(),
      std::make_shared<InMemorySymbolOutputSink>());
  return compiler->build(parser);
}

static shared_ptr<BuildOptions> makeOptions(const std::string& filename) {
  auto opts = std::make_shared<BuildOptions>();
  build_options::setInputFilename(opts, filename);
  return opts;
}

TEST_SUITE("Characterization") {
  TEST_CASE("Compilation harness compiles and inspects a ROM in memory") {
    CompilationHarness harness;
    auto compiler = harness.compileSource(
        "tmp/harness_self_test.bas", "10 PRINT \"HI\"\n20 END\n");
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    CHECK(artifact->pages.size() >= 2);
    CHECK(artifact->romSize > 0);
    CHECK(harness.romWriter->hasArtifact() == true);
    CHECK(fileExists("tmp/harness_self_test.bas") == false);
    CHECK(fileExists("tmp/harness_self_test.rom") == false);
  }

  TEST_CASE("CLI diagnostic formatting is preserved") {
    Logger logger;
    logger.setFile("program.bas");
    logger.setLineNumber(3);
    logger.error("Boom");
    logger.info("Info");

    CHECK(logger.trace().toString() ==
          "program.bas:3: ERROR: Boom\nInfo\n");
  }

  TEST_CASE("In-memory and file-backed parsing/compilation are equivalent") {
    const std::string path =
        testFixturePath("tests/integration/PRINT/test1.bas");
    std::ifstream input(path.c_str());
    REQUIRE(input.good());
    const std::string content((std::istreambuf_iterator<char>(input)),
                              std::istreambuf_iterator<char>());

    auto fileResolver = std::make_shared<FileSourceResolver>();
    auto memoryResolver = std::make_shared<InMemorySourceResolver>();
    memoryResolver->add(path, content);

    shared_ptr<Parser> fileParser;
    shared_ptr<Compiler> fileCompiler;
    shared_ptr<Parser> memoryParser;
    shared_ptr<Compiler> memoryCompiler;

    auto fileOpts = makeOptions(path);
    auto memoryOpts = makeOptions(path);

    REQUIRE(compileWith(fileResolver, path, fileOpts, fileParser,
                        fileCompiler) == true);
    REQUIRE(compileWith(memoryResolver, path, memoryOpts, memoryParser,
                        memoryCompiler) == true);

    CHECK(fileParser->toString() == memoryParser->toString());
    CHECK(fileCompiler->getCodeSize() == memoryCompiler->getCodeSize());
    CHECK(fileCompiler->getRamSize() == memoryCompiler->getRamSize());

    std::vector<unsigned char> fileCode(65536, 0);
    std::vector<unsigned char> memoryCode(65536, 0);
    int fileSize = fileCompiler->write(fileCode.data(), 0x8000);
    int memorySize = memoryCompiler->write(memoryCode.data(), 0x8000);

    REQUIRE(fileSize == memorySize);
    fileCode.resize(fileSize);
    memoryCode.resize(memorySize);
    CHECK(fileCode == memoryCode);

  }

  TEST_CASE("ROM bytes are equivalent for in-memory and file-backed programs") {
    const std::string filePath =
        testFixturePath("tests/integration/PRINT/test1.bas");
    const std::string memoryPath = "tmp/characterization_rom_memory.bas";
    std::ifstream input(filePath.c_str());
    REQUIRE(input.good());
    const std::string content((std::istreambuf_iterator<char>(input)),
                              std::istreambuf_iterator<char>());

    auto memoryResolver = std::make_shared<InMemorySourceResolver>();
    memoryResolver->add(memoryPath, content);

    shared_ptr<Parser> parser;
    shared_ptr<Compiler> compiler;
    auto fileOpts = makeOptions(filePath);
    fileOpts->outputFilename = "tmp/characterization_file.rom";
    REQUIRE(compileWith(std::make_shared<FileSourceResolver>(), filePath,
                        fileOpts, parser, compiler) == true);

    auto fileWriter = std::make_shared<InMemoryRomWriter>();
    Rom fileRom(fileWriter);
    REQUIRE(fileRom.build(compiler) == true);

    auto memoryOpts = makeOptions(memoryPath);
    memoryOpts->outputFilename = "tmp/characterization_memory.rom";
    REQUIRE(compileWith(memoryResolver, memoryPath, memoryOpts, parser,
                        compiler) == true);

    auto memoryWriter = std::make_shared<InMemoryRomWriter>();
    Rom memoryRom(memoryWriter);
    REQUIRE(memoryRom.build(compiler) == true);

    CHECK(artifactBytes(*fileRom.getArtifact()) ==
          artifactBytes(*memoryRom.getArtifact()));
  }

  TEST_CASE("Plain-ROM overflow marks the input for the ASCII8 retry") {
    std::string content;
    int line = 10;
    for (int i = 0; i < 200; i++) {
      content += std::to_string(line++) + " A=" + std::to_string(i) + "\n";
      content += std::to_string(line++) + " B=A+" + std::to_string(i) + "\n";
      content += std::to_string(line++) + " C=A*B\n";
      content += std::to_string(line++) + " PRINT C\n";
    }
    content += std::to_string(line) + " END\n";
    const std::string path = "tmp/characterization_retry.bas";
    auto memoryResolver = std::make_shared<InMemorySourceResolver>();
    memoryResolver->add(path, content);

    auto opts = makeOptions(path);
    shared_ptr<Parser> parser;
    shared_ptr<Compiler> compiler;
    REQUIRE(compileWith(memoryResolver, path, opts, parser, compiler) == true);

    auto plainWriter = std::make_shared<InMemoryRomWriter>();
    Rom plainRom(plainWriter);
    CHECK(plainRom.build(compiler) == false);
    std::string diagnostic = plainRom.getLogger()->errors().toString();
    CHECK(diagnostic.find("plain ROM limit") != std::string::npos);
    CHECK(opts->autoROM == false);

    opts->compileMode = BuildOptions::CompileMode::ASCII8;
    opts->megaROM = true;
    build_options::setInputFilename(opts, path);

    REQUIRE(compileWith(memoryResolver, path, opts, parser, compiler) == true);
    auto ascii8Writer = std::make_shared<InMemoryRomWriter>();
    Rom ascii8Rom(ascii8Writer);
    REQUIRE(ascii8Rom.build(compiler) == true);
    CHECK(ascii8Rom.romSize > 0x8000);
  }
}

// NOLINTEND
