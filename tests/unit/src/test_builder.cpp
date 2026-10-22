/***
 * @file test_builder.cpp
 * @brief MSXBAS2ROM builder (ROM + resources) unit testing
 */

// NOLINTBEGIN
// test-inventory: file-adapter

#include <cstdio>
#include <fstream>
#include <string>

#include "build_options.h"
#include "compiler.h"
#include "doctest/doctest.h"
#include "file_resource_content_source.h"
#include "in_memory_resource_content_source.h"
#include "logger.h"
#include "resources.h"
#include "test_support.h"
#include "test_temp_dir.h"

static std::string createTempBin(const TestTempDir& temp,
                                 const std::string& filename, int size) {
  const std::string path = temp.file(filename);
  std::ofstream ofs(path, std::ios::binary);
  std::string data(size, static_cast<char>(0xAA));
  ofs.write(data.data(), data.size());
  ofs.close();
  return path;
}

TEST_SUITE("Builder") {
  TEST_CASE("Builds ROM from compiled code") {
    CompilationHarness harness;
    auto compiler = harness.compileSource(
        "tmp/builder_valid.bas", "10 PRINT \"ROM\"\n20 END\n");
    REQUIRE(compiler != nullptr);

    auto artifact = harness.buildRom(compiler);
    REQUIRE(artifact != nullptr);
    CHECK(artifact->romSize > 0);
    CHECK(harness.romWriter->hasArtifact() == true);
    CHECK(harness.romWriter->artifact().pages.size() == artifact->pages.size());
  }

  TEST_CASE("Fails ROM build with non-compiled input") {
    CompilationHarness harness;
    auto compiler = harness.makeCompiler();
    auto artifact = harness.buildRom(compiler);

    CHECK(artifact == nullptr);
    CHECK(harness.romWriter->hasArtifact() == false);
  }

  TEST_CASE("Rejects resource block larger than 16K") {
    auto source = std::make_shared<InMemoryResourceContentSource>();
    source->addBinary("tmp/builder_large_resource.bin",
                      std::vector<unsigned char>(0x4001, 0xAA));

    ResourceManager rm(source);
    REQUIRE(rm.addFile("tmp/builder_large_resource.bin", "tmp") == true);
    CHECK(rm.buildMap(0, 0) == false);
    CHECK(rm.logger->errors().toString().find("Resource file size exceeds") !=
          std::string::npos);
  }

  TEST_CASE("Builds resource map with multiple resources") {
    auto source = std::make_shared<InMemoryResourceContentSource>();
    source->addBinary("tmp/builder_multi_a.bin",
                      std::vector<unsigned char>(16, 0xAA));
    source->addBinary("tmp/builder_multi_b.bin",
                      std::vector<unsigned char>(32, 0xBB));

    ResourceManager rm(source);
    REQUIRE(rm.addFile("tmp/builder_multi_a.bin", "tmp") == true);
    REQUIRE(rm.addFile("tmp/builder_multi_b.bin", "tmp") == true);
    CHECK(rm.buildMap(0, 0x8000) == true);
    CHECK(rm.pages.size() >= 1);
    CHECK(rm.resourcesPackedSize > 0);
    CHECK(rm.resourcesUnpackedSize > 0);
    CHECK(rm.toString().size() > 0);
  }

  TEST_CASE("Builds resource map with a multi-block text resource") {
    auto source = std::make_shared<InMemoryResourceContentSource>();
    source->addText("tmp/builder_txt.txt",
                    "line one\nline two\nline three\n");

    ResourceManager rm(source);
    REQUIRE(rm.addFile("tmp/builder_txt.txt", "tmp") == true);
    CHECK(rm.buildMap(0, 0x8000) == true);
    CHECK(rm.pages.size() >= 1);
  }

  TEST_CASE("Builds resource map spanning multiple segments") {
    auto source = std::make_shared<InMemoryResourceContentSource>();
    source->addBinary("tmp/builder_segment.bin",
                      std::vector<unsigned char>(0x3FF0, 0xCC));

    ResourceManager rm(source);
    REQUIRE(rm.addFile("tmp/builder_segment.bin", "tmp") == true);
    CHECK(rm.buildMap(0, 0x8000) == true);
    CHECK(rm.pages.size() >= 2);
  }

  TEST_CASE("File adapter reports missing, empty and truncated resources") {
    TestTempDir temp;
    auto fileSource = std::make_shared<FileResourceContentSource>();
    std::vector<unsigned char> content;
    std::string error;

    CHECK(fileSource->readBinary(temp.file("builder_missing.bin"), content,
                                 error) == false);
    CHECK(fileSource->exists(temp.file("builder_missing.bin")) == false);

    const std::string empty = createTempBin(temp, "builder_empty.bin", 0);
    CHECK(fileSource->readBinary(empty, content, error) == false);
    CHECK(error.find("empty") != std::string::npos);

    const std::string truncated = createTempBin(temp, "builder_truncated.bin", 4);
    CHECK(fileSource->readBinary(truncated, content, error) == true);
    CHECK(content.size() == 4);
  }
}

// NOLINTEND
