/***
 * @file test_generated_z80.cpp
 * @brief Non-scoring generated-Z80 regression fingerprints
 */

// NOLINTBEGIN

#include <cstdio>
#include <string>
#include <vector>

#include "doctest/doctest.h"
#include "test_support.h"

static unsigned int fnv1a(const std::vector<unsigned char>& bytes) {
  unsigned int hash = 2166136261u;
  for (unsigned char b : bytes) {
    hash ^= b;
    hash *= 16777619u;
  }
  return hash;
}

static std::vector<unsigned char> maskKernelTargets(
    const std::vector<unsigned char>& code) {
  std::vector<unsigned char> masked = code;
  for (std::size_t i = 0; i + 2 < masked.size();) {
    unsigned char op = masked[i];
    if (op == 0xCD || op == 0xC3 || op == 0xCA || op == 0xC2 || op == 0xD2 ||
        op == 0xDA || op == 0xCC || op == 0xC4) {
      masked[i + 1] = 0;
      masked[i + 2] = 0;
      i += 3;
    } else {
      i++;
    }
  }
  return masked;
}

static std::vector<unsigned char> compileBytes(const std::string& name,
                                               const std::string& source) {
  CompilationHarness harness;
  auto compiler = harness.compileSource("tmp/" + name + ".bas", source);
  REQUIRE(compiler.get() != nullptr);
  int n = compiler->getCodeSize();
  std::vector<unsigned char> bytes(n);
  compiler->write(bytes.data(), 0);
  return bytes;
}

TEST_SUITE("GeneratedZ80Regression") {
  TEST_CASE("Representative programs keep their normalised fingerprint") {
    struct Case {
      const char* name;
      const char* source;
      int size;
      unsigned int fingerprint;
    };
    const Case cases[] = {
        {"r6_z80_rom", "10 PRINT \"X\"\n20 END\n", 55, 0x05F847A4u},
        {"r6_z80_sym", "10 A=1\n20 END\n", 57, 0xEC0697D5u},
        {"r6_z80_line", "10 LINE (0,0)-(1,1),15\n20 END\n", 87, 0x71960EE5u},
        {"r6_z80_for", "10 FOR I=1 TO 10\n20 NEXT\n", 144, 0x58ACBFD5u},
        {"r6_z80_print", "10 PRINT USING \"##\";A\n20 END\n", 89, 0xDB9E4E3Fu},
        {"r6_z80_mtf", "10 CMD MTF 0\n20 END\n", 86, 0x8C0E5348u},
    };
    for (const auto& c : cases) {
      auto bytes = compileBytes(c.name, c.source);
      CHECK(static_cast<int>(bytes.size()) == c.size);
      CHECK(fnv1a(maskKernelTargets(bytes)) == c.fingerprint);
      CHECK(bytes.size() > 2);
    }
  }

  TEST_CASE("Normalisation masks only the operands of call/jp") {
    std::vector<unsigned char> raw = {0x3E, 0x01, 0xCD, 0x34, 0x12, 0xC3,
                                      0x78, 0x56, 0x21, 0x11, 0x22};
    auto masked = maskKernelTargets(raw);
    CHECK(masked[0] == 0x3E);
    CHECK(masked[1] == 0x01);
    CHECK(masked[2] == 0xCD);
    CHECK(masked[3] == 0);
    CHECK(masked[4] == 0);
    CHECK(masked[5] == 0xC3);
    CHECK(masked[6] == 0);
    CHECK(masked[7] == 0);
    CHECK(masked[8] == 0x21);
    CHECK(masked[9] == 0x11);
    CHECK(masked[10] == 0x22);
  }
}

// NOLINTEND
