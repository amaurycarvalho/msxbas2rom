/***
 * @file test_pletter.cpp
 * @brief Pletter compression unit testing
 * @author Amaury Carvalho (2019-2026)
 * @note
 * doctest reference:
 *   https://github.com/doctest/doctest/blob/master/doc/markdown/tutorial.md
 */

// NOLINTBEGIN

#include <cstddef>
#include <vector>

#include "doctest/doctest.h"
#include "pletter.h"

static void checkBytes(const std::vector<unsigned char>& actual,
                       const std::vector<unsigned char>& expected) {
  REQUIRE(actual.size() == expected.size());
  for (std::size_t i = 0; i < expected.size(); ++i) {
    CHECK(actual[i] == expected[i]);
  }
}

TEST_SUITE("Pletter suite") {
  TEST_CASE("maxPackedSize returns the worst-case bound") {
    CHECK(Pletter::maxPackedSize(1) == 6);
    CHECK(Pletter::maxPackedSize(5) == 11);
    CHECK(Pletter::maxPackedSize(6) == 12);
    CHECK(Pletter::maxPackedSize(200) == 230);
    CHECK(Pletter::maxPackedSize(8192) == 9221);
  }

  TEST_CASE("pack rejects null and empty input") {
    Pletter pletter;
    std::vector<unsigned char> out(1, 0xAA);
    unsigned char byte = 0;

    CHECK(pletter.pack(nullptr, 1, out) == 0);
    CHECK(out.empty());

    out.assign(1, 0xAA);
    CHECK(pletter.pack(&byte, 0, out) == 0);
    CHECK(out.empty());
  }

  TEST_CASE("pack rounds-trips 1 through 6 byte inputs") {
    const std::vector<std::vector<unsigned char>> expected = {
        {0x1F, 0x42, 0xFF, 0xFF, 0xFF, 0xF8},
        {0x0F, 0x42, 0x42, 0xFF, 0xFF, 0xFF, 0xFC},
        {0x17, 0x42, 0x00, 0xFF, 0xFF, 0xFF, 0xFE},
        {0x19, 0x42, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x80},
        {0x1D, 0x42, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x80},
        {0x1A, 0x42, 0x7F, 0x00, 0xFF, 0xFF, 0xFF, 0xE0},
    };

    for (std::size_t n = 1; n <= 6; ++n) {
      Pletter pletter;
      std::vector<unsigned char> input(n, 0x42);
      std::vector<unsigned char> out;
      int packed = pletter.pack(input.data(), (int)input.size(), out);

      CHECK(packed > 0);
      CHECK(out.size() == static_cast<std::size_t>(packed));
      CHECK(packed <= Pletter::maxPackedSize(static_cast<int>(n)));
      checkBytes(out, expected[n - 1]);
    }
  }

  TEST_CASE("pack preserves the 96-byte constant payload") {
    Pletter pletter;
    std::vector<unsigned char> input(96, 0xAB);
    std::vector<unsigned char> out;
    int packed = pletter.pack(input.data(), (int)input.size(), out);

    REQUIRE(packed == 9);
    checkBytes(out, {0x1B, 0xAB, 0xFE, 0x7F, 0x00,
                     0xFF, 0xFF, 0xFF, 0xE0});
  }

  TEST_CASE("pack preserves the 96-byte incrementing payload") {
    Pletter pletter;
    std::vector<unsigned char> input;
    for (int i = 0; i < 96; ++i) {
      input.push_back(static_cast<unsigned char>(i));
    }
    std::vector<unsigned char> out;
    int packed = pletter.pack(input.data(), (int)input.size(), out);

    REQUIRE(packed == 113);
    checkBytes(out,
               {0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x06, 0x07,
                0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x00, 0x0E, 0x0F, 0x10,
                0x11, 0x12, 0x13, 0x14, 0x15, 0x00, 0x16, 0x17, 0x18, 0x19,
                0x1A, 0x1B, 0x1C, 0x1D, 0x00, 0x1E, 0x1F, 0x20, 0x21, 0x22,
                0x23, 0x24, 0x25, 0x00, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B,
                0x2C, 0x2D, 0x00, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34,
                0x35, 0x00, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D,
                0x00, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x00,
                0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x00, 0x4E,
                0x4F, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x00, 0x56, 0x57,
                0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x3F, 0x5E, 0x5F, 0xFF,
                0xFF, 0xFF, 0xF0});
  }

  TEST_CASE("incompressible data respects the size bound") {
    Pletter pletter;
    std::vector<unsigned char> input(200);
    unsigned value = 0x12345678u;
    for (std::size_t i = 0; i < input.size(); ++i) {
      value = value * 1103515245u + 12345u;
      input[i] = static_cast<unsigned char>(value >> 16);
    }
    std::vector<unsigned char> out;
    int packed = pletter.pack(input.data(), (int)input.size(), out);

    CHECK(packed > 0);
    CHECK(out.size() == static_cast<std::size_t>(packed));
    CHECK(packed <= Pletter::maxPackedSize((int)input.size()));
  }
}

TEST_SUITE("Slow") {
  TEST_CASE("pack preserves the 8K constant payload") {
    Pletter pletter;
    std::vector<unsigned char> input(8192, 0xAB);
    std::vector<unsigned char> out;
    int packed = pletter.pack(input.data(), (int)input.size(), out);

    REQUIRE(packed == 10);
    CHECK(packed <= Pletter::maxPackedSize(8192));
    checkBytes(out, {0x1F, 0xAB, 0xFF, 0xFF, 0xE7,
                     0x00, 0xFF, 0xFF, 0xFF, 0xFE});
  }
}

// NOLINTEND
