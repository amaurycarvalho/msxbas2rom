/***
 * @file test_kernel_layout.cpp
 * @brief Non-scoring kernel dispatch integrity checks
 */

// NOLINTBEGIN

#include "compiler_hooks.h"
#include "doctest/doctest.h"
#include "test_support.h"

TEST_SUITE("KernelLayoutIntegrity") {
  TEST_CASE("Every dispatch entry resolves inside the kernel image") {
    auto image = kernelImage();
    REQUIRE(image.get() != nullptr);
    REQUIRE(image->size() > 0);
    for (int i = 0; i < DISP_ENTRIES; i++) {
      unsigned int address = image->dispatchAddress(i);
      INFO("dispatch index: " << i);
      CHECK(address != 0);
      CHECK(address < image->size());
    }
  }

  TEST_CASE("MR anchor entries are inside the kernel range") {
    auto image = kernelImage();
    CHECK(image->dispatchAddress(DISP_MR_CALL) < image->size());
    CHECK(image->dispatchAddress(DISP_MR_JUMP) < image->size());
    CHECK(image->dispatchAddress(DISP_MR_GET_DATA) < image->size());
  }
}

// NOLINTEND
