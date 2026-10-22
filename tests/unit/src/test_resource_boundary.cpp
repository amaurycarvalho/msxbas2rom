/***
 * @file test_resource_boundary.cpp
 * @brief Resource-content source boundary characterization tests
 */

// NOLINTBEGIN
// test-inventory: file-adapter

#include <cstdio>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "doctest/doctest.h"
#include "file_resource_content_source.h"
#include "in_memory_resource_content_source.h"
#include "logger.h"
#include "resource_blob_reader.h"
#include "resource_content_source.h"
#include "resource_txt_reader.h"
#include "test_temp_dir.h"

static std::string createTempBinary(const TestTempDir& temp,
                                    const std::string& filename,
                                    const std::vector<unsigned char>& data) {
  const std::string path = temp.file(filename);
  std::ofstream ofs(path, std::ios::binary);
  ofs.write(reinterpret_cast<const char*>(data.data()), data.size());
  ofs.close();
  return path;
}

static std::string createTempText(const TestTempDir& temp,
                                  const std::string& filename,
                                  const std::string& content) {
  const std::string path = temp.file(filename);
  std::ofstream ofs(path);
  ofs << content;
  ofs.close();
  return path;
}

TEST_SUITE("Resource content boundary") {
  TEST_CASE("Blob reader accepts valid in-memory bytes") {
    auto source = std::make_shared<InMemoryResourceContentSource>();
    std::vector<unsigned char> bytes = {0x00, 0x11, 0x22, 0x33, 0xFF};
    source->addBinary("asset.bin", bytes);

    ResourceBlobReader reader("asset.bin", source);

    REQUIRE(reader.load() == true);
    REQUIRE(reader.data.size() == 1);
    CHECK(reader.data[0] == bytes);
    CHECK(reader.unpackedSize == 5);
    CHECK(reader.packedSize == 5);
  }

  TEST_CASE("Blob reader handles the 16K boundary and empty content") {
    auto source = std::make_shared<InMemoryResourceContentSource>();
    source->addBinary("boundary.bin",
                      std::vector<unsigned char>(0x4000, 0xA5));
    source->addBinary("empty.bin", std::vector<unsigned char>());

    ResourceBlobReader boundary("boundary.bin", source);
    REQUIRE(boundary.load() == true);
    CHECK(boundary.unpackedSize == 0x4000);
    CHECK(boundary.data[0].front() == 0xA5);
    CHECK(boundary.data[0].back() == 0xA5);

    ResourceBlobReader empty("empty.bin", source);
    CHECK(empty.load() == false);
    CHECK(empty.getLogger()->errors().toString().find("Resource is empty") !=
          std::string::npos);
  }

  TEST_CASE("Blob reader reports missing in-memory content") {
    auto source = std::make_shared<InMemoryResourceContentSource>();
    ResourceBlobReader reader("missing.bin", source);

    CHECK(reader.load() == false);
    CHECK(reader.getLogger()->errors().toString().find(
              "Resource file not found: missing.bin") != std::string::npos);
  }

  TEST_CASE("Text reader preserves lines and truncates at 255 bytes") {
    auto source = std::make_shared<InMemoryResourceContentSource>();
    source->addText("asset.txt", "line one\r\nline two\n" + std::string(300, 'X'));

    ResourceTxtReader reader("asset.txt", source);

    REQUIRE(reader.load() == true);
    REQUIRE(reader.data.size() == 4);
    CHECK(reader.data[0][0] == 2);
    CHECK(reader.data[1][0] == 8);
    CHECK(reader.data[2][0] == 8);
    CHECK(reader.data[3][0] == 255);
  }

  TEST_CASE("File adapter keeps the same content as the in-memory adapter") {
    TestTempDir temp;
    std::vector<unsigned char> bytes = {0xDE, 0xAD, 0xBE, 0xEF};
    const std::string binaryPath = createTempBinary(temp, "boundary.bin", bytes);
    const std::string textPath =
        createTempText(temp, "boundary.txt", "alpha\nbeta\n");

    auto source = std::make_shared<InMemoryResourceContentSource>();
    source->addBinary(binaryPath, bytes);
    source->addText(textPath, "alpha\nbeta\n");

    auto fileSource = std::make_shared<FileResourceContentSource>();

    std::vector<unsigned char> memoryBytes;
    std::vector<unsigned char> fileBytes;
    std::string error;

    REQUIRE(source->readBinary(binaryPath, memoryBytes, error) == true);
    REQUIRE(fileSource->readBinary(binaryPath, fileBytes, error) == true);
    CHECK(memoryBytes == fileBytes);

    std::string memoryText;
    std::string fileText;
    REQUIRE(source->readText(textPath, memoryText, error) == true);
    REQUIRE(fileSource->readText(textPath, fileText, error) == true);
    CHECK(memoryText == fileText);

    ResourceTxtReader memoryReader(textPath, source);
    ResourceTxtReader fileReader(textPath, fileSource);

    REQUIRE(memoryReader.load() == true);
    REQUIRE(fileReader.load() == true);
    CHECK(memoryReader.data == fileReader.data);
  }
}

// NOLINTEND
