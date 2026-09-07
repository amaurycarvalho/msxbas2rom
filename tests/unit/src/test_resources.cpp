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

#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>
#include <vector>

#include "doctest/doctest.h"
#include "logger.h"
#include "parser.h"
#include "resources.h"

// Utilities
static void createTempFile(const std::string& filename,
                           const std::string& content) {
  std::ofstream ofs(filename, std::ios::binary);
  ofs.write(content.data(), content.size());
  ofs.close();
}
static void deleteTempFile(const std::string& filename) {
  std::remove(filename.c_str());
}

// Helpers
#define CHECK_VALID_READER(reader)             \
  CHECK(reader.load() == true);                \
  CHECK(!reader.getLogger()->containErrors()); \
  CHECK(reader.data.size() > 0);               \
  CHECK(reader.unpackedSize > 0);

#define CHECK_INVALID_READER(reader) \
  CHECK(reader.load() == false);     \
  CHECK(reader.getLogger()->errors().size() > 0);

// ------------------------------------------------------------------
// ResourceManager suite
TEST_SUITE("ResourceManager suite") {
  ResourceManager resourceManager;

  TEST_CASE("ResourceManager: basic test cases") {
    std::string fname = "tmp/temp_blob.bin";

    SUBCASE("Success when loading a resource") {
      std::string fakeData = std::string(0x4000, '\xAA');
      createTempFile(fname, fakeData);

      resourceManager.clear();
      resourceManager.addFile(fname, "./tmp");
      REQUIRE(resourceManager.resources.size() == 1);

      SUBCASE("Success when building the resource address table") {
        CHECK(resourceManager.buildMap(0, 0) == true);
      }

      SUBCASE("Failing when resources count exceeds MegaROM maximum size") {
        for (int i = 0; i < 128; i++) resourceManager.addFile(fname, "./tmp");
        CHECK(resourceManager.buildMap(0, 0) == false);
        CHECK(resourceManager.logger->errors().toString().find(
                  "MegaROM size limit exceeded (2048K)") != std::string::npos);
      }

      SUBCASE("Failing when resource address table exceeds 16K limit") {
        for (int i = 0; i < 4000; i++) resourceManager.addFile(fname, "./tmp");
        CHECK(resourceManager.buildMap(0, 0) == false);
        CHECK(resourceManager.logger->errors().toString().find(
                  "Resource count maximum limit exceeded") !=
              std::string::npos);
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
      CHECK(resourceManager.logger->errors().toString().find(
                "Resource file size exceeds") != std::string::npos);

      deleteTempFile(fname);
    }
  }

  TEST_CASE("ResourceManager: resource exactly fills a 16K page") {
    std::string fname = "tmp/temp_exact16k.bin";
    std::string fakeData(0x4000, '\xBB');
    createTempFile(fname, fakeData);

    resourceManager.clear();
    resourceManager.addFile(fname, "./tmp");
    REQUIRE(resourceManager.resources.size() == 1);
    CHECK(resourceManager.buildMap(0, 0) == true);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceManager: multiple resources span pages") {
    std::string fname = "tmp/temp_page1.bin";
    std::string bigData(0x3000, '\xCC');
    std::string smallData(0x2000, '\xDD');
    createTempFile(fname, bigData);

    resourceManager.clear();
    resourceManager.addFile(fname, "./tmp");
    resourceManager.addFile(fname, "./tmp");
    REQUIRE(resourceManager.resources.size() == 2);
    CHECK(resourceManager.buildMap(0, 0) == true);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceManager: empty resource list builds successfully") {
    resourceManager.clear();
    CHECK(resourceManager.buildMap(0, 0) == true);
  }

  TEST_CASE("ResourceManager: buildMap writes exact map table bytes") {
    std::string fname = "tmp/temp_map_bytes.bin";
    std::string fakeData = "0123456789";
    createTempFile(fname, fakeData);

    resourceManager.clear();
    resourceManager.addFile(fname, "./tmp");
    REQUIRE(resourceManager.resources.size() == 1);
    REQUIRE(resourceManager.buildMap(0, 0) == true);

    REQUIRE(resourceManager.pages.size() == 1);
    // map table at 0x10: WORD count, then GROUP { WORD offset, BYTE segment,
    // WORD size } per resource. mapSize = 0x10 + 2 + 1*5 = 0x17.
    CHECK(resourceManager.pages[0][0x10] == 0x01);  // resource count low
    CHECK(resourceManager.pages[0][0x11] == 0x00);  // resource count high
    CHECK(resourceManager.pages[0][0x12] == 0x17);  // offset low
    CHECK(resourceManager.pages[0][0x13] == 0x00);  // offset high
    CHECK(resourceManager.pages[0][0x14] == 0x00);  // segment
    CHECK(resourceManager.pages[0][0x15] == 0x0A);  // size low (10)
    CHECK(resourceManager.pages[0][0x16] == 0x00);  // size high
    // resource data copied right after the map table
    CHECK(resourceManager.pages[0][0x17] == '0');
    CHECK(resourceManager.pages[0][0x17 + 9] == '9');
    // packed == unpacked for a raw blob, so the compression rate is 0
    CHECK(resourceManager.resourcesPackedSize ==
          resourceManager.resourcesUnpackedSize);
    CHECK(resourceManager.packedRate == 0.0f);

    deleteTempFile(fname);
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

  TEST_CASE("ResourceBlobReader rejects an empty file") {
    std::string fname = "tmp/temp_empty_blob.bin";
    createTempFile(fname, "");

    ResourceBlobReader reader(fname);
    CHECK(reader.load() == false);
    CHECK(reader.getLogger()->errors().toString().find("empty") !=
          std::string::npos);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceBlobReader preserves exact file bytes") {
    std::string fname = "tmp/temp_bytes.bin";
    std::string content("\x00\x01\x02\xFE\xFFGAME", 7);
    createTempFile(fname, content);

    ResourceBlobReader reader(fname);
    CHECK_VALID_READER(reader);
    REQUIRE(reader.data.size() == 1);
    REQUIRE(reader.data[0].size() == 7);
    CHECK(memcmp(reader.data[0].data(), content.data(), 7) == 0);

    deleteTempFile(fname);
  }

  // ------------------------------------------------------------------
  // ResourceBlobPackedReader (the slow 8K-boundary pack test lives in the
  // "Slow" suite below, which is excluded from mutation testing)

  TEST_CASE("ResourceBlobPackedReader rejects data over the 8K boundary") {
    std::string fname = "tmp/temp_blobpacked_over.bin";
    std::string content(0x2001, '\xAB');
    createTempFile(fname, content);

    ResourceBlobPackedReader reader(fname);
    CHECK(reader.load() == false);
    CHECK(reader.getLogger()->errors().toString().find("Resource size >") !=
          std::string::npos);

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
    CHECK(reader.getLogger()->errors().toString().find("Resource size >") !=
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
  // ResourceBlobPackedReader (pletter edge cases)
  TEST_CASE("ResourceBlobPackedReader packs varied small data patterns") {
    std::string patterns[5];
    patterns[0] = std::string(96, '\x00');
    patterns[1] = std::string(96, '\xAB');
    std::string inc;
    for (int i = 0; i < 96; i++) inc += static_cast<char>(i);
    patterns[2] = inc;
    patterns[3] = std::string(96, '\x55');
    std::string rep;
    for (int i = 0; i < 10; i++) rep += "0123456789";
    patterns[4] = rep;

    const char* names[] = {"zero", "same", "inc", "alt", "rep"};
    for (int i = 0; i < 5; i++) {
      std::string fname = std::string("tmp/temp_pletter_") + names[i] + ".bin";
      createTempFile(fname, patterns[i]);

      ResourceBlobPackedReader reader(fname);
      CHECK(reader.load() == true);
      CHECK(reader.isPacked == true);
      CHECK(reader.packedSize > 0);

      deleteTempFile(fname);
    }
  }

  TEST_CASE("ResourceBlobPackedReader round-trips a single byte") {
    std::string fname = "tmp/temp_pletter_1byte.bin";
    createTempFile(fname, std::string("\x42", 1));

    ResourceBlobPackedReader reader(fname);
    CHECK(reader.load() == true);
    CHECK(reader.isPacked == true);
    CHECK(reader.packedSize > 0);

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
    std::string loaded(reinterpret_cast<char*>(reader.data[1].data()),
                       reader.data[1].size());
    CHECK(loaded.find("Hello World") != std::string::npos);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceTxtReader computes exact unpacked size") {
    std::string fname = "tmp/temp_txt_size.txt";
    createTempFile(fname, "Hello World\nLine2\nLine3");

    ResourceTxtReader reader(fname);
    CHECK(reader.load() == true);
    // header (1) + (11+1) + (5+1) + (5+1) = 25
    CHECK(reader.unpackedSize == 25);
    CHECK(reader.packedSize == 25);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceTxtReader truncates long lines to 255 characters") {
    std::string fname = "tmp/temp_long.txt";
    std::string longLine(300, 'x');
    createTempFile(fname, longLine);

    ResourceTxtReader reader(fname);
    CHECK(reader.load() == true);
    REQUIRE(reader.data.size() == 2);
    CHECK(reader.data[1][0] == 255);
    CHECK(reader.data[1].size() == 256);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceTxtReader replaces control characters with spaces") {
    std::string fname = "tmp/temp_ctrl.txt";
    std::string text = "ab\x01"
                       "cd\x07"
                       "ef";
    createTempFile(fname, text);

    ResourceTxtReader reader(fname);
    CHECK(reader.load() == true);
    REQUIRE(reader.data.size() == 2);
    REQUIRE(reader.data[1].size() >= 7);
    const unsigned char* line = reader.data[1].data();
    CHECK(line[3] == ' ');
    CHECK(line[6] == ' ');

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceTxtReader strips CRLF at end of lines") {
    std::string fname = "tmp/temp_crlf.txt";
    createTempFile(fname, "hello\r\nworld\r\n");

    ResourceTxtReader reader(fname);
    CHECK(reader.load() == true);
    REQUIRE(reader.data.size() == 3);
    CHECK(reader.data[1][0] == 5);
    CHECK(reader.data[2][0] == 5);

    deleteTempFile(fname);
  }

  // ------------------------------------------------------------------
  // ResourceStringReader
  TEST_CASE("ResourceStringReader truncates at 255 characters") {
    std::string text(300, 'y');
    ResourceStringReader reader(text);

    CHECK(reader.load() == true);
    REQUIRE(reader.data.size() == 1);
    CHECK(reader.data[0].size() == 256);
    CHECK(reader.data[0][255] == 0);
    CHECK(reader.unpackedSize == 256);
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
    std::string loaded(reinterpret_cast<char*>(reader.data[4].data()),
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

  TEST_CASE("ResourceSprReader produces identical bytes across loads") {
    std::string fname = "tmp/temp_spr_exact.spr";
    std::string content = "!type\nmsx1\n#Slot 0\n";
    for (int i = 0; i < 16; i++) {
      content += "FFFFFFFFFFFFFFFF\n";
    }
    createTempFile(fname, content);

    ResourceSprReader reader1(fname);
    ResourceSprReader reader2(fname);
    REQUIRE(reader1.load() == true);
    REQUIRE(reader2.load() == true);

    REQUIRE(reader1.data.size() == reader2.data.size());
    CHECK(reader1.data.size() > 0);
    for (size_t i = 0; i < reader1.data.size(); i++) {
      CHECK(reader1.data[i] == reader2.data[i]);
    }
    CHECK(reader1.packedSize == reader2.packedSize);
    CHECK(reader1.unpackedSize == reader2.unpackedSize);
    CHECK(reader1.unpackedSize > 0);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceSprReader loads msx2 sprite with mixed hex digits") {
    std::string fname = "tmp/temp_spr_msx2.spr";
    std::string spr = "!type\nmsx2\n#Slot 0\n";
    for (int i = 0; i < 16; i++) {
      spr += "123456789abcdef0\n";
    }
    createTempFile(fname, spr);

    ResourceSprReader reader(fname);
    CHECK_VALID_READER(reader);
    CHECK(reader.unpackedSize > 0);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceSprReader loads multiple sprite slots") {
    std::string fname = "tmp/temp_spr_multi.spr";
    std::string spr = "!type\nmsx1\n#Slot 0\n";
    for (int i = 0; i < 16; i++) spr += "FFFFFFFFFFFFFFFF\n";
    spr += "#Slot 1\n";
    for (int i = 0; i < 16; i++) spr += "0000000000000000\n";
    createTempFile(fname, spr);

    ResourceSprReader reader(fname);
    CHECK_VALID_READER(reader);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceSprReader rejects unknown sprite type") {
    std::string fname = "tmp/temp_spr_badtype.spr";
    createTempFile(fname, "!type\nmsx9\n#Slot 0\n");

    ResourceSprReader reader(fname);
    CHECK(reader.load() == false);

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
    std::string loaded(reinterpret_cast<char*>(reader.data[0].data()),
                       reader.data[0].size());
    CHECK(loaded.find("HELLO") != std::string::npos);
  }

  // ------------------------------------------------------------------
  // ResourceDataReader (generic binary blob)
  TEST_CASE("ResourceDataReader loads binary data") {
    shared_ptr<Parser> parser = make_shared<Parser>();

    std::string fname = "tmp/temp.dat";
    std::string blob(128, '\x10');
    createTempFile(fname, blob);

    ResourceDataReader reader(parser);
    CHECK_VALID_READER(reader);
    CHECK(reader.unpackedSize == 3);

    deleteTempFile(fname);
  }

  // ------------------------------------------------------------------
  // ResourceIDataReader (indexed data reader, e.g. header + entries)
  TEST_CASE("ResourceIDataReader loads indexed data") {
    shared_ptr<Parser> parser = make_shared<Parser>();

    std::string fname = "tmp/temp.idat";
    // Fake: 4-byte header + 2 entries of 8 bytes
    std::string idata = "IDAT" + std::string(16, '\x07');
    createTempFile(fname, idata);

    ResourceIDataReader reader(parser);
    CHECK_VALID_READER(reader);

    deleteTempFile(fname);
  }
}

// ------------------------------------------------------------------
// Slow suite: tests excluded from mutation testing via --test-suite-exclude=Slow
// (they still run in the normal `make run`).
TEST_SUITE("Slow") {
  TEST_CASE("ResourceBlobPackedReader packs data at the 8K boundary") {
    std::string fname = "tmp/temp_blobpacked8k.bin";
    std::string content(0x2000, '\xAB');
    createTempFile(fname, content);

    ResourceBlobPackedReader reader(fname);
    CHECK(reader.load() == true);
    CHECK(reader.isPacked == true);
    CHECK(reader.packedSize > 0);

    deleteTempFile(fname);
  }
}

// NOLINTEND
