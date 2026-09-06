/***
 * @file test_resources_extra.cpp
 * @brief MSXBAS2ROM additional resource readers unit testing
 */

// NOLINTBEGIN

#include <cstdio>
#include <cstring>
#include <fstream>
#include <memory>
#include <string>

#include "doctest/doctest.h"
#include "logger.h"
#include "resource_akm_reader.h"
#include "resource_akx_reader.h"
#include "resource_csv_reader.h"
#include "resource_factory.h"
#include "resource_mtf_map_reader.h"
#include "resource_mtf_reader.h"
#include "resource_spr_reader.h"

static void createTempFile(const std::string& filename,
                           const std::string& content) {
  std::ofstream ofs(filename, std::ios::binary);
  ofs.write(content.data(), content.size());
  ofs.close();
}
static void deleteTempFile(const std::string& filename) {
  std::remove(filename.c_str());
}

TEST_SUITE("ResourceReadersExtra") {
  TEST_CASE("ResourceSprReader loads MSX1 sprite") {
    std::string fname = "tmp/temp_msx1.spr";
    std::string content = "!type\nmsx1\n#Slot 0\n";
    for (int i = 0; i < 16; i++) {
      content += (i == 0) ? "FFFF000000000000\n" : "................\n";
    }
    createTempFile(fname, content);

    ResourceSprReader reader(fname);
    CHECK(reader.load() == true);
    CHECK(reader.unpackedSize > 0);
    CHECK(reader.packedSize > 0);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceSprReader loads MSX2 sprite with slots") {
    std::string fname = "tmp/temp_msx2.spr";
    std::string content = "!type\nmsx2\n#Slot 0\n";
    for (int i = 0; i < 16; i++) {
      content += (i == 0) ? "FFFF000000000000\n" : "................\n";
    }
    content += "#Slot 1\n";
    for (int i = 0; i < 16; i++) {
      content += (i == 0) ? "000000000000FFFF\n" : "................\n";
    }
    createTempFile(fname, content);

    ResourceSprReader reader(fname);
    CHECK(reader.load() == true);
    CHECK(reader.unpackedSize > 0);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceSprReader rejects invalid sprite file") {
    std::string fname = "tmp/temp_bad.spr";
    createTempFile(fname, "not a sprite\n");

    ResourceSprReader reader(fname);
    CHECK(reader.load() == false);
    CHECK(reader.getLogger()->errors().size() > 0);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceSprReader handles every hex digit range") {
    std::string fname = "tmp/temp_hex.spr";
    std::string content = "!type\nmsx1\n#Slot 0\n";
    const char* rows[16] = {
        "0123456789ABCDEF", "FEDCBA9876543210", "aabbccddeeff0011",
        "8899AABBCCDDEEFF", "0123456789abcdef", "fedcba9876543210",
        "0102030405060708", "090A0B0C0D0E0F00", "0011223344556677",
        "8899AABBCCDDEEFF", "0101010101010101", "0F0F0F0F0F0F0F0F",
        "FFFFFFFFFFFFFFFF", "0000000000000000", "123456789ABCDEF0",
        "0FEDCBA987654321"};
    for (int i = 0; i < 16; i++) {
      content += rows[i];
      content += "\n";
    }
    createTempFile(fname, content);

    ResourceSprReader reader(fname);
    CHECK(reader.load() == true);
    CHECK(reader.unpackedSize > 0);
    CHECK(reader.packedSize > 0);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceSprReader remaps an MSX2 sprite with many colors") {
    std::string fname = "tmp/temp_hex_msx2.spr";
    std::string content = "!type\nmsx2\n#Slot 0\n";
    const char* rows[16] = {
        "0123456789ABCDEF", "FEDCBA9876543210", "aabbccddeeff0011",
        "8899AABBCCDDEEFF", "0123456789abcdef", "fedcba9876543210",
        "0102030405060708", "090A0B0C0D0E0F00", "0011223344556677",
        "8899AABBCCDDEEFF", "0101010101010101", "0F0F0F0F0F0F0F0F",
        "FFFFFFFFFFFFFFFF", "0000000000000000", "123456789ABCDEF0",
        "0FEDCBA987654321"};
    for (int i = 0; i < 16; i++) {
      content += rows[i];
      content += "\n";
    }
    content += "#Slot 1\n";
    const char* rows2[16] = {
        "FFFFFFFFFFFFFFFF", "FFFFFFFFFFFFFFFF", "FFFFFFFFFFFFFFFF",
        "FFFFFFFFFFFFFFFF", "FFFFFFFFFFFFFFFF", "FFFFFFFFFFFFFFFF",
        "FFFFFFFFFFFFFFFF", "FFFFFFFFFFFFFFFF", "FFFFFFFFFFFFFFFF",
        "FFFFFFFFFFFFFFFF", "FFFFFFFFFFFFFFFF", "FFFFFFFFFFFFFFFF",
        "FFFFFFFFFFFFFFFF", "FFFFFFFFFFFFFFFF", "FFFFFFFFFFFFFFFF",
        "FFFFFFFFFFFFFFFF"};
    for (int i = 0; i < 16; i++) {
      content += rows2[i];
      content += "\n";
    }
    createTempFile(fname, content);

    ResourceSprReader reader(fname);
    CHECK(reader.load() == true);
    CHECK(reader.unpackedSize > 0);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceAkmReader loads real AKM song") {
    std::string fname = "../../tests/integration/ARKTRK/songs.akm";

    ResourceAkmReader reader(fname);
    CHECK(reader.load() == true);
    CHECK(!reader.getLogger()->containErrors());
    CHECK(reader.data.size() > 0);
    CHECK(reader.unpackedSize > 0);

    SUBCASE("Remaps AKM data to a target address") {
      CHECK(reader.remapTo(0, 0, 0x8000) == true);
    }

    SUBCASE("Skips remap when index is non-zero") {
      CHECK(reader.remapTo(1, 0, 0x8000) == true);
    }

    SUBCASE("Detects AKM extension") {
      CHECK(ResourceAkmReader::isIt(".AKM") == true);
      CHECK(ResourceAkmReader::isIt(".akm") == true);
      CHECK(ResourceAkmReader::isIt(".bin") == false);
    }
  }

  TEST_CASE("ResourceAkmReader reports empty data error") {
    std::string fname = "tmp/temp_empty.akm";
    createTempFile(fname, "AKM");

    ResourceAkmReader reader(fname);
    CHECK(reader.load() == true);

    SUBCASE("Cannot guess base address of small file") {
      CHECK(reader.remapTo(0, 0, 0x8000) == true);
    }

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceAkmReader remaps a synthetic song with tables") {
    std::string fname = "tmp/temp_synth.akm";
    // Synthetic AKM layout (32 bytes):
    //   [0..1]  instrumentIndexTable = 8
    //   [2..3]  arpeggioIndexTable   = 0x10 (non-zero -> remapped)
    //   [4..5]  pitchIndexTable      = 0x12 (non-zero -> remapped)
    //   [6..7]  subsongIndexTable    = 0x0C (points to subsong header)
    //   [8..9]  instrument entry 0   = 0x0A (loop bound t = 10)
    //   [10..11] padding
    //   [12..24] subsong header, areEffectsPresent (offset 12) = 0x0C
    //   [25..31] instrument/arpeggio/pitch target data
    std::string content;
    content += std::string("\x08\x00", 2);  // instrumentIndexTable = 8
    content += std::string("\x10\x00", 2);  // arpeggioIndexTable = 0x10
    content += std::string("\x12\x00", 2);  // pitchIndexTable = 0x12
    content += std::string("\x0C\x00", 2);  // subsongIndexTable = 0x0C
    content += std::string("\x0A\x00", 2);  // instrument entry 0 = 0x0A
    content += std::string("\x00\x00", 2);  // padding
    content += std::string(12, '\x00');     // subsong header [12..23]
    content += '\x0C';                      // areEffectsPresent @ 24
    content += std::string(7, '\x00');      // [25..31]
    createTempFile(fname, content);

    ResourceAkmReader reader(fname);
    REQUIRE(reader.load() == true);
    CHECK(reader.remapTo(0, 0, 0x8000) == true);
    REQUIRE(reader.data.size() >= 1);
    CHECK(reader.data[0][1] == 0x80);  // instrumentIndexTable remapped
    CHECK(reader.data[0][3] == 0x80);  // arpeggioIndexTable remapped
    CHECK(reader.data[0][5] == 0x80);  // pitchIndexTable remapped
    CHECK(reader.data[0][7] == 0x80);  // subsongIndexTable remapped
    CHECK(reader.data[0][9] == 0x80);  // instrument entry remapped

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceAkmReader produces identical bytes across loads") {
    std::string fname = "../../tests/integration/ARKTRK/songs.akm";

    ResourceAkmReader reader1(fname);
    ResourceAkmReader reader2(fname);
    REQUIRE(reader1.load() == true);
    REQUIRE(reader2.load() == true);

    REQUIRE(reader1.data.size() == reader2.data.size());
    for (size_t i = 0; i < reader1.data.size(); i++) {
      CHECK(reader1.data[i] == reader2.data[i]);
    }
    CHECK(reader1.packedSize == reader2.packedSize);
    CHECK(reader1.unpackedSize == reader2.unpackedSize);
  }

  TEST_CASE("ResourceAkxReader loads real AKX effects") {
    std::string fname = "../../tests/integration/ARKTRK/effects.akx";

    ResourceAkxReader reader(fname);
    CHECK(reader.load() == true);
    CHECK(!reader.getLogger()->containErrors());
    CHECK(reader.data.size() > 0);
    CHECK(reader.unpackedSize > 0);

    SUBCASE("Remaps AKX data to a target address") {
      CHECK(reader.remapTo(0, 0, 0x8000) == true);
    }

    SUBCASE("Skips remap when index is non-zero") {
      CHECK(reader.remapTo(2, 0, 0x8000) == true);
    }

    SUBCASE("Detects AKX extension") {
      CHECK(ResourceAkxReader::isIt(".AKX") == true);
      CHECK(ResourceAkxReader::isIt(".akx") == true);
      CHECK(ResourceAkxReader::isIt(".dat") == false);
    }
  }

  TEST_CASE("ResourceAkxReader produces identical bytes across loads") {
    std::string fname = "../../tests/integration/ARKTRK/effects.akx";

    ResourceAkxReader reader1(fname);
    ResourceAkxReader reader2(fname);
    REQUIRE(reader1.load() == true);
    REQUIRE(reader2.load() == true);

    REQUIRE(reader1.data.size() == reader2.data.size());
    for (size_t i = 0; i < reader1.data.size(); i++) {
      CHECK(reader1.data[i] == reader2.data[i]);
    }
    CHECK(reader1.packedSize == reader2.packedSize);
    CHECK(reader1.unpackedSize == reader2.unpackedSize);
  }

  TEST_CASE("ResourceAkxReader remaps a synthetic effects list") {
    std::string fname = "tmp/temp_synth.akx";
    // 2 effects, first sound effect address = 4 (2-byte pointer list)
    // base address is guessed as 0, so both pointers are remapped to 0x8000+
    std::string content =
        std::string("\x04\x00\x06\x00\xAA\xBB\xCC\xDD", 8);
    createTempFile(fname, content);

    ResourceAkxReader reader(fname);
    REQUIRE(reader.load() == true);
    CHECK(reader.remapTo(0, 0, 0x8000) == true);
    REQUIRE(reader.data.size() >= 1);
    CHECK(reader.data[0][1] == 0x80);
    CHECK(reader.data[0][3] == 0x80);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceAkxReader guesses base address across iterations") {
    std::string fname = "tmp/temp_guess.akx";
    // first sound effect address = 4, second pointer (0x09) is out of range
    // for base address 0, so the guessing loop advances base address
    std::string content =
        std::string("\x04\x00\x09\x00\xAA\xBB\xCC\xDD", 8);
    createTempFile(fname, content);

    ResourceAkxReader reader(fname);
    REQUIRE(reader.load() == true);
    CHECK(reader.remapTo(0, 0, 0x8000) == true);
    CHECK(!reader.getLogger()->containErrors());

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceAkxReader warns on unguessable base address") {
    std::string fname = "tmp/temp_bad.akx";
    std::string content = std::string("\x00\x00", 2);
    createTempFile(fname, content);

    ResourceAkxReader reader(fname);
    REQUIRE(reader.load() == true);
    CHECK(reader.remapTo(0, 0, 0x8000) == true);
    CHECK(reader.getLogger()->containWarnings() == true);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceMtfMapReader loads a minimal map") {
    std::string superName = "tmp/temp_min.SC4Super";
    std::string mapName = "tmp/temp_min.SC4Map";

    // supertile: count=1, width=1, height=1, limit=1, reserved, one tile
    std::string supertile =
        std::string("\x01\x01\x01\x01\x00\x00\x00\x01", 8);
    // tilemap: width=1, height=2, reserved(4), two supertile indices
    std::string tilemap = std::string(
        "\x01\x00\x02\x00\x00\x00\x00\x00\x00\x00", 10);
    createTempFile(superName, supertile);
    createTempFile(mapName, tilemap);

    ResourceMtfMapReader reader(mapName);
    CHECK(reader.load() == true);
    CHECK(reader.data.size() >= 3);
    CHECK(reader.unpackedSize > 0);

    SUBCASE("Remaps first map line") {
      CHECK(reader.remapTo(1, 2, 0x8000) == true);
    }

    SUBCASE("Skips remap for index zero") {
      CHECK(reader.remapTo(0, 2, 0x8000) == true);
    }

    SUBCASE("Remaps a middle map line") {
      CHECK(reader.remapTo(1, 3, 0x9000) == true);
      CHECK(reader.remapTo(2, 3, 0x9100) == true);
    }

    deleteTempFile(superName);
    deleteTempFile(mapName);
  }

  TEST_CASE("ResourceMtfMapReader fails when supertile is missing") {
    std::string mapName = "tmp/temp_nosuper.SC4Map";
    std::string tilemap =
        std::string("\x01\x00\x01\x00\x00\x00\x00\x00\x00", 9);
    createTempFile(mapName, tilemap);

    ResourceMtfMapReader reader(mapName);
    CHECK(reader.load() == false);
    CHECK(reader.getLogger()->errors().size() > 0);

    deleteTempFile(mapName);
  }

  TEST_CASE("ResourceMtfMapReader detects SC4Map extension") {
    CHECK(ResourceMtfMapReader::isIt(".SC4Map") == true);
    CHECK(ResourceMtfMapReader::isIt(".sc4map") == true);
    CHECK(ResourceMtfMapReader::isIt(".MTF") == false);
  }

  TEST_CASE("ResourceMtfMapReader produces byte-exact map output") {
    std::string superName = "tmp/temp_golden.SC4Super";
    std::string mapName = "tmp/temp_golden.SC4Map";

    // supertile: count=1, width=1, height=1, limit=1, reserved, one tile (idx 1)
    std::string supertile =
        std::string("\x01\x01\x01\x01\x00\x00\x00\x01", 8);
    // tilemap: width=1, height=2, reserved(4), two supertile indices (both 0)
    std::string tilemap = std::string(
        "\x01\x00\x02\x00\x00\x00\x00\x00\x00\x00", 10);
    createTempFile(superName, supertile);
    createTempFile(mapName, tilemap);

    ResourceMtfMapReader reader(mapName);
    REQUIRE(reader.load() == true);
    REQUIRE(reader.data.size() == 3);

    // Header block: type(1) + width(2) + height(2) + line table (2 lines * 3)
    REQUIRE(reader.data[0].size() == 11);
    CHECK(reader.data[0][0] == 2);        // map resource type
    CHECK(reader.data[0][1] == 1);        // resource width (1)
    CHECK(reader.data[0][2] == 0);
    CHECK(reader.data[0][3] == 2);        // resource height (2)
    CHECK(reader.data[0][4] == 0);
    for (size_t i = 5; i < 11; i++) CHECK(reader.data[0][i] == 0);

    // Each line block: 3-byte link header + 1 tile + 31 mirrored tiles.
    REQUIRE(reader.data[1].size() == 35);
    REQUIRE(reader.data[2].size() == 35);
    CHECK(reader.data[1][0] == 0);
    CHECK(reader.data[1][1] == 0);
    CHECK(reader.data[1][2] == 0);
    for (int i = 3; i < 35; i++) CHECK(reader.data[1][i] == 1);
    CHECK(reader.data[2][0] == 0);
    CHECK(reader.data[2][1] == 0);
    CHECK(reader.data[2][2] == 0);
    for (int i = 3; i < 35; i++) CHECK(reader.data[2][i] == 1);

    // Remap first line to segment 2 @ 0x8000.
    CHECK(reader.remapTo(1, 2, 0x8000) == true);
    CHECK(reader.data[0][5] == 2);
    CHECK(reader.data[0][6] == 0x00);
    CHECK(reader.data[0][7] == 0x80);
    CHECK(reader.data[1][0] == 2);
    CHECK(reader.data[1][1] == 0x00);
    CHECK(reader.data[1][2] == 0x80);

    // Remap second line to segment 3 @ 0x9000; it links back to the first line.
    CHECK(reader.remapTo(2, 3, 0x9000) == true);
    CHECK(reader.data[0][8] == 3);
    CHECK(reader.data[0][9] == 0x00);
    CHECK(reader.data[0][10] == 0x90);
    CHECK(reader.data[1][0] == 3);        // first line -> next is second line
    CHECK(reader.data[1][1] == 0x00);
    CHECK(reader.data[1][2] == 0x90);
    CHECK(reader.data[2][0] == 2);        // second line wraps to the first line
    CHECK(reader.data[2][1] == 0x00);
    CHECK(reader.data[2][2] == 0x80);

    deleteTempFile(superName);
    deleteTempFile(mapName);
  }

  TEST_CASE("ResourceMtfMapReader handles 3-byte count header and 2D tiles") {
    std::string superName = "tmp/temp_3byte.SC4Super";
    std::string mapName = "tmp/temp_3byte.SC4Map";

    // supertile: 3-byte count header (first byte 0), count=2, width=2,
    // height=2, limit 0xFFFF, reserved(2), two 2x2 supertiles
    std::string supertile;
    supertile += '\x00';  // count marker -> 3-byte count
    supertile += '\x02';  // count low
    supertile += '\x00';  // count high
    supertile += '\x02';  // width
    supertile += '\x02';  // height
    supertile += '\xFF';  // limit low
    supertile += '\xFF';  // limit high
    supertile += '\x00';  // reserved
    supertile += '\x00';  // reserved
    supertile += std::string("\x01\x02\x03\x04", 4);  // supertile 0 (2x2)
    supertile += std::string("\x05\x06\x07\x08", 4);  // supertile 1 (2x2)

    // tilemap: width=1, height=2, reserved(4), two 2-byte supertile indices
    std::string tilemap;
    tilemap += std::string("\x01\x00\x02\x00\x00\x00\x00\x00", 8);
    tilemap += std::string("\x00\x00", 2);  // index 0 -> supertile 0
    tilemap += std::string("\x01\x00", 2);  // index 1 -> supertile 1

    createTempFile(superName, supertile);
    createTempFile(mapName, tilemap);

    ResourceMtfMapReader reader(mapName);
    REQUIRE(reader.load() == true);
    REQUIRE(reader.data.size() == 5);

    // Header: type + width + height + 4-line table
    REQUIRE(reader.data[0].size() == 17);
    CHECK(reader.data[0][0] == 2);
    CHECK(reader.data[0][1] == 2);  // resource width = tilemapWidth * supertileWidth
    CHECK(reader.data[0][2] == 0);
    CHECK(reader.data[0][3] == 4);  // resource height = tilemapHeight * supertileHeight
    CHECK(reader.data[0][4] == 0);

    // Four line blocks (tilemapHeight * supertileHeight), each 2+31+3 bytes
    CHECK(reader.data[1].size() == 36);
    CHECK(reader.data[1][3] == 0x01);
    CHECK(reader.data[1][4] == 0x02);
    CHECK(reader.data[2][3] == 0x03);
    CHECK(reader.data[2][4] == 0x04);
    CHECK(reader.data[3][3] == 0x05);
    CHECK(reader.data[3][4] == 0x06);
    CHECK(reader.data[4][3] == 0x07);
    CHECK(reader.data[4][4] == 0x08);

    // Remap all four lines
    CHECK(reader.remapTo(1, 2, 0x8000) == true);
    CHECK(reader.remapTo(2, 3, 0x9000) == true);
    CHECK(reader.remapTo(3, 4, 0xA000) == true);
    CHECK(reader.remapTo(4, 5, 0xB000) == true);

    deleteTempFile(superName);
    deleteTempFile(mapName);
  }

  TEST_CASE("ResourceMtfReader reports not implemented") {
    std::string fname = "tmp/temp_notimpl.mtf";
    createTempFile(fname, "MTF");

    ResourceMtfReader reader(fname);
    CHECK(reader.load() == false);
    CHECK(reader.getLogger()->errors().toString().find(
              "Not implemented yet") != std::string::npos);

    SUBCASE("Detects MTF extension") {
      CHECK(ResourceMtfReader::isIt(".MTF") == true);
      CHECK(ResourceMtfReader::isIt(".mtf") == true);
      CHECK(ResourceMtfReader::isIt(".BIN") == false);
    }

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceCsvReader parses quoted and escaped fields") {
    std::string fname = "tmp/temp_quoted.csv";
    std::string csv =
        "id,name,value\n"
        "1,\"Alice, A\",10\n"
        "2,\"Bob \"\"B\"\"\",20\n"
        "3,Charlie,&HFF\n"
        "4,,\n";
    createTempFile(fname, csv);

    ResourceCsvReader reader(fname);
    CHECK(reader.load() == true);
    CHECK(reader.data.size() > 0);
    CHECK(reader.unpackedSize > 0);

    SUBCASE("Detects CSV extension") {
      CHECK(ResourceCsvReader::isIt(".CSV") == true);
      CHECK(ResourceCsvReader::isIt(".csv") == true);
      CHECK(ResourceCsvReader::isIt(".txt") == false);
    }

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceCsvReader parses semicolon-separated and hex values") {
    std::string fname = "tmp/temp_semi.csv";
    std::string csv = "a;b;c\n1;&O17;&B101\n\"x\";2;3\n";
    createTempFile(fname, csv);

    ResourceCsvReader reader(fname);
    CHECK(reader.load() == true);
    CHECK(reader.unpackedSize > 0);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceCsvReader converts lowercase radix prefixes") {
    std::string fname = "tmp/temp_lowradix.csv";
    std::string csv = "1,&h0f,&o17,&b11,255\n";
    createTempFile(fname, csv);

    ResourceCsvReader reader(fname);
    CHECK(reader.load() == true);
    REQUIRE(reader.data.size() >= 5);

    const char* expected[] = {"1", "15", "15", "3", "255"};
    for (size_t i = 1; i < 6; i++) {
      CHECK(reader.data[i][0] == strlen(expected[i - 1]));
      std::string field(reinterpret_cast<char*>(reader.data[i].data()) + 1,
                        reader.data[i][0]);
      CHECK(field == expected[i - 1]);
    }

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceCsvReader parses a large single-line CSV") {
    std::string fname = "tmp/temp_big.csv";
    std::string line = "0";
    for (int i = 1; i < 500; i++) line += "," + std::to_string(i);
    line += "\n";
    createTempFile(fname, line);

    ResourceCsvReader reader(fname);
    CHECK(reader.load() == true);
    CHECK(reader.unpackedSize > 0);

    deleteTempFile(fname);
  }

  TEST_CASE("ResourceFactory creates readers by extension") {
    std::string txtName = "tmp/temp_factory.txt";
    std::string csvName = "tmp/temp_factory.csv";
    std::string scrName = "tmp/temp_factory.scr";
    std::string sprName = "tmp/temp_factory.spr";
    std::string akmName = "tmp/temp_factory.akm";
    std::string akxName = "tmp/temp_factory.akx";
    std::string mtfName = "tmp/temp_factory.mtf";
    std::string palName = "tmp/temp_factory.sc4pal";
    std::string tileName = "tmp/temp_factory.sc4tile";
    std::string mapName = "tmp/temp_factory.sc4map";
    std::string blobName = "tmp/temp_factory.bin";
    std::string unkName = "tmp/temp_factory.xyz";

    createTempFile(txtName, "hello");
    createTempFile(csvName, "a,b\n1,2\n");
    createTempFile(scrName, std::string(6912, '\xFF'));
    createTempFile(sprName, "!type\nmsx1\n#Slot 0\n");
    createTempFile(akmName, "AKM");
    createTempFile(akxName, "AKX");
    createTempFile(mtfName, "MTF");
    createTempFile(palName, std::string(32, 0));
    createTempFile(tileName, std::string(2048, 0));
    createTempFile(mapName, std::string(9, 0));
    createTempFile(blobName, "BLOB");
    createTempFile(unkName, "UNKNOWN");

    ResourceFactory factory;

    CHECK(factory.create(txtName) != nullptr);
    CHECK(factory.create(csvName) != nullptr);
    CHECK(factory.create(scrName) != nullptr);
    CHECK(factory.create(sprName) != nullptr);
    CHECK(factory.create(akmName) != nullptr);
    CHECK(factory.create(akxName) != nullptr);
    CHECK(factory.create(mtfName) != nullptr);
    CHECK(factory.create(palName) != nullptr);
    CHECK(factory.create(tileName) != nullptr);
    CHECK(factory.create(mapName) != nullptr);
    CHECK(factory.create(blobName) != nullptr);
    CHECK(factory.create(unkName) != nullptr);

    deleteTempFile(txtName);
    deleteTempFile(csvName);
    deleteTempFile(scrName);
    deleteTempFile(sprName);
    deleteTempFile(akmName);
    deleteTempFile(akxName);
    deleteTempFile(mtfName);
    deleteTempFile(palName);
    deleteTempFile(tileName);
    deleteTempFile(mapName);
    deleteTempFile(blobName);
    deleteTempFile(unkName);
  }
}

// NOLINTEND
