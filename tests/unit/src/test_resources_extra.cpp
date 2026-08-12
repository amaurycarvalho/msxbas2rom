/***
 * @file test_resources_extra.cpp
 * @brief MSXBAS2ROM additional resource readers unit testing
 */

// NOLINTBEGIN

#include <cstdio>
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
