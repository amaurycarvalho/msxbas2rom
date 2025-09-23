/***
 * @file test_resources.cpp
 * @brief MSXBAS2ROM resources unit testing
 * @author Amaury Carvalho (2019-2025)
 * @note
 * doctest reference:
 *   https://github.com/doctest/doctest/blob/master/doc/markdown/tutorial.md
 * compiling:
 *   make all
 * testing:
 *   ./test_resources
 */

// NOLINTBEGIN

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <cstdio>
#include <fstream>
#include <vector>

#include "doctest/doctest.h"
#include "header.h"
#include "resources.h"

// Utilities
static void createTempFile(const std::string &filename,
                           const std::string &content) {
  std::ofstream ofs(filename, std::ios::binary);
  ofs.write(content.data(), content.size());
  ofs.close();
}
static void deleteTempFile(const std::string &filename) {
  std::remove(filename.c_str());
}

// Helpers
#define CHECK_VALID_READER(reader)         \
  CHECK(reader.load() == true);            \
  CHECK(reader.getErrorMessage().empty()); \
  CHECK(reader.data.size() > 0);           \
  CHECK(reader.unpackedSize > 0);

#define CHECK_INVALID_READER(reader) \
  CHECK(reader.load() == false);     \
  CHECK(reader.getErrorMessage().size() > 0);

// ------------------------------------------------------------------
// ResourceManager suite
TEST_SUITE("ResourceManager suite") {
  ResourceManager resourceManager;

  TEST_CASE("ResourceManager: basic test cases") {
    std::string fname = "tmp/temp_blob.bin";

    SUBCASE("Loading a resource") {
      std::string fakeData = std::string(0x4000, '\xAA');
      createTempFile(fname, fakeData);

      resourceManager.clear();
      resourceManager.addFile(fname, "./tmp");
      REQUIRE(resourceManager.resources.size() == 1);

      SUBCASE("Building the resource map") {
        CHECK(resourceManager.buildMap(0, 0) == true);
      }

      deleteTempFile(fname);
    }

    SUBCASE("ResourceManager: resource exceeds 16K limit") {
      std::string fakeData = std::string(0x4001, '\xAA');
      createTempFile(fname, fakeData);

      resourceManager.clear();
      resourceManager.addFile(fname, "./tmp");
      REQUIRE(resourceManager.resources.size() == 1);
      CHECK(resourceManager.buildMap(0, 0) == false);
      CHECK(resourceManager.getErrorMessage().find(
                "Resource file size exceeds") != std::string::npos);

      deleteTempFile(fname);
    }
  }
}

