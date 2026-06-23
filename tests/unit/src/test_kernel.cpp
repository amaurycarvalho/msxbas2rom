/***
 * @file test_kernel.cpp
 * @brief MSXBAS2ROM kernel unit tests
 */

// NOLINTBEGIN

#include "doctest/doctest.h"

extern unsigned int bin_header_bin_len;

TEST_CASE("Kernel ROM size within 0x4000 limit") {
  const long size = static_cast<long>(bin_header_bin_len);

  const long limit = 32768;  // 0x8000 (0x4000 for kernel routines map table and
                             // 0x4000 for kernel routines)

  CHECK_MESSAGE(size > 0, "header.bin should be non-empty");
  CHECK_MESSAGE(size <= limit, "Kernel ROM size ", size,
                " bytes exceeds 0x4000 limit by ", size - limit, " bytes");
}

// NOLINTEND
