/***
 * @file test_kernel_layout.cpp
 * @brief Non-scoring kernel dispatch integrity checks
 */

// NOLINTBEGIN

#include "compiler_hooks.h"
#include "doctest/doctest.h"
#include "generated_kernel_module_library.h"
#include "test_support.h"

TEST_SUITE("KernelLayoutIntegrity") {
  TEST_CASE("Every dispatch entry resolves to a kernel module export") {
    GeneratedKernelModuleLibrary library;
    const auto& dispatchMap = library.dispatchExports();
    auto image = kernelImage();
    REQUIRE(image.get() != nullptr);

    CHECK(dispatchMap.size() == DISP_ENTRIES);
    for (int i = 0; i < DISP_ENTRIES; i++) {
      INFO("dispatch index: " << i);

      auto dispatch = dispatchMap.find(i);
      REQUIRE(dispatch != dispatchMap.end());

      const KernelModule* owner = nullptr;
      for (const auto& module : library.modules()) {
        if (module.findExport(dispatch->second)) {
          owner = &module;
          break;
        }
      }
      REQUIRE(owner != nullptr);

      const KernelExport* entry = owner->findExport(dispatch->second);
      REQUIRE(entry != nullptr);
      CHECK(entry->offset < owner->size());

      if (owner->hasFixedAddress) {
        CHECK(owner->fixedAddress != 0);
        CHECK(owner->fixedAddress + entry->offset < image->size());
      } else {
        CHECK(!owner->bytes.empty());
      }
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
