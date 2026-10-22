/***
 * @file test_mutation_resources.cpp
 * @brief Round 6 sentinel-driven resource reader tests
 */

// NOLINTBEGIN

#include <memory>
#include <string>
#include <vector>

#include "doctest/doctest.h"
#include "in_memory_resource_content_source.h"
#include "resource_akm_reader.h"
#include "resource_akx_reader.h"
#include "resource_spr_reader.h"

static std::shared_ptr<InMemoryResourceContentSource> round6Source() {
  return std::make_shared<InMemoryResourceContentSource>();
}

static void addBinary(const std::shared_ptr<InMemoryResourceContentSource>& src,
                      const std::string& name, const std::string& bytes) {
  src->addBinary(name, std::vector<unsigned char>(bytes.begin(), bytes.end()));
  src->addText(name, bytes);
}

TEST_SUITE("Round6Resources") {
  TEST_CASE("AKM sentinel bytes survive a full header remap") {
    auto source = round6Source();
    std::string content;
    content += std::string("\x08\x00", 2);  // instrument table
    content += std::string("\x10\x00", 2);  // arpeggio table
    content += std::string("\x12\x00", 2);  // pitch table
    content += std::string("\x0C\x00", 2);  // subsong table
    content += std::string("\x0A\x00", 2);  // instrument entry
    content += std::string("\x00\x00", 2);  // padding
    content += std::string("\x00\x00", 2);  // note block = 0
    content += std::string("\x00\x00", 2);  // track table = 0
    for (int i = 16; i < 24; i++) content += static_cast<char>(0xA0 + i);
    content += '\x0C';  // effects flag
    for (int i = 25; i < 32; i++) content += static_cast<char>(0xA0 + i);
    addBinary(source, "tmp/r6_sentinel.akm", content);

    ResourceAkmReader reader("tmp/r6_sentinel.akm", source);
    REQUIRE(reader.load() == true);
    REQUIRE(reader.remapTo(0, 0, 0x8000) == true);
    REQUIRE(reader.data.size() >= 1);
    const auto& out = reader.data[0];
    REQUIRE(out.size() == content.size());
    CHECK(out[0] == 0x08);
    CHECK(out[1] == 0x80);
    CHECK(out[2] == 0x10);
    CHECK(out[3] == 0x80);
    CHECK(out[4] == 0x12);
    CHECK(out[5] == 0x80);
    CHECK(out[6] == 0x0C);
    CHECK(out[7] == 0x80);
    CHECK(out[8] == 0x0A);
    CHECK(out[9] == 0x80);
    for (int i = 16; i < 24; i++) CHECK(out[i] == static_cast<unsigned char>(0xA0 + i));
    CHECK(out[24] == 0x0C);
    for (int i = 25; i < 32; i++) CHECK(out[i] == static_cast<unsigned char>(0xA0 + i));
  }

  TEST_CASE("AKX pointer remap leaves sentinel payload untouched") {
    auto source = round6Source();
    std::string content =
        std::string("\x04\x00\x06\x00", 4) +
        std::string("\xA0\xA1\xA2\xA3\xA4\xA5\xA6\xA7", 8);
    addBinary(source, "tmp/r6_sentinel.akx", content);

    ResourceAkxReader reader("tmp/r6_sentinel.akx", source);
    REQUIRE(reader.load() == true);
    REQUIRE(reader.remapTo(0, 0, 0x8000) == true);
    REQUIRE(reader.data.size() >= 1);
    const auto& out = reader.data[0];
    CHECK(out[0] == 0x04);
    CHECK(out[1] == 0x80);
    CHECK(out[2] == 0x06);
    CHECK(out[3] == 0x80);
    for (int i = 4; i < 12; i++)
      CHECK(out[i] == static_cast<unsigned char>(0xA0 + i - 4));
  }

  TEST_CASE("SPR MSX1 single pixel lands on every quadrant and bit") {
    const int colors[] = {1, 2, 15};
    for (int color : colors) {
      for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
          auto source = round6Source();
          std::string content = "!type\nmsx1\n#Slot 0\n";
          for (int row = 0; row < 16; row++) {
            for (int col = 0; col < 16; col++) {
              if (row == y && col == x) {
                content += "0123456789ABCDEF"[color];
              } else {
                content += '.';
              }
            }
            content += '\n';
          }
          addBinary(source, "tmp/r6_pixel.spr", content);
          ResourceSprReader reader("tmp/r6_pixel.spr", source);
          INFO("x=" << x << " y=" << y << " color=" << color);
          REQUIRE(reader.load() == true);
          CHECK(reader.unpackedSize == 35);
          CHECK(reader.packedSize > 0);
          CHECK(reader.data.size() == 1);
        }
      }
    }
  }

  TEST_CASE("SPR MSX2 single pixel lands on every quadrant and row color") {
    for (int y = 0; y < 16; y++) {
      for (int x = 0; x < 16; x++) {
        auto source = round6Source();
        std::string content = "!type\nmsx2\n#Slot 0\n";
        for (int row = 0; row < 16; row++) {
          for (int col = 0; col < 16; col++) {
            content += (row == y && col == x) ? '5' : '.';
          }
          content += '\n';
        }
        addBinary(source, "tmp/r6_pixel2.spr", content);
        ResourceSprReader reader("tmp/r6_pixel2.spr", source);
        INFO("x=" << x << " y=" << y);
        REQUIRE(reader.load() == true);
        CHECK(reader.unpackedSize == 50);
        CHECK(reader.packedSize > 0);
        CHECK(reader.data.size() == 1);
      }
    }
  }
}

// NOLINTEND
