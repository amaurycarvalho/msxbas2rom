/***
 * @file test_file_adapters.cpp
 * @brief Explicit file-backed adapter tests
 */

// NOLINTBEGIN
// test-inventory: file-adapter

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "doctest/doctest.h"
#include "file_resource_content_source.h"
#include "file_rom_writer.h"
#include "file_source_resolver.h"
#include "file_symbol_output_sink.h"
#include "rom_artifact.h"
#include "source_document.h"

static std::string writeTextFile(const std::string& filename,
                                 const std::string& content) {
  const std::string path = "tmp/" + filename;
  std::ofstream out(path.c_str(), std::ios::binary);
  out.write(content.data(), content.size());
  out.close();
  return path;
}

TEST_SUITE("File adapters") {
  TEST_CASE("FileSourceResolver reports success and existing diagnostics") {
    FileSourceResolver resolver;
    SourceDocument document;
    const std::string valid =
        writeTextFile("adapter_source.bas", "10 PRINT \"HI\"\n20 END\n");
    const std::string empty = writeTextFile("adapter_empty.bas", "");

    CHECK(resolver.resolve(valid, document) == true);
    CHECK(document.content.find("PRINT") != std::string::npos);

    CHECK(resolver.resolve(empty, document) == false);
    CHECK(resolver.lastError() == "Empty file");

    CHECK(resolver.resolve("tmp/adapter_missing.bas", document) == false);
    CHECK(resolver.lastError() == "File doesn't exist");

    std::remove(valid.c_str());
    std::remove(empty.c_str());
  }

  TEST_CASE("FileRomWriter writes artifact pages and reports failures") {
    RomArtifact artifact;
    artifact.pages.push_back(std::vector<unsigned char>(4, 0xAA));
    artifact.pages.push_back(std::vector<unsigned char>(2, 0xBB));

    FileRomWriter writer;
    std::string error;
    const std::string output = "tmp/adapter_output.rom";
    REQUIRE(writer.write(artifact, output, error) == true);

    std::ifstream input(output.c_str(), std::ios::binary);
    REQUIRE(input.good());
    std::vector<unsigned char> bytes(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());
    REQUIRE(bytes.size() == 6);
    CHECK(bytes[0] == 0xAA);
    CHECK(bytes[4] == 0xBB);
    input.close();
    std::remove(output.c_str());

    CHECK(writer.write(artifact, "tmp/no_such_dir/out.rom", error) == false);
    CHECK(error.find("Cannot create output file") != std::string::npos);
  }

  TEST_CASE("FileResourceContentSource reads binary and text") {
    FileResourceContentSource source;
    const std::string binary = writeTextFile("adapter_asset.bin", "ABC");
    const std::string text = writeTextFile("adapter_asset.txt", "line\n");
    const std::string empty = writeTextFile("adapter_empty.bin", "");

    std::vector<unsigned char> bytes;
    std::string content;
    std::string error;

    REQUIRE(source.readBinary(binary, bytes, error) == true);
    CHECK(bytes.size() == 3);
    REQUIRE(source.readText(text, content, error) == true);
    CHECK(content == "line\n");
    CHECK(source.exists(binary) == true);
    CHECK(source.exists("tmp/adapter_missing.bin") == false);
    CHECK(source.readBinary(empty, bytes, error) == false);
    CHECK(error.find("empty") != std::string::npos);

    std::remove(binary.c_str());
    std::remove(text.c_str());
    std::remove(empty.c_str());
  }

  TEST_CASE("FileSymbolOutputSink writes text and binary and reports failures") {
    FileSymbolOutputSink sink;
    const std::string textFile = "tmp/adapter_symbols.sym";
    const std::string binaryFile = "tmp/adapter_symbols.elf";

    CHECK(sink.writeText(textFile, "symbols") == true);
    CHECK(sink.writeBinary(binaryFile,
                           std::vector<unsigned char>{0x7F, 'E', 'L', 'F'}) ==
          true);
    CHECK(sink.writeText("tmp/no_such_dir/x.sym", "x") == false);
    CHECK(sink.writeBinary("tmp/no_such_dir/x.elf",
                           std::vector<unsigned char>{1}) == false);

    std::remove(textFile.c_str());
    std::remove(binaryFile.c_str());
  }
}

// NOLINTEND
