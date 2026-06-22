/***
 * @file test_kernel.cpp
 * @brief MSXBAS2ROM kernel unit tests
 */

// NOLINTBEGIN

#include <cstdio>
#include <string>

#include "doctest/doctest.h"

TEST_CASE("Kernel ROM size within 0x4000 limit") {
  const char* header_bin_path = "src/infrastructure/kernel/asm/bin/header.bin";

  FILE* f = fopen(header_bin_path, "rb");
  if (!f) {
    // Fallback: try relative from test binary location
    f = fopen("../../src/infrastructure/kernel/asm/bin/header.bin", "rb");
  }
  if (!f) {
    // Test may run from project root
    f = fopen("../../../../src/infrastructure/kernel/asm/bin/header.bin", "rb");
  }

  REQUIRE_MESSAGE(f != nullptr,
                  "header.bin not found — run kernel build first");

  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fclose(f);

  const long limit = 32768;  // 0x8000 (0x4000 for kernel routines map table and
                             // 0x4000 for kernel routines)

  CHECK_MESSAGE(size > 0, "header.bin should be non-empty");
  CHECK_MESSAGE(size <= limit, "Kernel ROM size ", size,
                " bytes exceeds 0x4000 limit by ", size - limit, " bytes");
}

// NOLINTEND