// ------------------------------------------------------------------
// ResourceReader
TEST_SUITE("ResourceReader suite") {
  // ------------------------------------------------------------------
  // ResourceBlobReader
  TEST_CASE("ResourceBlobReader loads data") {
    // Simulate fake packed file header + data
    std::string fname = "tmp/temp_blob.bin";
    std::string fakeData = "TMP$" + std::string(0x8000, '\xAA');

    createTempFile(fname, fakeData);

    ResourceBlobReader reader(fname);
    CHECK_VALID_READER(reader);
    REQUIRE(reader.data.size() == 1);
    CHECK(reader.data[0].size() == 0x8004);

    deleteTempFile(fname);
  }

  // ------------------------------------------------------------------
  // ResourceBlobPackedReader
  TEST_CASE("ResourceBlobPackedReader loads packed data") {
    // Simulate fake packed file header + data
    std::string fname = "tmp/temp_blobpacked.bin";
    std::string fakePacked = "TMP$" + std::string(100, '\xAA');

    createTempFile(fname, fakePacked);

    ResourceBlobPackedReader reader(fname);
    CHECK_VALID_READER(reader);
    CHECK(reader.isPacked == true);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceBlobPackedReader packed data size limit exceeded") {
    // Simulate fake packed file header + data
    std::string fname = "tmp/temp_blobpacked.bin";
    std::string fakePacked = "TMP$" + std::string(0x8000, '\xAA');

    createTempFile(fname, fakePacked);

    ResourceBlobPackedReader reader(fname);
    CHECK_INVALID_READER(reader);
    CHECK(reader.getErrorMessage().find("Resource size >") !=
          std::string::npos);

    deleteTempFile(fname);
  }

  // ------------------------------------------------------------------
  // ResourceBlobChunkPackedReader
  TEST_CASE("ResourceBlobChunkPackedReader loads chunked packed data") {
    std::string fname = "tmp/temp_blobchunk.bin";
    // Simulate a file with 2 "chunks"
    std::string fakeChunk =
        "CHNK" + std::string(64, '\x01') + "CHNK" + std::string(64, '\x02');
    createTempFile(fname, fakeChunk);

    ResourceBlobChunkPackedReader reader(fname);
    CHECK_VALID_READER(reader);
    CHECK(reader.data.size() >= 2);

    deleteTempFile(fname);
  }

  // ------------------------------------------------------------------
  // ResourceTxtReader
  TEST_CASE("ResourceTxtReader parses plain text file") {
    std::string fname = "tmp/temp_text.txt";
    std::string text = "Hello World\nLine2\nLine3";
    createTempFile(fname, text);

    ResourceTxtReader reader(fname);
    CHECK_VALID_READER(reader);

    // Check first block has our text
    REQUIRE(reader.data.size() == 4);
    std::string loaded(reinterpret_cast<char *>(reader.data[1].data()),
                       reader.data[1].size());
    CHECK(loaded.find("Hello World") != std::string::npos);

    deleteTempFile(fname);
  }

  // ------------------------------------------------------------------
  // ResourceCsvReader
  TEST_CASE("ResourceCsvReader parses CSV data") {
    std::string fname = "tmp/temp_csv.csv";
    std::string csv = "id,name\n1,Alice\n2,Bob\n";
    createTempFile(fname, csv);

    ResourceCsvReader reader(fname);
    CHECK_VALID_READER(reader);

    REQUIRE(reader.data.size() == 7);
    std::string loaded(reinterpret_cast<char *>(reader.data[4].data()),
                       reader.data[4].size());
    CHECK(loaded.find("Alice") != std::string::npos);

    deleteTempFile(fname);
  }

  // ------------------------------------------------------------------
  // ResourceScrReader (MSX screen dump 6144 bytes + 768 color table)
  TEST_CASE("ResourceScrReader loads SCR format") {
    std::string fname = "tmp/temp_scr.scr";
    std::string scr(6912, '\xFF');  // 6144+768
    createTempFile(fname, scr);

    ResourceScrReader reader(fname);
    CHECK_VALID_READER(reader);
    CHECK(reader.unpackedSize == 6912);

    deleteTempFile(fname);
  }

  // ------------------------------------------------------------------
  // ResourceSprReader (MSX sprite pattern 512 bytes)
  TEST_CASE("ResourceSprReader loads SPR format") {
    std::string fname = "tmp/temp_spr.spr";
    std::string spr =
        "!type\nmsx1\n#Slot 0\n"
        "................\n"
        "................\n"
        ".....FFFF.......\n"
        "....FFFFFFF.....\n"
        "...FF.....FFF...\n"
        "...F.2..2...F...\n"
        "...F........F...\n"
        "...FF.......F...\n"
        "....F.1..1..F...\n"
        "....FF.111..F...\n"
        ".....FF.....F...\n"
        "......FFFFFFF...\n"
        "................\n"
        "................\n"
        "................\n"
        "................";
    createTempFile(fname, spr);

    ResourceSprReader reader(fname);
    CHECK_VALID_READER(reader);
    CHECK(reader.unpackedSize == 101);
    CHECK(reader.packedSize == 51);

    deleteTempFile(fname);
  }

  // ------------------------------------------------------------------
  // ResourceAkmReader (AKM custom format stub)
  TEST_CASE("ResourceAkmReader loads AKM file") {
    std::string fname = "tmp/temp.akm";
    std::string akm = "AKM" + std::string(20, '\x01');
    createTempFile(fname, akm);

    ResourceAkmReader reader(fname);
    CHECK_VALID_READER(reader);

    deleteTempFile(fname);
  }

  // ------------------------------------------------------------------
  // ResourceAkxReader (AKX custom format stub)
  TEST_CASE("ResourceAkxReader loads AKX file") {
    std::string fname = "tmp/temp.akx";
    std::string akx = "AKX" + std::string(30, '\x02');
    createTempFile(fname, akx);

    ResourceAkxReader reader(fname);
    CHECK_VALID_READER(reader);

    deleteTempFile(fname);
  }

  // ------------------------------------------------------------------
  // ResourceMtfPaletteReader (MSX palette usually 32 bytes)
  TEST_CASE("ResourceMtfPaletteReader loads palette") {
    std::string fname = "tmp/temp_palette.mtf";
    std::string pal(32, '\x0F');
    createTempFile(fname, pal);

    ResourceMtfPaletteReader reader(fname);
    CHECK_VALID_READER(reader);
    CHECK(reader.unpackedSize == 33);

    deleteTempFile(fname);
  }

  // ------------------------------------------------------------------
  // ResourceMtfTilesetReader (tile data, e.g. 256 tiles * 8 bytes = 2048)
  TEST_CASE("ResourceMtfTilesetReader loads tileset") {
    std::string fname = "tmp/temp_tileset.mtf";
    std::string tiles(2048, '\x04');
    createTempFile(fname, tiles);

    ResourceMtfTilesetReader reader(fname);
    CHECK_VALID_READER(reader);
    CHECK(reader.unpackedSize == 2049);

    deleteTempFile(fname);
  }

  // ------------------------------------------------------------------
  // ResourceMtfMapReader (map file, e.g. 32x24 = 768 bytes)
  TEST_CASE("ResourceMtfMapReader loads map") {
    std::string fname = "tmp/temp_map.mtf";
    std::string map(768, '\x05');
    createTempFile(fname, map);

    ResourceMtfMapReader reader(fname);
    CHECK_INVALID_READER(reader);

    deleteTempFile(fname);
  }

  // ------------------------------------------------------------------
  // ResourceStringReader (string table, null-terminated text)
  TEST_CASE("ResourceStringReader loads string table") {
    std::string str = "HELLO\0WORLD\0";

    ResourceStringReader reader(str);
    CHECK_VALID_READER(reader);

    REQUIRE(reader.data.size() > 0);
    std::string loaded(reinterpret_cast<char *>(reader.data[0].data()),
                       reader.data[0].size());
    CHECK(loaded.find("HELLO") != std::string::npos);
  }

  // ------------------------------------------------------------------
  // ResourceDataReader (generic binary blob)
  TEST_CASE("ResourceDataReader loads binary data") {
    Parser parser;

    std::string fname = "tmp/temp.dat";
    std::string blob(128, '\x10');
    createTempFile(fname, blob);

    ResourceDataReader reader(&parser);
    CHECK_VALID_READER(reader);
    CHECK(reader.unpackedSize == 3);

    deleteTempFile(fname);
  }

  // ------------------------------------------------------------------
  // ResourceIDataReader (indexed data reader, e.g. header + entries)
  TEST_CASE("ResourceIDataReader loads indexed data") {
    Parser parser;

    std::string fname = "tmp/temp.idat";
    // Fake: 4-byte header + 2 entries of 8 bytes
    std::string idata = "IDAT" + std::string(16, '\x07');
    createTempFile(fname, idata);

    ResourceIDataReader reader(&parser);
    CHECK_VALID_READER(reader);

    deleteTempFile(fname);
  }
}

// NOLINTEND
